# Microsoft Developer Studio Project File - Name="txt_converter_char" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=txt_converter_char - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "txt-converter-char.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "txt-converter-char.mak" CFG="txt_converter_char - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "txt_converter_char - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "txt_converter_char - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".."
# PROP Intermediate_Dir "tmp\txt_converter_char\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "..\3rdparty\msinttypes\include" /I "..\3rdparty\mysql\include" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WIN32" /D "__WIN32" /D "DB_MANUAL_CAST_TO_UNION" /D "MINICORE" /D "TXT_SQL_CONVERT" /D "WITH_SQL" /D "WITH_TXT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x405 /d "NDEBUG"
# ADD RSC /l 0x417 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib libmysql.lib /nologo /subsystem:console /machine:I386 /libpath:"..\3rdparty\mysql\lib"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".."
# PROP Intermediate_Dir "tmp\txt_converter_char\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /ZI /Od /I "..\3rdparty\msinttypes\include" /I "..\3rdparty\mysql\include" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WIN32" /D "__WIN32" /D "DB_MANUAL_CAST_TO_UNION" /D "MINICORE" /D "TXT_SQL_CONVERT" /D "WITH_SQL" /D "WITH_TXT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x405 /d "_DEBUG"
# ADD RSC /l 0x417 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib libmysql.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\3rdparty\mysql\lib"

!ENDIF 

# Begin Target

# Name "txt_converter_char - Win32 Release"
# Name "txt_converter_char - Win32 Debug"
# Begin Group "common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\common\cbasetypes.h
# End Source File
# Begin Source File

SOURCE=..\src\common\core.c
# End Source File
# Begin Source File

SOURCE=..\src\common\core.h
# End Source File
# Begin Source File

SOURCE=..\src\common\db.c
# End Source File
# Begin Source File

SOURCE=..\src\common\db.h
# End Source File
# Begin Source File

SOURCE=..\src\common\ers.c
# End Source File
# Begin Source File

SOURCE=..\src\common\ers.h
# End Source File
# Begin Source File

SOURCE=..\src\common\lock.c
# End Source File
# Begin Source File

SOURCE=..\src\common\lock.h
# End Source File
# Begin Source File

SOURCE=..\src\common\malloc.c
# End Source File
# Begin Source File

SOURCE=..\src\common\malloc.h
# End Source File
# Begin Source File

SOURCE=..\src\common\mapindex.c
# End Source File
# Begin Source File

SOURCE=..\src\common\mapindex.h
# End Source File
# Begin Source File

SOURCE=..\src\common\mmo.h
# End Source File
# Begin Source File

SOURCE=..\src\common\showmsg.c
# End Source File
# Begin Source File

SOURCE=..\src\common\showmsg.h
# End Source File
# Begin Source File

SOURCE=..\src\common\sql.c
# End Source File
# Begin Source File

SOURCE=..\src\common\sql.h
# End Source File
# Begin Source File

SOURCE=..\src\common\strlib.c
# End Source File
# Begin Source File

SOURCE=..\src\common\strlib.h
# End Source File
# Begin Source File

SOURCE=..\src\common\timer.c
# End Source File
# Begin Source File

SOURCE=..\src\common\timer.h
# End Source File
# Begin Source File

SOURCE=..\src\common\txt.c
# End Source File
# Begin Source File

SOURCE=..\src\common\txt.h
# End Source File
# Begin Source File

SOURCE=..\src\common\utils.c
# End Source File
# Begin Source File

SOURCE=..\src\common\utils.h
# End Source File
# Begin Source File

SOURCE=..\src\common\version.h
# End Source File
# End Group
# Begin Group "char"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\char\accregdb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\accregdb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\accregdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\auctiondb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\auctiondb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\auctiondb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\castledb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\castledb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\castledb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\chardb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\chardb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\chardb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\charregdb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\charregdb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\charregdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\charserverdb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\charserverdb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\charserverdb_sql.h
# End Source File
# Begin Source File

SOURCE=..\src\char\charserverdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\charserverdb_txt.h
# End Source File
# Begin Source File

SOURCE=..\src\char\csdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\csdb_txt.h
# End Source File
# Begin Source File

SOURCE=..\src\char\csdbiterator.h
# End Source File
# Begin Source File

SOURCE=..\src\char\csdbiterator_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\csdbiterator_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\frienddb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\frienddb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\frienddb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\guilddb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\guilddb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\guilddb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\homundb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\homundb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\homundb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\hotkeydb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\hotkeydb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\hotkeydb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\maildb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\maildb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\maildb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\memodb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\memodb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\memodb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\mercdb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\mercdb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\mercdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\partydb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\partydb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\partydb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\petdb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\petdb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\petdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\questdb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\questdb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\questdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\rankdb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\rankdb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\rankdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\skilldb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\skilldb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\skilldb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\statusdb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\statusdb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\statusdb_txt.c
# End Source File
# Begin Source File

SOURCE=..\src\char\storagedb.h
# End Source File
# Begin Source File

SOURCE=..\src\char\storagedb_sql.c
# End Source File
# Begin Source File

SOURCE=..\src\char\storagedb_txt.c
# End Source File
# End Group
# Begin Group "converter"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\src\txt-converter\char-converter.c"
# End Source File
# End Group
# End Target
# End Project
