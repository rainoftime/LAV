#===-- tools/LAV/Makefile --------------------------------------------------===#
#
#                     The LAV Software Verification Tool
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
#===----------------------------------------------------------------------===#

LEVEL=../..
TOOLNAME = LAV
USEDLIBS = expression.a misc.a core.a transformation.a solvers.a threads.a
LINK_COMPONENTS = support jit bitreader bitwriter ipo linker engine 

include $(LEVEL)/Makefile.common

ifeq ($(ENABLE_BOOLECTOR), 1)
        LIBS += $(PROJ_SRC_ROOT)/solvers/boolector/lib/libboolector.a
        LIBS += $(PROJ_SRC_ROOT)/solvers/boolector/lib/liblgl.a
endif

ifeq ($(ENABLE_BOOLECTOR_OLD), 1)
        LIBS += $(PROJ_SRC_ROOT)/solvers/boolector_old/lib/libboolector.a
endif

ifeq ($(ENABLE_YICES), 1)
        LIBS += $(PROJ_SRC_ROOT)/solvers/yices/lib/libyices.a
endif

ifeq ($(ENABLE_MATHSAT), 1)
        LIBS += $(PROJ_SRC_ROOT)/solvers/mathsat/lib/libmathsat.a
endif

ifeq ($(ENABLE_Z3), 1)
        LIBS += $(PROJ_SRC_ROOT)/solvers/z3/bin/libz3.a
        LIBS += -lrt
endif

LIBS += -lsolvers
LIBS += -lexpression
LIBS += -lgmp  
LIBS += -lgmpxx  
