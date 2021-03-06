#
# Makefile.am -- liboafilterwheel Makefile template
#
# Copyright 2014,2015,2017,2018,2021
#   James Fidell (james@openastroproject.org)
#
# License:
#
# This file is part of the Open Astro Project.
#
# The Open Astro Project is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The Open Astro Project is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the Open Astro Project.  If not, see
# <http://www.gnu.org/licenses/>.
#

AM_CPPFLAGS = \
  -I$(top_srcdir)/include \
  -I$(top_srcdir)/ext/libEFWFilter/include \
  $(LIBHIDAPI_CFLAGS) \
  $(LIBUSB_CFLAGS) \
  -I./sx -I./xagyl -I./zwo -I./brightstar \
  $(DYNLIB_EXTN_CFLAGS)

lib_LTLIBRARIES = liboafilterwheel.la

if LIBUDEV_COND
XAGYLDIR = xagyl
XAGYLLIB = $(XAGYLDIR)/libxagyl.la
BRIGHTSTARDIR = brightstar
BRIGHTSTARLIB = $(BRIGHTSTARDIR)/libbrightstar.la
endif

if LIBFTDI_COND
XAGYLDIR = xagyl
XAGYLLIB = $(XAGYLDIR)/libxagyl.la
endif

if LIBZWOFW_COND
ZWODIR = zwo
ZWOLIB = $(ZWODIR)/libzwo.la
endif

SUBDIRS = sx $(XAGYLDIR) $(ZWODIR) $(BRIGHTSTARDIR)

liboafilterwheel_la_SOURCES = \
	oafw.c unimplemented.c utils.c genericControl.c genericCallback.c

liboafilterwheel_la_LIBADD = \
  sx/libsx.la $(XAGYLLIB) $(ZWOLIB) $(BRIGHTSTARLIB)

WARNINGS = -g -O -Wall -Werror -Wpointer-arith -Wuninitialized -Wsign-compare -Wformat-security -Wno-pointer-sign $(OSX_WARNINGS)

warnings:
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=0 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)

verbose-warnings:
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)'
	$(MAKE) V=1 CFLAGS='$(WARNINGS)' CXXFLAGS='$(WARNINGS)' $(check_PROGRAMS)
