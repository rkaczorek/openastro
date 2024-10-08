/*****************************************************************************
 *
 * focus.c -- focus scoring algorithms
 *
 * Copyright 2015,2017,2018,2021
 *   James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#include <oa_common.h>

#include <openastro/imgproc.h>
#include <openastro/demosaic.h>
#include <openastro/errno.h>
#include <openastro/util.h>
#include <openastro/video/formats.h>

#include "sobel.h"
#include "scharr.h"
#include "gauss.h"


int
oaFocusScore ( void* source, void* target, int xSize, int ySize,
    int frameFormat )
{
  void*		currentSource = source;
  void*		currentTarget = target;
  void*		workspace1;
  void*		workspace2;
  int		freeSource, freeTarget;
  int		numPixels, result, canProcess;
  int		cfaPattern = 0, new8BitFormat = 0;

  freeSource = freeTarget = canProcess = 0;
  numPixels = xSize * ySize;
  workspace1 = workspace2 = 0;

  if ( oaFrameFormats[ frameFormat ].rawColour ) {
    switch ( frameFormat ) {
      case OA_PIX_FMT_RGGB8:
      case OA_PIX_FMT_RGGB16LE:
      case OA_PIX_FMT_RGGB16BE:
        cfaPattern = OA_DEMOSAIC_RGGB;
        new8BitFormat = OA_PIX_FMT_RGGB8;
        break;
      case OA_PIX_FMT_BGGR8:
      case OA_PIX_FMT_BGGR16LE:
      case OA_PIX_FMT_BGGR16BE:
        cfaPattern = OA_DEMOSAIC_BGGR;
        new8BitFormat = OA_PIX_FMT_BGGR8;
        break;
      case OA_PIX_FMT_GRBG8:
      case OA_PIX_FMT_GRBG16LE:
      case OA_PIX_FMT_GRBG16BE:
        cfaPattern = OA_DEMOSAIC_GRBG;
        new8BitFormat = OA_PIX_FMT_GRBG8;
        break;
      case OA_PIX_FMT_GBRG8:
      case OA_PIX_FMT_GBRG16LE:
      case OA_PIX_FMT_GBRG16BE:
        cfaPattern = OA_DEMOSAIC_GBRG;
        new8BitFormat = OA_PIX_FMT_GBRG8;
        break;
      default:
        oaLogError ( OA_LOG_IMGPROC, "%s: can't handle format %d", __func__,
            frameFormat );
        // FIX ME --return meaningful error code
        return -1;
        break;
    }

    if ( oaFrameFormats[ frameFormat ].bitsPerPixel == 16 ) {
      int i, length = xSize * ySize;
      uint8_t* s = currentSource;
      uint8_t* t;

      if (!( workspace1 = malloc ( xSize * ySize ))) {
        return -OA_ERR_MEM_ALLOC;
      }
      t = workspace1;
      if ( oaFrameFormats[ frameFormat ].littleEndian ) {
        s++;
      }
      for ( i = 0; i < length; i++ ) {
        *t++ = *s++;
        s++;
      }
      currentSource = workspace1;
      frameFormat = new8BitFormat;
    }

    if ( oaFrameFormats[ frameFormat ].bitsPerPixel == 8 ) {
      if (!( workspace2 = malloc ( xSize * ySize * 3 ))) {
        return -OA_ERR_MEM_ALLOC;
      }
      oademosaic ( currentSource, workspace2, xSize, ySize, 8,
          cfaPattern, OA_DEMOSAIC_NEAREST_NEIGHBOUR );
      currentSource = workspace2;
      frameFormat = OA_PIX_FMT_RGB24;
    }
  }

  if ( OA_PIX_FMT_RGB24 == frameFormat || OA_PIX_FMT_BGR24 == frameFormat ) {
    uint8_t*	s;
    uint8_t*	t;
    int		n = numPixels, v;

    if (!( currentTarget = malloc ( xSize * ySize ))) {
      return -OA_ERR_MEM_ALLOC;
    }

    // Convert the colours to a luminance value
    // These seem to be common formulae:
    // L = 0.3086.R + 0.6094.G + 0.0820.B
    // L = 0.299.R + 0.587.G + 0.114.B
    //
    // To make life easy and remove multiplications I'm going to use
    // L = 5/16.R + 9/16.G + 1/8.B
    // which works out as
    // L = 0.3125.R + 0.5625.G + 0.125.B

    t = currentTarget;
    s = currentSource;
    while ( n-- ) {
      if ( OA_PIX_FMT_RGB24 == frameFormat ) {
        v = ( *s << 2 ) + *s;
        s++;
        v += ( *s << 3 ) + *s;
        s++;
        v += *s << 1;
        s++;
      } else {
        v += *s << 1;
        s++;
        v += ( *s << 3 ) + *s;
        s++;
        v = ( *s << 2 ) + *s;
        s++;
      }
      *t++ = v >> 4;
    }
    currentSource = currentTarget;
    freeSource = 1;
    canProcess = 1;
  }

  if ( workspace1 ) {
    free ( workspace1 );
    workspace1 = 0;
  }
  if ( workspace2 ) {
    free ( workspace2 );
    workspace1 = 0;
  }

  if ( OA_PIX_FMT_GREY16BE == frameFormat ||
      OA_PIX_FMT_GREY16LE == frameFormat ) {
    uint8_t*	s;
    uint8_t*	t;
    int		n = numPixels;

    if (!( currentTarget = malloc ( xSize * ySize ))) {
      return -OA_ERR_MEM_ALLOC;
    }

    t = currentTarget;
    s = currentSource;
    if ( OA_PIX_FMT_GREY16LE == frameFormat ) {
      s++;
    }
    while ( n-- ) {
      *t++ = *s++;
      s++;
    }
    currentSource = currentTarget;
    freeSource = 1;
    canProcess = 1;
  }

  if ( canProcess || OA_PIX_FMT_GREY8 == frameFormat ) {
    if (!( currentTarget = malloc ( numPixels ))) {
      if ( freeSource ) {
        free ( currentSource );
      }
      return -OA_ERR_MEM_ALLOC;
    }
    gauss8_3x3 ( currentSource, currentTarget, xSize, ySize );
    currentSource = currentTarget;
    freeSource = 1;
    if ( target ) {
      currentTarget = target;
    } else {
      if (!( currentTarget = malloc ( numPixels ))) {
        free ( currentSource );
        return -OA_ERR_MEM_ALLOC;
      }
      freeTarget = 1;
    }
    result = sobel8 ( currentSource, currentTarget, xSize, ySize );
    if ( freeTarget ) {
      free ( currentTarget );
    }
    if ( freeSource ) {
      free ( currentSource );
    }
    return result;
  }

  // FIX ME -- return more meaningful error
  return -1;
}
