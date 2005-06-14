
OBJECTS = sample.dll sig.dll pid.dll gui.dll upnp.dll

ifdef CYGWIN
   EXT = dll
else
   EXT = so
endif

ADDONS = $(OBJECTS:%.dll=%.$(EXT))
COMMON_H = ../common/dll.h

txt sql all: $(ADDONS)


%.$(EXT): %.c
	$(CC) $(CFLAGS) -shared -o ../../addons/$@ $<
	@touch $@

sample.$(EXT): sample.c $(COMMON_H)
sig.$(EXT): sig.c $(COMMON_H)
pid.$(EXT): pid.c $(COMMON_H)
gui.$(EXT): ../../addons/gui.conf gui.c $(COMMON_H)
upnp.$(EXT): ../../addons/upnp.conf


../../addons/%.conf: %.txt
	cp -r $< $@

../../addons/gui.conf: gui.txt
../../addons/upnp.conf: upnp.txt

clean:
	rm -rf $(ADDONS)
