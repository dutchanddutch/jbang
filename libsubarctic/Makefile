programs :=

all :: ${programs}

clean ::
	${RM} ${programs}

-include modules.mk

modules.mk: peripherals.txt
	bin/gen-modules $<

peripherals.txt:
	find ${srcdir}include/ti/subarctic -type f -name '*.h' \
		-exec ${topdir}bin/scrape-peripherals {} \+ >$@

all :: modules.mk libsubarctic.a

libsubarctic.a: ${modules} map-phys.o
	${RM} $@
	$(AR) qsU $@ $^

clean ::
	${RM} hw-* peripherals.txt modules.mk libsubarctic.a

# where to look for sources
vpath %.cc src


# all packages
declared_pkgs :=

# default packages
pkgs =


target-arch := arm-linux-gnueabihf
include common.mk

CXXFLAGS += -fno-exceptions -fno-rtti -fno-threadsafe-statics
