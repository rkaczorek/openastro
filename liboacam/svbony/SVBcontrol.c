/*****************************************************************************
 *
 * SVBcontrol.c -- control functions for SVBony cameras
 *
 * Copyright 2020,2021 James Fidell (james@openastroproject.org)
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
#include <openastro/camera.h>

#include <SVBCameraSDK.h>
#include <pthread.h>

#include "oacamprivate.h"
#include "SVBoacam.h"
#include "SVBstate.h"


int
oaSVBCameraTestControl ( oaCamera* camera, int control,
    oaControlValue* val )
{
  int32_t	val_s32;
  SVB_STATE*	cameraInfo = camera->_private;
  COMMON_INFO*	commonInfo = camera->_common;

	oaLogInfo ( OA_LOG_CAMERA, "%s ( %p, %d, %p ): entered", __func__, camera,
			control, val );

  if ( !camera->OA_CAM_CTRL_TYPE( control )) {
    return -OA_ERR_INVALID_CONTROL;
  }

  if ( camera->OA_CAM_CTRL_TYPE( control ) != val->valueType ) {
    return -OA_ERR_INVALID_CONTROL_TYPE;
  }

  switch ( control ) {
    case OA_CAM_CTRL_BRIGHTNESS:
    case OA_CAM_CTRL_BLUE_BALANCE:
    case OA_CAM_CTRL_RED_BALANCE:
    case OA_CAM_CTRL_GAMMA:
    case OA_CAM_CTRL_GAIN:
    case OA_CAM_CTRL_EXPOSURE_ABSOLUTE:
    case OA_CAM_CTRL_USBTRAFFIC:
    case OA_CAM_CTRL_OVERCLOCK:
      val_s32 = val->int32;
      if ( val_s32 >= commonInfo->OA_CAM_CTRL_MIN( control ) &&
          val_s32 <= commonInfo->OA_CAM_CTRL_MAX( control ) &&
          ( 0 == ( val_s32 - commonInfo->OA_CAM_CTRL_MIN( control )) %
          commonInfo->OA_CAM_CTRL_STEP( control ))) {
        return OA_ERR_NONE;
      }
      break;

    case OA_CAM_CTRL_BINNING:
    {
      int i = 0;

      val_s32 = val->discrete;
      while ( cameraInfo->binModes[i] && i < 16 ) {
        if ( val_s32 == cameraInfo->binModes[i] ) {
          return OA_ERR_NONE;
        }
        i++;
      }
      break;
    }

    // This lot are all boolean, so we'll take any value
    case OA_CAM_CTRL_HIGHSPEED:
    case OA_CAM_CTRL_HFLIP:
    case OA_CAM_CTRL_VFLIP:
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAIN ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_GAMMA ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BRIGHTNESS ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_EXPOSURE_ABSOLUTE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_RED_BALANCE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_BLUE_BALANCE ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_USBTRAFFIC ):
    case OA_CAM_CTRL_MODE_AUTO( OA_CAM_CTRL_OVERCLOCK ):
      return OA_ERR_NONE;
      break;

    default:
      // If we reach here it's because we don't recognise the control
      fprintf ( stderr, "Unrecognised control %d in oaSVBCameraTestControl\n",
          control );
      return -OA_ERR_INVALID_CONTROL;
      break;
  }

  // And if we reach here it's because the value wasn't valid

	oaLogInfo ( OA_LOG_CAMERA, "%s: exiting", __func__ );

  return -OA_ERR_OUT_OF_RANGE;
}


int
oaSVBCameraSetResolution ( oaCamera* camera, int x, int y )
{
  FRAMESIZE	s;
  OA_COMMAND	command;
  SVB_STATE*	cameraInfo = camera->_private;
  int		retval;

  if ( !cameraInfo->usb3Cam ) {
    if (( x * y ) % 1024 ) {
      return -OA_ERR_OUT_OF_RANGE;
    }
  }

  OA_CLEAR ( command );
  command.commandType = OA_CMD_RESOLUTION_SET;
  s.x = x;
  s.y = y;
  command.commandData = &s;
  cameraInfo = camera->_private;
  oaDLListAddToTail ( cameraInfo->commandQueue, &command );
  pthread_cond_broadcast ( &cameraInfo->commandQueued );
  pthread_mutex_lock ( &cameraInfo->commandQueueMutex );
  while ( !command.completed ) {
    pthread_cond_wait ( &cameraInfo->commandComplete,
        &cameraInfo->commandQueueMutex );
  }
  pthread_mutex_unlock ( &cameraInfo->commandQueueMutex );
  retval = command.resultCode;

  return retval;
}
