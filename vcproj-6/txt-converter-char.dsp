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
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "tmp\txt_converter_char\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Zi /O2 /I "..\src\common" /I "..\3rdparty\msinttypes\include" /FI"config.vc.h" /D "WIN32" /D "NDEBUG" /D "TXT_SQL_CONVERT" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\tools"
# PROP Intermediate_Dir "tmp\txt_converter_char\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /ZI /Od /I "..\src\common" /I "..\3rdparty\msinttypes\include" /D "WIN32" /D "_DEBUG" /D "TXT_SQL_CONVERT" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"txt-converter-char.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "txt_converter_char - Win32 Release"
# Name "txt_converter_char - Win32 Debug"
# Begin Group "char_sql"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\char_sql\char.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\char.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_guild.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_guild.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_mercenary.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_mercenary.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_party.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_party.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_pet.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_pet.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_storage.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\int_storage.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\inter.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char_sql\inter.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_sql"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_sql"

!ENDIF 

# End Source File
# End Group
# Begin Group "char_txt"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\char\char.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\char.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\int_guild.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\int_guild.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\int_party.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\int_party.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\int_pet.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\int_pet.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\int_storage.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\int_storage.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\inter.c

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\char\inter.h

!IF  "$(CFG)" == "txt_converter_char - Win32 Release"

# PROP Intermediate_Dir "tmp\txt_converter_char\Release\char_txt"

!ELSEIF  "$(CFG)" == "txt_converter_char - Win32 Debug"

# PROP Intermediate_Dir "tmp\txt_converter_char\Debug\char_txt"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE="..\src\txt-converter\char-converter.c"
# End Source File
# End Target
# End Project
