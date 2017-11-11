programs :=
programs += jbang

all :: libsubarctic/libsubarctic.a ${programs}

clean ::
	${RM} ${programs}
	${MAKE} -C libsubarctic clean

libsubarctic/libsubarctic.a:
	${MAKE} -C libsubarctic

jbang: hw-subarctic.o


# where to look for sources
vpath %.cc src


# all packages
declared_pkgs :=

# default packages
pkgs =


target-arch := arm-linux-gnueabihf

include common.mk

LDFLAGS += -L libsubarctic
LDLIBS += -lsubarctic
