# vim: sw=2

################ common flags and rules ########################################

.DELETE_ON_ERROR:

SHELL := /bin/bash

ifndef CROSS_COMPILE
  ifdef target-arch
    CROSS_COMPILE := ${target-arch}-
  endif
endif

target-flags =

CXX = ${CROSS_COMPILE}g++ ${target-flags}
CC = ${CROSS_COMPILE}gcc ${target-flags}

target-arch != ${CC} -dumpmachine

ifeq "${target-arch}" "arm-linux-gnueabihf"
  # targeting a beaglebone
  target-flags += -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=hard -mthumb
else
  # otherwise probably compiling for use locally on some x86 machine.
  # I considered adding -mx32 to target-flags but it's a hassle with libs.
  ifndef CROSS_COMPILE
    target-flags += -march=native
  endif
endif

LD = ${CC}
LDFLAGS =
LDFLAGS += -z now	# resolve dynamic symbols at start instead of lazily
LDFLAGS += -z relro	# this also allows full RELRO hardening
LDLIBS =

CPPFLAGS =
CPPFLAGS += -iquote src
CPPFLAGS += -iquote include
CPPFLAGS += -D_FILE_OFFSET_BITS=64	# use true off_t instead of compat
CPPFLAGS += -D_FORTIFY_SOURCE=2		# harden against buffer overflows

flags =
flags += -funsigned-char		# is default on arm, set it to be sure
flags += -fno-strict-aliasing -fwrapv	# say yes to sanity
flags += -fstack-protector		# hardening

flags += -Wall -Wextra
flags += -Werror
flags += -Wno-unused-parameter
flags += -Wno-error=unused-function
flags += -Wno-error=unused-variable

flags += -g

flags += -Og				# better for debugging
#flags += -O2 -fno-schedule-insns{,2}	# better for performance
# note: instruction scheduling makes single-stepping really painful

# doubles are expensive on cortex-a8, so if you need fast math consider:
#flags += -ffast-math -fsingle-precision-constant -Wdouble-promotion

CFLAGS = -std=gnu11 ${flags}		# C 2011 + GNU extensions

CXXFLAGS = -std=gnu++1z ${flags}	# C++ 2017 + GNU extensions
CXXFLAGS += -fno-operator-names		# seriously wtf
CXXFLAGS += -Wno-invalid-offsetof	# ANNOYING

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
dep = ${depdir}/$@.d
CPPFLAGS += -MMD -MQ $@ -MQ ${dep} -MP -MF ${dep}

# use them
-include ${depdir}/*.d

# clean them up
clean ::
	${RM} -r ${depdir}

# kill the implicit rules that compile and link in one command since doing so
# confuses dependency generation and isn't supported by distcc.
%: %.c
%: %.cc


################ to check what the compiler is making of your code #############

ifdef use_clang

%.asm: %.c
	$(COMPILE.c) -S -Xclang -masm-verbose $(OUTPUT_OPTION) $<
%.asm: %.cc
	$(COMPILE.cc) -S -Xclang -masm-verbose $(OUTPUT_OPTION) $<

else

%.asm: %.c
	$(COMPILE.c) -S -g0 -fverbose-asm $(OUTPUT_OPTION) $<
%.asm: %.cc
	$(COMPILE.cc) -S -g0 -fverbose-asm $(OUTPUT_OPTION) $<

endif

clean ::
	${RM} *.asm
