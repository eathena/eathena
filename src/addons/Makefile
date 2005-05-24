txt sql all: sample uptime dump pid

COMMON_H = ../common/dll.h
ifdef CYGWIN
	EXT = dll
else
	EXT = so
endif

sample: sample.c $(COMMON_H)
	$(CC) $(CFLAGS) -shared -o ../../addons/$@.$(EXT) $<
uptime: uptime.c $(COMMON_H)
	$(CC) $(CFLAGS) -shared -o ../../addons/$@.$(EXT) $<
dump: dump.c $(COMMON_H)
	$(CC) $(CFLAGS) -shared -o ../../addons/$@.$(EXT) $<
pid: pid.c $(COMMON_H)
	$(CC) $(CFLAGS) -shared -o ../../addons/$@.$(EXT) $<

clean:
	rm -rf *.o
