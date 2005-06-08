txt sql all: sample uptime dump pid gui

COMMON_H = ../common/dll.h
ifdef CYGWIN
	EXT = dll
else
	EXT = so
endif

%: %.c
	$(CC) $(CFLAGS) -shared -o ../../addons/$@.$(EXT) $<

sample: sample.c $(COMMON_H)
uptime: uptime.c $(COMMON_H)
dump: dump.c $(COMMON_H)
pid: pid.c $(COMMON_H)
gui: gui.c $(COMMON_H)

clean:
	rm -rf *.o
