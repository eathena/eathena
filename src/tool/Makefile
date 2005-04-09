all: adduser convert

adduser:
	$(CC) -o ../../tool/$@ adduser.c

convert:
	$(CC) -o ../../tool/$@ convert.c

clean:
	rm -rf ../../tool/adduser ../../tool/convert
