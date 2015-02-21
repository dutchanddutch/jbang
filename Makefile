programs :=
programs += jbang

all :: ${programs}

clean ::
	${RM} ${programs}


jbang: jbang.cc hw-subarctic.cc map-phys.cc


# where to look for sources
vpath %.cc src


# all packages
declared_pkgs :=

# default packages
pkgs =


include common.mk

CXXFLAGS += -fno-exceptions -fno-rtti -fno-threadsafe-statics
