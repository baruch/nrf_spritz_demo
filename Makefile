TARGET=_target_sdcc_nrf24le1_32
CCFLAGS=-Isdk/include --std-c99 -I sdk/$(TARGET)/include/ --opt-code-size --model-large
LDFLAGS= -Lsdk/$(TARGET)/lib -lnrf24le1
PROGRAMS = spritz.ihx
SOURCES = main.c spritz.c
OBJECTS = ${SOURCES:c=rel}
LIBNRF = sdk/$(TARGET)/lib/nrf24le1.lib

all: ${PROGRAMS} tags

#-include .deps

#.deps: ${SOURCES}
#	sdcc $(CCFLAGS) -M $(SOURCES) > .deps.tmp
#	sed -i -e 's/.rel:/.ihx:/' .deps.tmp
#	mv .deps.tmp .deps

${PROGRAMS}: ${OBJECTS} ${LIBNRF}
	sdcc $(CCFLAGS) $(LDFLAGS) -o $@ $^

%.rel: %.c
	sdcc $(CCFLAGS) -c -o $@ $<

$(LIBNRF):
	make -C sdk all

# This also tags the SDK but we don't depend on it as it will be too noisy to do so and it rarely changes
tags: main.c
	ctags -R

clean:
	rm -rf  *.asm  *.cdb  *.ihx  *.lk  *.lst  *.map  *.mem  *.omf  *.rel  *.rst  *.sym .deps tags

.PHONY: all clean
