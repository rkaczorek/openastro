/*****************************************************************************
 *
 * SPinoacam.c -- main entrypoint for Point Grey Spinnaker interface
 *
 * Copyright 2018 James Fidell (james@openastroproject.org)
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

#if HAVE_LIBDL
#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#endif
#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <spinnaker/spinc/SpinnakerC.h>

#include "oacamprivate.h"
#include "unimplemented.h"
#include "Spinoacam.h"

// Pointers to Spinnaker functions so we can use them via libdl.

SPINNAKERC_API	( *p_spinSystemGetInstance )( spinSystem* );
SPINNAKERC_API	( *p_spinCameraListClear )( spinCameraList );
SPINNAKERC_API	( *p_spinCameraListCreateEmpty )( spinCameraList* );
SPINNAKERC_API	( *p_spinCameraListDestroy )( spinCameraList );
SPINNAKERC_API	( *p_spinCameraListGetSize )( spinCameraList, size_t* );
SPINNAKERC_API	( *p_spinInterfaceListClear )( spinInterfaceList );
SPINNAKERC_API	( *p_spinInterfaceListCreateEmpty )( spinInterfaceList* );
SPINNAKERC_API	( *p_spinInterfaceListDestroy )( spinInterfaceList );
SPINNAKERC_API	( *p_spinInterfaceListGetSize )( spinInterfaceList, size_t* );
SPINNAKERC_API	( *p_spinSystemGetCameras )( spinSystem, spinCameraList );
SPINNAKERC_API	( *p_spinSystemGetInterfaces )( spinSystem, spinInterfaceList );
SPINNAKERC_API	( *p_spinSystemReleaseInstance )( spinSystem );
SPINNAKERC_API	( *p_spinInterfaceListGet )( spinInterfaceList, size_t,
			spinInterface );
SPINNAKERC_API	( *p_spinInterfaceRelease )( spinInterface );
SPINNAKERC_API	( *p_spinInterfaceGetTLNodeMap )( spinInterface,
			spinNodeMapHandle* );
SPINNAKERC_API	( *p_spinNodeMapGetNode )( spinNodeMapHandle, const char*,
			spinNodeHandle* );
SPINNAKERC_API	( *p_spinNodeIsAvailable )( spinNodeHandle, bool8_t* );
SPINNAKERC_API	( *p_spinNodeIsReadable )( spinNodeHandle, bool8_t* );
SPINNAKERC_API	( *p_spinStringGetValue )( spinNodeHandle, char*, size_t* );
SPINNAKERC_API	( *p_spinInterfaceGetCameras )( spinInterface, spinCameraList );
SPINNAKERC_API	( *p_spinCameraListGet )( spinCameraList, size_t, spinCamera* );
SPINNAKERC_API	( *p_spinCameraGetTLDeviceNodeMap )( spinCamera,
			spinNodeMapHandle* );
SPINNAKERC_API	( *p_spinCameraRelease )( spinCamera );


#if HAVE_LIBDL
static void*		_getDLSym ( void*, const char* );
#endif

/**
 * Cycle through the list of cameras returned by the Spinnaker library
 */

int
oaSpinGetCameras ( CAMERA_LIST* deviceList, int flags )
{
  spinSystem		systemHandle;
  spinInterfaceList	ifaceListHandle = 0;
  size_t		numInterfaces = 0;
  spinCameraList	cameraListHandle = 0;
  size_t		numCameras = 0;
  spinInterface		ifaceHandle = 0;
  spinNodeMapHandle	ifaceNodeMapHandle = 0;
  spinNodeHandle	ifaceNameHandle = 0;
  bool8_t		ifaceNameAvailable = False;
  bool8_t		ifaceNameReadable = False;
  char			ifaceName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		ifaceNameLen = SPINNAKER_MAX_BUFF_LEN;
  spinCamera		cameraHandle;
  spinNodeMapHandle	cameraNodeMapHandle = 0;
  spinNodeHandle	vendorNameHandle = 0;
  bool8_t		vendorNameAvailable = False;
  bool8_t		vendorNameReadable = False;
  spinNodeHandle	modelNameHandle = 0;
  bool8_t		modelNameAvailable = False;
  bool8_t		modelNameReadable = False;
  char			vendorName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		vendorNameLen = SPINNAKER_MAX_BUFF_LEN;
  char			modelName[ SPINNAKER_MAX_BUFF_LEN ];
  size_t		modelNameLen = SPINNAKER_MAX_BUFF_LEN;
  unsigned int		i, j, numFound;
  oaCameraDevice*       devices;
  DEVICE_INFO*		_private;
  int			ret;


#if HAVE_LIBDL
  static void*		libHandle = 0;

  if ( !libHandle ) {
    if (!( libHandle = dlopen( "libSpinnaker_C.so.1", RTLD_LAZY ))) {
      return 0;
    }
  }

  dlerror();

  if (!( *( void** )( &p_spinSystemGetInstance ) = _getDLSym ( libHandle,
      "spinSystemGetInstance" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListClear ) = _getDLSym ( libHandle,
      "spinCameraListClear" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListCreateEmpty ) = _getDLSym ( libHandle,
      "spinCameraListCreateEmpty" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListDestroy ) = _getDLSym ( libHandle,
      "spinCameraListDestroy" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListGetSize ) = _getDLSym ( libHandle,
      "spinCameraListGetSize" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListClear ) = _getDLSym ( libHandle,
      "spinInterfaceListClear" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListCreateEmpty ) = _getDLSym ( libHandle,
      "spinInterfaceListCreateEmpty" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListDestroy ) = _getDLSym ( libHandle,
      "spinInterfaceListDestroy" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListGetSize ) = _getDLSym ( libHandle,
      "spinInterfaceListGetSize" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinSystemGetCameras ) = _getDLSym ( libHandle,
      "spinSystemGetCameras" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinSystemGetInterfaces ) = _getDLSym ( libHandle,
      "spinSystemGetInterfaces" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinSystemReleaseInstance ) = _getDLSym ( libHandle,
      "spinSystemReleaseInstance" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceListGet ) = _getDLSym ( libHandle,
      "spinInterfaceListGet" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceRelease ) = _getDLSym ( libHandle,
      "spinInterfaceRelease" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceGetTLNodeMap ) = _getDLSym ( libHandle,
      "spinInterfaceGetTLNodeMap" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeMapGetNode ) = _getDLSym ( libHandle,
      "spinNodeMapGetNode" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeIsAvailable ) = _getDLSym ( libHandle,
      "spinNodeIsAvailable" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinNodeIsReadable ) = _getDLSym ( libHandle,
      "spinNodeIsReadable" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinStringGetValue ) = _getDLSym ( libHandle,
      "spinStringGetValue" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinInterfaceGetCameras ) = _getDLSym ( libHandle,
      "spinInterfaceGetCameras" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraListGet ) = _getDLSym ( libHandle,
      "spinCameraListGet" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraGetTLDeviceNodeMap ) = _getDLSym ( libHandle,
      "spinCameraGetTLDeviceNodeMap" ))) {
    return 0;
  }
  if (!( *( void** )( &p_spinCameraRelease ) = _getDLSym ( libHandle,
      "spinCameraRelease" ))) {
    return 0;
  }

#else /* HAVE_LIBDL */

  p_spinSystemGetInstance = spinSystemGetInstance;
  p_spinCameraListClear = spinCameraListClear;
  p_spinCameraListCreateEmpty = spinCameraListCreateEmpty;
  p_spinCameraListDestroy = spinCameraListDestroy;
  p_spinCameraListGetSize = spinCameraListGetSize;
  p_spinInterfaceListClear = spinInterfaceListClear;
  p_spinInterfaceListCreateEmpty = spinInterfaceListCreateEmpty;
  p_spinInterfaceListDestroy = spinInterfaceListDestroy;
  p_spinInterfaceListGetSize = spinInterfaceListGetSize;
  p_spinSystemGetCameras = spinSystemGetCameras;
  p_spinSystemGetInterfaces = spinSystemGetInterfaces;
  p_spinSystemReleaseInstance = spinSystemReleaseInstance;
  p_spinInterfaceListGet = spinInterfaceListGet;
  p_spinInterfaceRelease = spinInterfaceRelease;
  p_spinInterfaceGetTLNodeMap = spinInterfaceGetTLNodeMap;
  p_spinNodeMapGetNode = spinNodeMapGetNode;
  p_spinNodeIsAvailable = spinNodeIsAvailable;
  p_spinNodeIsReadable = spinNodeIsReadable;
  p_spinStringGetValue = spinStringGetValue;
  p_spinInterfaceGetCameras = spinInterfaceGetCameras;
  p_spinCameraListGet = spinCameraListGet;
  p_spinCameraGetTLDeviceNodeMap = spinCameraGetTLDeviceNodeMap;
  p_spinCameraRelease = spinCameraRelease;

#endif /* HAVE_LIBDL */

  if (( *p_spinSystemGetInstance )( &systemHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker system instance\n" );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinInterfaceListCreateEmpty )( &ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't create empty Spinnaker interface list\n" );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinSystemGetInterfaces )( systemHandle, ifaceListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker interfaces\n" );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinInterfaceListGetSize )( ifaceListHandle, &numInterfaces ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get size of Spinnaker interface list\n" );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  fprintf ( stderr, "Spinnaker: %ld interfaces found\n", numInterfaces );

  if ( !numInterfaces ) {
    fprintf ( stderr, "No Spinnaker interfaces found\n" );
    ( void ) ( *p_spinInterfaceListClear  )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy  )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance  )( systemHandle );
    return 0;
  }

  if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't create empty Spinnaker camera list\n" );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinSystemGetCameras )( systemHandle, cameraListHandle ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get Spinnaker camera list\n" );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
      SPINNAKER_ERR_SUCCESS ) {
    fprintf ( stderr, "Can't get size of Spinnaker camera list\n" );
    ( void ) ( *p_spinCameraListClear )( cameraListHandle );
    ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_SYSTEM_ERROR;
  }

  ( void ) ( *p_spinCameraListClear )( cameraListHandle );
  ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
  cameraListHandle = 0;

  fprintf ( stderr, "Spinnaker: %ld cameras found\n", numCameras );

  if ( !numCameras ) {
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return 0;
  }

  if (!( devices = malloc ( sizeof ( oaCameraDevice ) * numCameras ))) {
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_MEM_ALLOC;
  }

  if (!( _private = malloc ( sizeof ( DEVICE_INFO ) * numCameras ))) {
    free (( void* ) devices );
    ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
    ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
    ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
    return -OA_ERR_MEM_ALLOC;
  }

  numFound = 0;
  for ( i = 0; i < numInterfaces; i++ ) {
    if (( *p_spinInterfaceListGet )( ifaceListHandle, i, &ifaceHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker interface from list\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinInterfaceGetTLNodeMap )( ifaceHandle, &ifaceNodeMapHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker TL node map\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinNodeMapGetNode )( ifaceNodeMapHandle, "InterfaceDisplayName",
        &ifaceNameHandle ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker TL map node\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinNodeIsAvailable )( ifaceNameHandle, &ifaceNameAvailable ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker interface name availability\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( ifaceNameAvailable ) {
      if (( *p_spinNodeIsReadable )( ifaceNameHandle, &ifaceNameReadable ) !=
          SPINNAKER_ERR_SUCCESS ) {
        fprintf ( stderr, "Can't get Spinnaker interface name readability\n" );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return -OA_ERR_SYSTEM_ERROR;
      }
      if ( ifaceNameReadable ) {
        if (( *p_spinStringGetValue )( ifaceNameHandle, ifaceName,
            &ifaceNameLen ) != SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr, "Can't get Spinnaker string value\n" );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }
      } else {
        ( void ) strcpy ( ifaceName, "name unreadable" );
      }
    } else {
      ( void ) strcpy ( ifaceName, "name unavailable" );
    }

    if (( *p_spinCameraListCreateEmpty )( &cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't create empty Spinnaker camera list\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinInterfaceGetCameras )( ifaceHandle, cameraListHandle ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker interface camera list\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if (( *p_spinCameraListGetSize )( cameraListHandle, &numCameras ) !=
        SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't get Spinnaker interface camera count\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }

    if ( numCameras ) {
      for ( j = 0; j < numCameras; j++ ) {
        if (( *p_spinCameraListGet )( cameraListHandle, j, &cameraHandle ) !=
            SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr, "Can't get Spinnaker interface camera\n" );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if (( *p_spinCameraGetTLDeviceNodeMap )( cameraHandle,
            &cameraNodeMapHandle ) != SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr, "Can't get Spinnaker camera node map\n" );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle, "DeviceVendorName",
            &vendorNameHandle ) != SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr, "Can't get Spinnaker camera name node\n" );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if (( *p_spinNodeIsAvailable )( vendorNameHandle,
            &vendorNameAvailable ) != SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr,
              "Can't get Spinnaker vendor name availability\n" );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if ( vendorNameAvailable ) {
          if (( *p_spinNodeIsReadable )( vendorNameHandle,
              &vendorNameReadable ) != SPINNAKER_ERR_SUCCESS ) {
            fprintf ( stderr, "Can't get Spinnaker vendor name readability\n" );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return -OA_ERR_SYSTEM_ERROR;
          }
          if ( vendorNameReadable ) {
            if (( *p_spinNodeMapGetNode )( cameraNodeMapHandle,
                "DeviceModelName", &modelNameHandle ) !=
                SPINNAKER_ERR_SUCCESS ) {
              fprintf ( stderr, "Can't get Spinnaker camera model node\n" );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }

            if (( *p_spinStringGetValue )( vendorNameHandle, vendorName,
                &vendorNameLen ) != SPINNAKER_ERR_SUCCESS ) {
              fprintf ( stderr, "Can't get Spinnaker vendor name string\n" );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }
          } else {
            ( void ) strcpy ( vendorName, "vendor unreadable" );
          }
        } else {
          ( void ) strcpy ( vendorName, "vendor unavailable" );
        }

        if (( *p_spinNodeIsAvailable )( modelNameHandle, &modelNameAvailable )
            != SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr,
              "Can't get Spinnaker vendor model availability\n" );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }

        if ( modelNameAvailable ) {
          if (( *p_spinNodeIsReadable )( modelNameHandle, &modelNameReadable )
              != SPINNAKER_ERR_SUCCESS ) {
            fprintf ( stderr,
                "Can't get Spinnaker vendor model readability\n" );
            ( void ) ( *p_spinCameraRelease )( cameraHandle );
            ( void ) ( *p_spinCameraListClear )( cameraListHandle );
            ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
            ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
            ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
            ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
            return -OA_ERR_SYSTEM_ERROR;
          }

          if ( modelNameReadable ) {
            if (( *p_spinStringGetValue )( modelNameHandle, modelName,
                &modelNameLen ) != SPINNAKER_ERR_SUCCESS ) {
              fprintf ( stderr,
                  "Can't get Spinnaker model name string\n" );
              ( void ) ( *p_spinCameraRelease )( cameraHandle );
              ( void ) ( *p_spinCameraListClear )( cameraListHandle );
              ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
              ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
              ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
              ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
              return -OA_ERR_SYSTEM_ERROR;
            }
          } else {
            ( void ) strcpy ( modelName, "model unreadable" );
          }
        } else {
          ( void ) strcpy ( modelName, "model unavailable" );
        }

        _oaInitCameraDeviceFunctionPointers ( &devices[ numFound ]);
        devices[ numFound ].interface = OA_CAM_IF_SPINNAKER;
/*
        memcpy (( void* ) &_private->pgeGuid, ( void* ) &guid, sizeof ( guid ));
*/
        devices[ numFound ]._private = &_private[ numFound ];
        devices[ numFound ].initCamera = oaSpinInitCamera;
        devices[ numFound ].hasLoadableFirmware = 0;
        if (( ret = _oaCheckCameraArraySize ( deviceList )) < 0 ) {
          free (( void* ) devices );
          free (( void* ) _private );
          _oaFreeCameraDeviceList ( deviceList );
          ( void ) ( *p_spinCameraRelease )( cameraHandle );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return ret;
        }
        deviceList->cameraList[ deviceList->numCameras++ ] =
            &devices[ numFound ];
        numFound++;

        fprintf ( stderr, "Interface: %s, Camera: %s %s\n", ifaceName,
            vendorName, modelName );
 
        if (( *p_spinCameraRelease )( cameraHandle ) !=
            SPINNAKER_ERR_SUCCESS ) {
          fprintf ( stderr, "Can't release Spinnaker camera\n" );
          ( void ) ( *p_spinCameraListClear )( cameraListHandle );
          ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
          ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
          ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
          ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
          return -OA_ERR_SYSTEM_ERROR;
        }
      }
      if (( *p_spinCameraListClear )( cameraListHandle ) !=
          SPINNAKER_ERR_SUCCESS ) { 
        fprintf ( stderr, "Can't release Spinnaker camera list\n" );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return -OA_ERR_SYSTEM_ERROR;
      }

      if (( *p_spinCameraListDestroy )( cameraListHandle ) !=
          SPINNAKER_ERR_SUCCESS ) {  
        fprintf ( stderr, "Can't destroy Spinnaker camera list\n" );
        ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
        ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
        ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
        return -OA_ERR_SYSTEM_ERROR;
      }
    } else {
      ( void ) ( *p_spinCameraListClear )( cameraListHandle );
      ( void ) ( *p_spinCameraListDestroy )( cameraListHandle );
      fprintf ( stderr, "Interface %s has no cameras\n",
          ifaceName );
    }

    if (( *p_spinInterfaceRelease )( ifaceHandle ) != SPINNAKER_ERR_SUCCESS ) {
      fprintf ( stderr, "Can't release Spinnaker interface\n" );
      ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
      ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
      ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );
      return -OA_ERR_SYSTEM_ERROR;
    }
  }

  ( void ) ( *p_spinInterfaceListClear )( ifaceListHandle );
  ( void ) ( *p_spinInterfaceListDestroy )( ifaceListHandle );
  ( void ) ( *p_spinSystemReleaseInstance )( systemHandle );

  return numFound;
}


#if HAVE_LIBDL
static void*
_getDLSym ( void* libHandle, const char* symbol )
{
  void* addr;
  char* error;

  addr = dlsym ( libHandle, symbol );
  if (( error = dlerror())) {
    fprintf ( stderr, "spinnaker DL error: %s\n", error );
    addr = 0;
  }

  return addr;
}
#endif