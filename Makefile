programs :=
programs += jbang

all :: libsubarctic/libsubarctic.a ${programs}

clean ::
	${RM} ${programs}
	${MAKE} -C libsubarctic clean

libsubarctic/libsubarctic.a:
	${MAKE} -C libsubarctic

jbang: jbang.cc hw-subarctic.cc


# where to look for sources
vpath %.cc src


# all packages
declared_pkgs :=

# default packages
pkgs =


include common.mk

LDFLAGS += -L libsubarctic
LDLIBS += -lsubarctic
