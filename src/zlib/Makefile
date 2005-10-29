
OBJS = unzip.o ioapi.o

.c.o:
	$(CC) -c $(CFLAGS) $*.c

all: unz.o

unz.o: $(OBJS)
	ar rcs $@ $(OBJS)

clean:
	rm -f *.o

# DO NOT DELETE
ioapi.o: ioapi.h
unzip.o: unzip.h ioapi.h crypt.h