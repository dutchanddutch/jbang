################ common flags and rules ########################################

SHELL := /bin/bash

CXX = ${CROSS_COMPILE}g++
CXX += -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -mthumb
LDFLAGS =
LDLIBS =
CXXFLAGS = -std=gnu++1y -funsigned-char
CXXFLAGS += -fno-strict-aliasing -fwrapv
CXXFLAGS += -Og
CXXFLAGS += -Wall -Wextra
#CXXFLAGS += -Werror
CXXFLAGS += -Wno-unused-parameter -Wno-error=unused-function
CPPFLAGS = -I . -I include

CXX += -fmax-errors=3

export GCC_COLORS = 1

clean ::
	${RM} *.o


################ package magic #################################################

define declare_pkg =
${pkg}_CFLAGS != pkg-config --cflags ${pkg}
${pkg}_LDLIBS != pkg-config --libs ${pkg}
endef
$(foreach pkg,${declared_pkgs}, $(eval ${declare_pkg}))

CXXFLAGS += $(foreach pkg,${pkgs},${${pkg}_CFLAGS})
LDLIBS += $(foreach pkg,${pkgs},${${pkg}_LDLIBS})


################ automatic header dependencies #################################

# a place to put them out of sight
depdir := .dep
$(shell mkdir -p ${depdir})

# generate them
CPPFLAGS += -MMD -MQ $@ -MP -MF >( cat >${depdir}/$@.d )

# use them
-include .dep/*.d

# clean them up
clean ::
	${RM} -r ${depdir}

# fix built-in rules that bork because they think all deps are sources
%: %.o
	${LINK.o} ${^:%.h=} ${LDLIBS} ${OUTPUT_OPTION}

%: %.c
	${LINK.c} ${^:%.h=} ${LDLIBS} ${OUTPUT_OPTION}

%: %.cc
	${LINK.cc} ${^:%.h=} ${LDLIBS} ${OUTPUT_OPTION}
