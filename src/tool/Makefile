all: adduser convert

adduser:
	$(CC) -o ../../tools/$@ adduser.c

convert:
	$(CC) -o ../../tools/$@ convert.c

clean:
	rm -rf *.o ../../tools/adduser ../../tools/convert
