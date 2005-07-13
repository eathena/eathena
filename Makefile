
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

LUAINCLUDE = -I../lua
LUALIB = -L/usr/local/lib -llua -llualib

PACKETDEF = -DPACKETVER=6 -DNEW_006b -DSO_REUSEPORT
#PACKETDEF = -DPACKETVER=5 -DNEW_006b
#PACKETDEF = -DPACKETVER=4 -DNEW_006b
#PACKETDEF = -DPACKETVER=3 -DNEW_006b
#PACKETDEF = -DPACKETVER=2 -DNEW_006b
#PACKETDEF = -DPACKETVER=1 -DNEW_006b

LIBS = -lz -lm

MAKE = make

PLATFORM = $(shell uname)

ifeq ($(findstring Linux,$(PLATFORM)), Linux)
LIBS += -ldl
endif

ifeq ($(findstring SunOS,$(PLATFORM)), SunOS)
LIBS += -lsocket -lnsl -ldl
MAKE = gmake
endif

ifeq ($(findstring FreeBSD,$(PLATFORM)), FreeBSD)
MAKE = gmake
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

MYSQLFLAG_INCLUDE_DEFAULT = /usr/local/include/mysql

ifdef SQLFLAG
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

LIB_S_DEFAULT = -L/usr/local/lib/mysql -lmysqlclient
MYSQLFLAG_CONFIG = $(shell which mysql_config)
ifeq ($(findstring /,$(MYSQLFLAG_CONFIG)), /)
LIB_S = $(shell $(MYSQLFLAG_CONFIG) --libs)
else
LIB_S = $(LIB_S_DEFAULT)
endif

MYLIB = CC="$(CC)" CFLAGS="$(CFLAGS) $(MYSQLFLAG_INCLUDE) $(LUAINCLUDE)" LIB_S="$(LUALIB) $(LIB_S) $(GCLIB) $(LIBS)"

endif

MKDEF = CC="$(CC)" CFLAGS="$(CFLAGS) $(LUAINCLUDE)" LIB_S=" $(LUALIB)$(GCLIB) $(LIBS)"

all: conf txt

.PHONY: txt sql converters addons tools webserver

conf:
	cp -r conf-tmpl conf
	rm -rf conf/.svn conf/*/.svn
	cp -r save-tmpl save
	rm -rf save/.svn

txt : src/common/GNUmakefile src/login/GNUmakefile src/char/GNUmakefile src/map/GNUmakefile src/ladmin/GNUmakefile conf
	cd src ; cd common ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd login ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd char ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd map ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd ladmin ; $(MAKE) $(MKDEF) $@ ; cd ..


ifdef SQLFLAG
sql: src/common/GNUmakefile src/login_sql/GNUmakefile src/char_sql/GNUmakefile src/map/GNUmakefile conf
	cd src ; cd common ; $(MAKE) $(MKDEF) $@ ; cd ..
	cd src ; cd login_sql ; $(MAKE) $(MYLIB) $@ ; cd ..
	cd src ; cd char_sql ; $(MAKE) $(MYLIB) $@ ; cd ..
	cd src ; cd map ; $(MAKE) $(MYLIB) $@ ; cd ..
else
sql: 
	$(MAKE) CC="$(CC)" OPT="$(OPT)" SQLFLAG=1 $@
endif


ifdef SQLFLAG
converters: src/common/GNUmakefile src/txt-converter/GNUmakefile
	cd src ; cd common ; $(MAKE) $(MKDEF) sql ; cd ..
	cd src ; cd txt-converter ; $(MAKE) $(MYLIB) ; cd ..
else
converters:
	$(MAKE) CC="$(CC)" OPT="$(OPT)" SQLFLAG=1 $@
endif

addons:
	cd src ; cd addons && $(MAKE) $(MKDEF) && cd ..

tools: 
	cd src ; cd tool && $(MAKE) $(MKDEF) && cd ..

webserver:
	cd src ; cd webserver && $(MAKE) $(MKDEF) && cd ..


clean: src/common/GNUmakefile src/login/GNUmakefile src/char/GNUmakefile src/map/GNUmakefile src/ladmin/GNUmakefile src/txt-converter/GNUmakefile
	cd src ; cd common ; $(MAKE) $@ ; cd ..
	cd src ; cd login ; $(MAKE) $@ ; cd ..
	cd src ; cd login_sql ; $(MAKE) $@ ; cd ..
	cd src ; cd char ; $(MAKE) $@ ; cd ..
	cd src ; cd char_sql ; $(MAKE) $@ ; cd ..
	cd src ; cd map ; $(MAKE) $@ ; cd ..
	cd src ; cd ladmin ; $(MAKE) $@ ; cd ..
	cd src ; cd addons ; $(MAKE) $@ ; cd ..
	cd src ; cd txt-converter ; $(MAKE) $@ ; cd ..

src/common/GNUmakefile: src/common/Makefile 
	sed -e 's/$$>/$$^/' src/common/Makefile > src/common/GNUmakefile
src/login/GNUmakefile: src/login/Makefile 
	sed -e 's/$$>/$$^/' src/login/Makefile > src/login/GNUmakefile
src/login_sql/GNUmakefile: src/login_sql/Makefile 
	sed -e 's/$$>/$$^/' src/login_sql/Makefile > src/login_sql/GNUmakefile
src/char/GNUmakefile: src/char/Makefile 
	sed -e 's/$$>/$$^/' src/char/Makefile > src/char/GNUmakefile
src/char_sql/GNUmakefile: src/char_sql/Makefile 
	sed -e 's/$$>/$$^/' src/char_sql/Makefile > src/char_sql/GNUmakefile
src/map/GNUmakefile: src/map/Makefile 
	sed -e 's/$$>/$$^/' src/map/Makefile > src/map/GNUmakefile
src/addons/GNUmakefile: src/addons/Makefile 
	sed -e 's/$$>/$$^/' src/addons/Makefile > src/addons/GNUmakefile
src/ladmin/GNUmakefile: src/ladmin/Makefile 
	sed -e 's/$$>/$$^/' src/ladmin/Makefile > src/ladmin/GNUmakefile
src/txt-converter/GNUmakefile: src/txt-converter/Makefile 
	sed -e 's/$$>/$$^/' src/txt-converter/Makefile > src/txt-converter/GNUmakefile
