
CC = gcc -pipe
# CC = gcc -pipe -DCHRIF_OLDINFO
# CC = gcc -pipe -DPCRE_SUPPORT
# CC = g++ --pipe
# CC = gcc -pipe -DGCOLLECT
# CC = gcc -pipe -DMEMWATCH
# CC = gcc -pipe -DDMALLOC -DDMALLOC_FUNC_CHECK
# CC = /usr/local/bin/gcc -fbounds-checking -pipe -DBCHECK

# GCLIB = -lgc
# GCLIB = -L/usr/local/lib -lpcre
GCLIB =
# GCLIB = -ldmalloc

PACKETDEF = -DPACKETVER=6 -DNEW_006b -DSO_REUSEPORT -DOLD_LOGIN
#PACKETDEF = -DPACKETVER=5 -DNEW_006b
#PACKETDEF = -DPACKETVER=4 -DNEW_006b
#PACKETDEF = -DPACKETVER=3 -DNEW_006b
#PACKETDEF = -DPACKETVER=2 -DNEW_006b
#PACKETDEF = -DPACKETVER=1 -DNEW_006b

LIBS = -lz -lm

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)), Linux)
	LIBS += -ldl
endif

ifeq ($(findstring SunOS,$(PLATFORM)), SunOS)
	LIBS += -lsocket -lnsl -ldl
endif

ifeq ($(findstring FreeBSD,$(PLATFORM)), FreeBSD)
	MAKE = gmake
else
	MAKE = make
endif

ifeq ($(findstring NetBSD,$(PLATFORM)), NetBSD)
	MAKE = gmake
	CC += -D__NETBSD__
endif

OPT = -g -O2 -ffast-math -Wall -Wno-sign-compare
# OPT += -DDUMPSTACK -rdynamic

ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
	OS_TYPE = -DCYGWIN
	CFLAGS =  $(OPT) -DFD_SETSIZE=4096 -I../common $(PACKETDEF) $(OS_TYPE)

else
	OS_TYPE =
	CFLAGS =  $(OPT) -I../common $(PACKETDEF) $(OS_TYPE)
	# CFLAGS = -DTWILIGHT  $(OPT) -Wall -I../common $(PACKETDEF) $(OS_TYPE)
endif

MYSQLFLAG_INCLUDE_DEFAULT = /usr/include/mysql/

MYSQLFLAG_CONFIG = $(shell which mysql_config)
ifeq ($(findstring /,$(MYSQLFLAG_CONFIG)), /)
	MYSQLFLAG_VERSION = $(shell $(MYSQLFLAG_CONFIG) --version | sed s:\\..*::) 
endif

ifeq ($(findstring 4,$(MYSQLFLAG_VERSION)), 4)
	MYSQLFLAG_CONFIG_ARGUMENT = --cflags
endif
ifeq ($(findstring 5,$(MYSQLFLAG_VERSION)), 5)
MYSQLFLAG_CONFIG_ARGUMENT = --include
endif

ifndef MYSQLFLAG_CONFIG_ARGUMENT
	MYSQLFLAG_CONFIG_ARGUMENT = --cflags
endif

ifeq ($(findstring /,$(MYSQLFLAG_CONFIG)), /)
	MYSQLFLAG_INCLUDE = $(shell $(MYSQLFLAG_CONFIG) $(MYSQLFLAG_CONFIG_ARGUMENT))
else
	MYSQLFLAG_INCLUDE = -I$(MYSQLFLAG_INCLUDE_DEFAULT)
endif

LIB_S = -L/usr/local/lib/mysql -lmysqlclient
MYSQLFLAG_CONFIG = $(shell which mysql_config)
ifeq ($(findstring /,$(MYSQLFLAG_CONFIG)), /)
	LIB_S = $(shell $(MYSQLFLAG_CONFIG) --libs)
endif

MYLIB = CC="$(CC)" CFLAGS="$(CFLAGS) $(MYSQLFLAG_INCLUDE)" LIB_S="$(LIB_S) $(GCLIB) $(LIBS)"


all: src/common/GNUmakefile src/login_sql/GNUmakefile
	cd src ; cd common ; $(MAKE) $(MYLIB) $@ ; cd ..
	cd src ; cd login_sql; $(MAKE) $(MYLIB) $@ ; cd ..

clean: src/common/GNUmakefile src/login_sql/GNUmakefile
	cd src ; cd common ; $(MAKE) $@ ; cd ..
	cd src ; cd login_sql ; $(MAKE) $@ ; cd ..


src/common/GNUmakefile: src/common/Makefile 
	sed -e 's/$$>/$$^/' src/common/Makefile > src/common/GNUmakefile

src/login_sql/GNUmakefile: src/login_sql/Makefile 
	sed -e 's/$$>/$$^/' src/login_sql/Makefile > src/login_sql/GNUmakefile

