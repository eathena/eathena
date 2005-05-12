txt sql all: sample

COMMON_H = ../common/dll.h
ifdef CYGWIN
	EXT = dll
else
	EXT = so
endif

sample: sample.c $(COMMON_H)
	$(CC) -shared -o ../../addons/$@.$(EXT) $<

clean:
	rm -rf *.o
