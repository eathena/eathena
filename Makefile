CCC = g++ -pipe
OPT = -g -O3 -ffast-math -D_REENTRANT
LIBS =  -lpthread -lz

##########################################################
PLATFORM = $(shell uname)
ifeq ($(findstring Linux,$(PLATFORM)), Linux)
   LIBS += -ldl
endif
ifeq ($(findstring SunOS,$(PLATFORM)), SunOS)
   LIBS += -lsocket -lnsl -ldl -lrt
endif
ifeq ($(findstring FreeBSD,$(PLATFORM)), FreeBSD)
   MAKE = gmake
endif
ifeq ($(findstring NetBSD,$(PLATFORM)), NetBSD)
   MAKE = gmake
   OS_TYPE = -D__NETBSD__
endif
ifeq ($(findstring CYGWIN,$(PLATFORM)), CYGWIN)
   OS_TYPE = -DCYGWIN
endif

##########################################################
# set some path variables for convenience
RETURN_PATH = ../..
BASE_PATH   = src/basics
COMMON_PATH = src/common

##########################################################
CPPFLAGS = $(OPT) -I$(RETURN_PATH)/$(COMMON_PATH) -I$(RETURN_PATH)/$(BASE_PATH) $(OS_TYPE)

##########################################################
# my defaults for mysql libs
MYSQL_INCLUDE = -I../../../mysql/include/ 
MYSQL_LIB     = -L../../../mysql/lib -lmysqlclient -lposix4 -lcrypt -lgen -lnsl -lm

##########################################################
ifdef SQLFLAG
MYSQLFLAG_CONFIG = $(shell which mysql_config)

ifeq ($(findstring /mysql_config,$(MYSQLFLAG_CONFIG)), /mysql_config)

MYSQLFLAG_VERSION = $(shell $(MYSQLFLAG_CONFIG) --version | sed s:\\..*::) 

ifeq ($(findstring 4,$(MYSQLFLAG_VERSION)), 4)
MYSQLFLAG_CONFIG_ARGUMENT = --cflags
endif

ifeq ($(findstring 5,$(MYSQLFLAG_VERSION)), 5)
MYSQLFLAG_CONFIG_ARGUMENT = --include
endif

ifndef MYSQLFLAG_CONFIG_ARGUMENT
MYSQLFLAG_CONFIG_ARGUMENT = --cflags
endif

MYSQL_INCLUDE = $(shell $(MYSQLFLAG_CONFIG) $(MYSQLFLAG_CONFIG_ARGUMENT)) 
MYSQL_LIB     = $(shell $(MYSQLFLAG_CONFIG) --libs)
endif

MKDEF = CCC="$(CCC)" CPPFLAGS="$(CPPFLAGS) $(MYSQL_INCLUDE)" LIBS="$(MYSQL_LIB) $(LIBS) $(GCLIB)" RETURN_PATH="$(RETURN_PATH)" BASE_PATH="$(BASE_PATH)" COMMON_PATH="$(COMMON_PATH)"
else
MKDEF = CCC="$(CCC)" CPPFLAGS="$(CPPFLAGS) -DTXT_ONLY" LIBS="$(LIBS) $(GCLIB)" RETURN_PATH="$(RETURN_PATH)" BASE_PATH="$(BASE_PATH)" COMMON_PATH="$(COMMON_PATH)"
endif


all: txt sql

conf:
	cp -r conf-tmpl conf
	rm -rf conf/.svn conf/*/.svn

txt : conf basics common login char map ladmin scriptchk

.PHONY : basics
basics: src/basics/GNUmakefile
	cd $(BASE_PATH) ; $(MAKE) $(MKDEF) all ; cd $(RETURN_PATH)

.PHONY : common
common: basics src/common/GNUmakefile
	cd $(COMMON_PATH) ; $(MAKE) $(MKDEF) txt ; cd $(RETURN_PATH)

login: basics common src/login/GNUmakefile
	cd src/login ; $(MAKE) $(MKDEF) txt ; cd $(RETURN_PATH)

char: basics common src/char/GNUmakefile
	cd src/char ; $(MAKE) $(MKDEF) txt ; cd $(RETURN_PATH)

map: basics common src/map/GNUmakefile 
	cd src/map ; $(MAKE) $(MKDEF) txt ; cd $(RETURN_PATH)

ladmin: basics common src/ladmin/GNUmakefile
	cd src/ladmin ; $(MAKE) $(MKDEF) all ; cd $(RETURN_PATH)

scriptchk: basics src/scriptchk/GNUmakefile
	cd src/scriptchk ; $(MAKE) $(MKDEF) all ; cd $(RETURN_PATH)

ifdef SQLFLAG
sql : conf basics common_sql login_sql char_sql map_sql scriptchk

.PHONY : common_sql
common_sql: basics src/common/GNUmakefile
	cd $(COMMON_PATH) ; $(MAKE) $(MKDEF) sql ; cd $(RETURN_PATH)

login_sql: basics common_sql src/login/GNUmakefile
	cd src/login ; $(MAKE) $(MKDEF) sql ; cd $(RETURN_PATH)

char_sql: basics common_sql src/char/GNUmakefile
	cd src/char ; $(MAKE) $(MKDEF) sql ; cd $(RETURN_PATH)

map_sql: basics common_sql src/map/GNUmakefile
	cd src/map ; $(MAKE) $(MKDEF) sql ; cd $(RETURN_PATH)

else

sql common_sql login_sql char_sql map_sql:
	$(MAKE) SQLFLAG=1 $@
endif

clean: src/basics/GNUmakefile src/common/GNUmakefile src/login/GNUmakefile src/char/GNUmakefile src/map/GNUmakefile src/ladmin/GNUmakefile src/scriptchk/GNUmakefile
	cd src/basics ; $(MAKE) $(MKDEF) $@ ; cd $(RETURN_PATH)
	cd src/common ; $(MAKE) $(MKDEF) $@ ; cd $(RETURN_PATH)
	cd src/login ; $(MAKE) $(MKDEF) $@ ; cd $(RETURN_PATH)
	cd src/char ; $(MAKE) $(MKDEF) $@ ; cd $(RETURN_PATH)
	cd src/map ; $(MAKE) $(MKDEF) $@ ; cd $(RETURN_PATH)
	cd src/ladmin ; $(MAKE) $(MKDEF) $@ ; cd $(RETURN_PATH)
	cd src/scriptchk ; $(MAKE) $(MKDEF) $@ ; cd $(RETURN_PATH)


src/basics/GNUmakefile: src/basics/Makefile
	sed -e 's/$$>/$$^/' src/basics/Makefile > src/basics/GNUmakefile
src/common/GNUmakefile: src/common/Makefile
	sed -e 's/$$>/$$^/' src/common/Makefile > src/common/GNUmakefile
src/login/GNUmakefile: src/login/Makefile
	sed -e 's/$$>/$$^/' src/login/Makefile > src/login/GNUmakefile
src/char/GNUmakefile: src/char/Makefile
	sed -e 's/$$>/$$^/' src/char/Makefile > src/char/GNUmakefile
src/map/GNUmakefile: src/map/Makefile
	sed -e 's/$$>/$$^/' src/map/Makefile > src/map/GNUmakefile
src/ladmin/GNUmakefile: src/ladmin/Makefile
	sed -e 's/$$>/$$^/' src/ladmin/Makefile > src/ladmin/GNUmakefile
src/scriptchk/GNUmakefile: src/scriptchk/Makefile
	sed -e 's/$$>/$$^/' src/scriptchk/Makefile > src/scriptchk/GNUmakefile
