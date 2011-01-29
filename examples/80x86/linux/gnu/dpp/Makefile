##############################################################################
# Product: DPP example, Linux/P-threads, g++
# Last Updated for Version: 4.0.00
# Date of the Last Update:  Apr 07, 2008
#
#                    Q u a n t u m     L e a P s
#                    ---------------------------
#                    innovating embedded systems
#
# Copyright (C) 2002-2008 Quantum Leaps, LLC. All rights reserved.
#
# This software may be distributed and modified under the terms of the GNU
# General Public License version 2 (GPL) as published by the Free Software
# Foundation and appearing in the file GPL.TXT included in the packaging of
# this file. Please note that GPL Section 2[b] requires that all works based
# on this software must also be made publicly available under the terms of
# the GPL ("Copyleft").
#
# Alternatively, this software may be distributed and modified under the
# terms of Quantum Leaps commercial licenses, which expressly supersede
# the GPL and are specifically designed for licensees interested in
# retaining the proprietary status of their code.
#
# Contact information:
# Quantum Leaps Web site:  http://www.quantum-leaps.com
# e-mail:                  info@quantum-leaps.com
##############################################################################
# general utilities
RM = rm

# The g++ compiler and linker
CC   = g++
LINK = g++

QP_INCDIR = ../../../../../include
QP_PRTDIR = ../../../../../ports/80x86/linux/gnu
QP_LIBDIR = $(QP_PRTDIR)/$(BINDIR)
BLDDIR    = .
LINKFLAGS =

CCINC    := -I$(QP_INCDIR) -I$(QP_PRTDIR) -I$(BLDDIR)
QP_DEP   := $(QP_INCDIR)/qassert.h \
	$(QP_INCDIR)/qep.h \
	$(QP_PRTDIR)/qep_port.h \
	$(QP_INCDIR)/qf.h \
	$(QP_PRTDIR)/qf_port.h

APP_DEP  := $(BLDDIR)/dpp.h \
	$(BLDDIR)/bsp.h

# dbg (default target) .......................................................

BINDIR   = dbg

# g++ options
# -c    : compile only
# -g    : source level debugging
CCFLAGS  = -g -c -Wall -o$@

# release ....................................................................
ifeq ($(MAKECMDGOALS), rel)

BINDIR   = rel

# g++ options
# -c    : compile only
# -O    : optimize
CCFLAGS  = -O -c -Wall -o$@

endif
ifeq ($(MAKECMDGOALS), rel_clean)

BINDIR   = rel

endif

# spy ........................................................................
ifeq ($(MAKECMDGOALS), spy)

BINDIR   = spy

# g++ options
# -c    : compile only
# -g    : source level debugging
# -D    : define preprocessor macro
CCFLAGS  = -g -c -Wall -DQ_SPY -o$@

QS_LIB  := $(QP_LIBDIR)/libqs.a

endif
ifeq ($(MAKECMDGOALS), spy_clean)

BINDIR   = spy

endif

#.............................................................................
$(BINDIR)/dpp : \
	$(BINDIR)/main.o \
	$(BINDIR)/bsp.o \
	$(BINDIR)/philo.o \
	$(BINDIR)/table.o \
	$(QP_LIBDIR)/libqep.a \
	$(QP_LIBDIR)/libqf.a
	$(LINK) $(LINKFLAGS) \
	-o $(BINDIR)/dpp \
	$(BINDIR)/main.o \
	$(BINDIR)/bsp.o \
	$(BINDIR)/philo.o \
	$(BINDIR)/table.o \
	$(QP_LIBDIR)/libqf.a \
	$(QP_LIBDIR)/libqep.a \
	$(QS_LIB) \
	-lpthread

$(BINDIR)/main.o: $(BLDDIR)/main.cpp $(APP_DEP) $(QP_DEP)
	$(CC) $(CCFLAGS) $(CCINC) $<

$(BINDIR)/bsp.o: $(BLDDIR)/bsp.cpp $(APP_DEP) $(APP_DEP) $(QP_DEP)
	$(CC) $(CCFLAGS) $(CCINC) $<

$(BINDIR)/philo.o: $(BLDDIR)/philo.cpp $(APP_DEP) $(QP_DEP)
	$(CC) $(CCFLAGS) $(CCINC) $<

$(BINDIR)/table.o: $(BLDDIR)/table.cpp $(APP_DEP) $(QP_DEP)
	$(CC) $(CCFLAGS) $(CCINC) $<


dbg: $(BINDIR)/dpp

rel: $(BINDIR)/dpp

spy: $(BINDIR)/dpp

# clean targets...

.PHONY: clean rel_clean spy_clean

rel_clean: clean

spy_clean: clean

dbg_clean: clean

clean:
	-$(RM) $(BINDIR)/*.o
	-$(RM) $(BINDIR)/*.d
	-$(RM) $(BINDIR)/dpp
