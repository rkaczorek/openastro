/*****************************************************************************
 *
 * yuv.c -- convert YUV formats to RGB888
 *
 * Copyright 2014,2020,2021
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
#include <openastro/video.h>

#include "yuv.h"
#include "yuvlut.h"

#define CLAMP(v,min,max) (( v < min ) ? min : (( v > max ) ? max : v ))


void
oaYUV444PtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  uint8_t* ys = source;
  uint8_t* us = ( uint8_t* ) source + len;
  uint8_t* vs = ( uint8_t* ) source + 2 * len;
  uint8_t* t = ( uint8_t* ) target;
  int32_t r32, g32, b32;
  uint8_t r8, g8, b8, y, u, v;

  while ( len > 0 ) {
    y = *ys++;
    u = *us++;
    v = *vs++;
    r32 = y + lut_1_4075[v];
    g32 = y - lut_0_3455[u] - lut_0_7169[v];
    b32 = y + lut_1_7790[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;
    len--;
  }
}


void
oaYUV422PtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  uint8_t* ys = ( uint8_t* ) source;
  uint8_t* us = ( uint8_t* ) source + len;
  uint8_t* vs = us + len / 2;
  uint8_t* t = ( uint8_t* ) target;
  int32_t r32, g32, b32;
  uint8_t r8, g8, b8, y1, y2, u, v;
  float o;

  while ( len > 0 ) {
    y1 = *ys++;
    y2 = *ys++;
    u = *us++;
    v = *vs++;

    // first pixel

    r32 = y1 + lut_1_4075[v];
    o = lut_0_3455[u] - lut_0_7169[v];
    g32 = y1 - o;
    b32 = y1 + lut_1_7790[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    // second pixel

    r32 = y2 + lut_1_4075[v];
    g32 = y2 - o;
    b32 = y2 + lut_1_7790[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    len -= 2;
  }
}


void
oaYUV420PtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  unsigned int len_div_4, width_div_2, r_div_2;
  unsigned int c, r;
  int32_t r32, g32, b32;
  uint8_t r8, g8, b8, y, u, v;
  uint8_t* ys = ( uint8_t* ) source;
  uint8_t* u_off;
  uint8_t* v_off;
  uint8_t* t = ( uint8_t* ) target;

  len_div_4 = len / 4;
  width_div_2 = xSize / 2;

  for ( r = 0; r < ySize; r++ ) {
    r_div_2 = r / 2;
    u_off = ( uint8_t* ) source + r_div_2 * width_div_2 + len;
    v_off = ( uint8_t* ) source + r_div_2 * width_div_2 + len + len_div_4;
    for ( c = 0; c < xSize; c++ ) {
      y = *ys++;
      u = *( u_off + c / 2 );
      v = *( v_off + c / 2 );
      r32 = y + lut_1_4075[v];
      g32 = y - lut_0_3455[u] - lut_0_7169[v];
      b32 = y + lut_1_7790[u];
      r8 = CLAMP(r32,0,255);
      g8 = CLAMP(g32,0,255);
      b8 = CLAMP(b32,0,255);
      *t++ = r8;
      *t++ = g8;
      *t++ = b8;
    }
  }
}


void
oaYUYVtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  uint8_t* s = ( uint8_t* ) source;
  uint8_t* t = ( uint8_t* ) target;
  int32_t r32, g32, b32;
  uint8_t r8, g8, b8, y1, y2, u, v;
  float o;

  while ( len > 0 ) {
    y1 = *s++;
    u = *s++;
    y2 = *s++;
    v = *s++;

    // first pixel

    r32 = y1 + lut_1_370705[v];
    o = lut_0_698001[v] - lut_0_337633[u];
    g32 = y1 - o;
    b32 = y1 + lut_1_732446[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    // second pixel

    r32 = y2 + lut_1_370705[v];
    g32 = y2 - o;
    b32 = y2 + lut_1_732446[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    len -= 2;
  }
}


void
oaUYVYtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  uint8_t* s = ( uint8_t* ) source;
  uint8_t* t = ( uint8_t* ) target;
  int32_t r32, g32, b32;
  uint8_t r8, g8, b8, y1, y2, u, v;
  float o;

  while ( len > 0 ) {
    u = *s++;
    y1 = *s++;
    v = *s++;
    y2 = *s++;

    // first pixel

    r32 = y1 + lut_1_370705[v];
    o = lut_0_698001[v] - lut_0_337633[u];
    g32 = y1 - o;
    b32 = y1 + lut_1_732446[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    // second pixel

    r32 = y2 + lut_1_370705[v];
    g32 = y2 - o;
    b32 = y2 + lut_1_732446[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    len -= 2;
  }
}


void
oaYVYUtoRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  uint8_t* s = ( uint8_t* ) source;
  uint8_t* t = ( uint8_t* ) target;
  int32_t r32, g32, b32;
  uint8_t r8, g8, b8, y1, y2, u, v;
  float o;

  while ( len > 0 ) {
    y1 = *s++;
    v = *s++;
    y2 = *s++;
    u = *s++;

    // first pixel

    r32 = y1 + lut_1_370705[v];
    o = lut_0_698001[v] - lut_0_337633[u];
    g32 = y1 - o;
    b32 = y1 + lut_1_732446[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    // second pixel

    r32 = y2 + lut_1_370705[v];
    g32 = y2 - o;
    b32 = y2 + lut_1_732446[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    len -= 2;
  }
}


void
oaNV12toRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  uint8_t* lp = ( uint8_t* ) source;
  uint8_t* cp = ( uint8_t* ) source;
  uint8_t* t = ( uint8_t* ) target;
  int32_t r32, g32, b32, i, j;
  uint8_t r8, g8, b8, y1, y2, u, v;
  float o, lut1, lut2;
  uint8_t* scp;

	cp += len; // start of chrominance data
	scp = cp;
 
	for ( j = 0; j < ySize; j++ ) {
		for ( i = 0; i < xSize; i += 2 ) {
			y1 = *lp++;
			y2 = *lp++;
			u = *cp++;
			v = *cp++;

			o = lut_0_698001[v] - lut_0_337633[u];
			lut1 = lut_1_370705[v];
			lut2 = lut_1_732446[u];

			// first pixel

			r32 = y1 + lut1;
			g32 = y1 - o;
			b32 = y1 + lut2;
			r8 = CLAMP(r32,0,255);
			g8 = CLAMP(g32,0,255);
			b8 = CLAMP(b32,0,255);
			*t++ = r8;
			*t++ = g8;
			*t++ = b8;

			// second pixel

			r32 = y2 + lut1;
			g32 = y2 - o;
			b32 = y2 + lut2;
			r8 = CLAMP(r32,0,255);
			g8 = CLAMP(g32,0,255);
			b8 = CLAMP(b32,0,255);
			*t++ = r8;
			*t++ = g8;
			*t++ = b8;
		}

		if ( j % 2 ) {
			scp = cp;
		} else {
			cp = scp;
		}
	}
}


void
oaNV21toRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  uint8_t* lp = ( uint8_t* ) source;
  uint8_t* cp = ( uint8_t* ) source;
  uint8_t* t = ( uint8_t* ) target;
  int32_t r32, g32, b32, i, j;
  uint8_t r8, g8, b8, y1, y2, u, v;
  float o, lut1, lut2;
  uint8_t* scp;

	cp += len; // start of chrominance data
	scp = cp;
 
	for ( j = 0; j < ySize; j++ ) {
		for ( i = 0; i < xSize; i += 2 ) {
			y1 = *lp++;
			y2 = *lp++;
			v = *cp++;
			u = *cp++;

			o = lut_0_698001[v] - lut_0_337633[u];
			lut1 = lut_1_370705[v];
			lut2 = lut_1_732446[u];

			// first pixel

			r32 = y1 + lut1;
			g32 = y1 - o;
			b32 = y1 + lut2;
			r8 = CLAMP(r32,0,255);
			g8 = CLAMP(g32,0,255);
			b8 = CLAMP(b32,0,255);
			*t++ = r8;
			*t++ = g8;
			*t++ = b8;

			// second pixel

			r32 = y2 + lut1;
			g32 = y2 - o;
			b32 = y2 + lut2;
			r8 = CLAMP(r32,0,255);
			g8 = CLAMP(g32,0,255);
			b8 = CLAMP(b32,0,255);
			*t++ = r8;
			*t++ = g8;
			*t++ = b8;
		}

		if ( j % 2 ) {
			scp = cp;
		} else {
			cp = scp;
		}
	}
}


void
oaYUV411toRGB888 ( void* source, void* target, unsigned int xSize,
    unsigned int ySize )
{
  unsigned int len = xSize * ySize;
  uint8_t* s = ( uint8_t* ) source;
  uint8_t* t = ( uint8_t* ) target;
  int32_t r32, g32, b32;
  uint8_t r8, g8, b8, y1, y2, y3, y4, u, v;
  float o;

  while ( len > 0 ) {
    u = *s++;
    y1 = *s++;
    y2 = *s++;
    v = *s++;
    y3 = *s++;
    y4 = *s++;

    o = lut_0_3455[u] - lut_0_7169[v];

    // first pixel

    r32 = y1 + lut_1_4075[v];
    g32 = y1 - o;
    b32 = y1 + lut_1_7790[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    // second pixel

    r32 = y2 + lut_1_4075[v];
    g32 = y2 - o;
    b32 = y2 + lut_1_7790[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    // third pixel

    r32 = y3 + lut_1_4075[v];
    g32 = y3 - o;
    b32 = y3 + lut_1_7790[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    // fourth pixel

    r32 = y4 + lut_1_4075[v];
    g32 = y4 - o;
    b32 = y4 + lut_1_7790[u];
    r8 = CLAMP(r32,0,255);
    g8 = CLAMP(g32,0,255);
    b8 = CLAMP(b32,0,255);
    *t++ = r8;
    *t++ = g8;
    *t++ = b8;

    len -= 6;
  }
}
