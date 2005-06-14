
STATICLIB = libz.a
SHAREDLIB = zlib.dll

OBJS = adler32.o compress.o crc32.o deflate.o gzio.o infback.o \
       inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o

all: $(SHAREDLIB) $(STATICLIB)

$(SHAREDLIB): zlib.def $(OBJS)
	dllwrap --driver-name $(CC) --def zlib.def -o ../../$@ $(OBJS)
	strip ../../$@

$(STATICLIB): $(OBJS)
	ar rcs ../common/$@ $(OBJS)

clean:
	rm -f *.o ../../$(SHAREDLIB) ../common/$(STATICLIB)

# DO NOT DELETE

adler32.o: zlib.h zconf.h
compress.o: zlib.h zconf.h
crc32.o: zutil.h zlib.h zconf.h crc32.h
deflate.o: deflate.h zutil.h zlib.h zconf.h
gzio.o: zutil.h zlib.h zconf.h
infback.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h
inffast.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h
inflate.o: zutil.h zlib.h zconf.h inftrees.h inflate.h inffast.h inffixed.h
inftrees.o: zutil.h zlib.h zconf.h inftrees.h
trees.o: deflate.h zutil.h zlib.h zconf.h trees.h
uncompr.o: zlib.h zconf.h
zutil.o: zutil.h zlib.h zconf.h