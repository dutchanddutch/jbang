programs :=

all :: ${programs}

clean ::
	${RM} ${programs}

-include modules.mk

modules.mk: peripherals.txt
	bin/gen-modules $<

peripherals.txt:
	find ${srcdir}include/ -type f -name '*.h' \
		-exec ${topdir}bin/scrape-peripherals {} \+ >$@

all :: libsubarctic.a(${modules} map-phys.o)

${modules}: %.o: %.cc %.ld
	${LINK.cc} -nostdlib -r ${^:%.h=} ${OUTPUT_OPTION}

clean ::
	${RM} hw-* peripherals.txt modules.mk libsubarctic.a

# where to look for sources
vpath %.cc src


# all packages
declared_pkgs :=

# default packages
pkgs =


include common.mk

#CXXFLAGS += -fno-exceptions -fno-rtti -fno-threadsafe-statics
