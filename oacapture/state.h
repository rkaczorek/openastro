/*****************************************************************************
 *
 * state.h -- global application state datastructures
 *
 * Copyright 2013,2014,2015,2016,2017,2018
 *     James Fidell (james@openastroproject.org)
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

#pragma once

#include "mainWindow.h"
#include "controlWidget.h"
#include "previewWidget.h"
#include "captureWidget.h"
#include "imageWidget.h"
#include "zoomWidget.h"
#include "histogramWidget.h"
#include "settingsWidget.h"
#include "focusOverlay.h"
#include "advancedSettings.h"


typedef struct
{
  MainWindow*		mainWindow;
  ControlWidget*	controlWidget;
  PreviewWidget*	previewWidget;
  CaptureWidget*	captureWidget;
  ImageWidget*		imageWidget;
  ZoomWidget*		zoomWidget;
  CameraWidget*		cameraWidget;
  int			histogramOn;
  HistogramWidget*	histogramWidget;
  SettingsWidget*	settingsWidget;
  AdvancedSettings*	advancedSettings;
  FocusOverlay*		focusOverlay;

  int			generalSettingsIndex;
  int			captureSettingsIndex;
  int			cameraSettingsIndex;
  int			profileSettingsIndex;
  int			filterSettingsIndex;
  int			autorunSettingsIndex;
  int			histogramSettingsIndex;
  int			demosaicSettingsIndex;
  int			fitsSettingsIndex;
  int			timerSettingsIndex;

  int			autorunEnabled;
  int			autorunRemaining;
  unsigned long		autorunStartNext;
  int			pauseEnabled;
  int			captureWasPaused;

  QString		lastRecordedFile;
  QString		currentDirectory;

  unsigned int		needGroupBoxBorders;

  unsigned long		firstFrameTime;
  unsigned long		lastFrameTime;
  double		currentFPS;

  int			preferredExposureControl;

  QString		appPath;

  int			cameraTempValid;
  float			cameraTemp;
  int			binningValid;
  int			binModeX;
  int			binModeY;

  int			gpsValid;
  double		longitude;
  double		latitude;
  double		altitude;

  int       cropMode;
  unsigned int  cropSizeX;
  unsigned int  cropSizeY;
  unsigned int  sensorSizeX;
  unsigned int  sensorSizeY;
} STATE;

extern STATE		state;

#define TOP_WIDGET ( state.settingsWidget ? state.settingsWidget->getTabset() :\
    ( state.mainWindow ? ( QWidget* ) state.mainWindow : ( QWidget* ) this ))
