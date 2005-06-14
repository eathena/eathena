
OBJECTS = sample.dll sig.dll pid.dll gui.dll upnp.dll

ifdef CYGWIN
   EXT = dll
else
   EXT = so
endif

PLUGINS = $(OBJECTS:%.dll=%.$(EXT))
COMMON_H = ../common/plugin.h

txt sql all: $(PLUGINS)


%.$(EXT): %.c
	$(CC) $(CFLAGS) -shared -o ../../plugins/$@ $<
	@touch $@

sample.$(EXT): sample.c $(COMMON_H)
sig.$(EXT): sig.c $(COMMON_H)
pid.$(EXT): pid.c $(COMMON_H)
gui.$(EXT): ../../plugins/gui.conf gui.c $(COMMON_H)
upnp.$(EXT): ../../plugins/upnp.conf


../../plugins/%.conf: %.txt
	cp -r $< $@

../../plugins/gui.conf: gui.txt
../../plugins/upnp.conf: upnp.txt

clean:
	rm -rf $(PLUGINS)
