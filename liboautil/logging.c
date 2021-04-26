/*****************************************************************************
 *
 * logging.c -- Handle logging
 *
 * Copyright 2021 James Fidell (james@openastroproject.org)
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <oa_common.h>
#include <openastro/util.h>

static unsigned int oaLogLevel = OA_LOG_NONE;
static unsigned int oaLogType = OA_LOG_NONE;
static unsigned int	oaLogToStderr = 1;
static char oaLogFile[ PATH_MAX+1 ];


void
oaSetLogLevel ( unsigned int logLevel )
{
	if ( logLevel > OA_LOG_DEBUG ) {
		logLevel = OA_LOG_DEBUG;
	}
	oaLogLevel = logLevel;
}


int
oaSetLogType ( unsigned int logType )
{
	if ( logType > OA_LOG_TYPE_MAX ) {
		return -OA_ERR_OUT_OF_RANGE;
	}
	oaLogType = logType;
	return OA_ERR_NONE;
}


int
oaAddLogType ( unsigned int logType )
{
	unsigned int n;

	n = oaLogType | logType;
	if ( n > OA_LOG_TYPE_MAX ) {
		return -OA_ERR_OUT_OF_RANGE;
	}
	oaLogType = n;
	return OA_ERR_NONE;
}


int
RemoveLogType ( unsigned int logType )
{
	oaLogType &= ~logType;
	return OA_ERR_NONE;
}


int
oaSetLogFile ( const char* logFile )
{
	FILE*		fp;

	if ( !strcmp ( logFile, "-" )) {
		oaLogToStderr = 1;
		return OA_ERR_NONE;
	}
	if (!( fp = fopen ( logFile, "a" ))) {
		return -OA_ERR_NOT_WRITEABLE;
	}

	fclose ( fp );
	return OA_ERR_NONE;
}


static int
_oaWriteLog ( unsigned int logLevel, char logLetter, unsigned int logType,
		int newline, const char* str, va_list args )
{
	FILE*		fp;

	if ( oaLogLevel >= logLevel && ( oaLogType & logType )) {
		if ( oaLogToStderr ) {
			fp = stderr;
		} else {
			if (!( fp = fopen ( oaLogFile, "a" ))) {
				return -OA_ERR_NOT_WRITEABLE;
			}
		}
		if ( logLetter ) {
			fprintf ( fp, "[%c] ", logLetter );
		}
		vfprintf ( fp, str, args );
		if ( newline ) {
			fprintf ( fp, "\n" );
			fflush ( fp );
		}
		if ( !oaLogToStderr ) {
			fclose ( fp );
		}
	}
	return OA_ERR_NONE;
}


static int
_oaWriteNL ( unsigned int logLevel, char logLetter, unsigned int logType )
{
	FILE*		fp;

	if ( oaLogLevel >= logLevel && ( oaLogType & logType )) {
		if ( oaLogToStderr ) {
			fp = stderr;
		} else {
			if (!( fp = fopen ( oaLogFile, "a" ))) {
				return -OA_ERR_NOT_WRITEABLE;
			}
		}
		if ( logLetter ) {
			fprintf ( fp, "[%c] ", logLetter );
		}
		fprintf ( fp, "\n" );
		fflush ( fp );
		if ( !oaLogToStderr ) {
			fclose ( fp );
		}
	}
	return OA_ERR_NONE;
}


int
oaLogError ( unsigned int logType, const char* str, ... )
{
	va_list	args;
	int			ret;

	va_start ( args, str );
	ret = _oaWriteLog ( OA_LOG_ERROR, 'E', 1, logType, str, args );
	va_end ( args );
	return ret;
}


int
oaLogWarning ( unsigned int logType, const char* str, ... )
{
	va_list	args;
	int			ret;

	va_start ( args, str );
	ret = _oaWriteLog ( OA_LOG_WARN, 'W', 1, logType, str, args );
	va_end ( args );
	return ret;
}


int
oaLogInfo ( unsigned int logType, const char* str, ... )
{
	va_list	args;
	int			ret;

	va_start ( args, str );
	ret = _oaWriteLog ( OA_LOG_INFO, 'I', 1, logType, str, args );
	va_end ( args );
	return ret;
}


int
oaLogDebug ( unsigned int logType, const char* str, ... )
{
	va_list	args;
	int			ret;

	va_start ( args, str );
	ret = _oaWriteLog ( OA_LOG_DEBUG, 'D', 1, logType, str, args );
	va_end ( args );
	return ret;
}


int
oaLogDebugNoNL ( unsigned int logType, const char* str, ... )
{
	va_list	args;
	int			ret;

	va_start ( args, str );
	ret = _oaWriteLog ( OA_LOG_DEBUG, 'D', 0, logType, str, args );
	va_end ( args );
	return ret;
}


int
oaLogDebugCont ( unsigned int logType, const char* str, ... )
{
	va_list	args;
	int			ret;

	va_start ( args, str );
	ret = _oaWriteLog ( OA_LOG_DEBUG, 0, 0, logType, str, args );
	va_end ( args );
	return ret;
}


int
oaLogDebugEndline ( unsigned int logType )
{
	return _oaWriteNL ( OA_LOG_DEBUG, 0, logType );
}
