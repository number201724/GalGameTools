# Microsoft Developer Studio Project File - Name="CatSystem2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=CatSystem2 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "CatSystem2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "CatSystem2.mak" CFG="CatSystem2 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "CatSystem2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "CatSystem2 - Win32 Grisaia" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /Gr /MD /W4 /GR- /O1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GS- /GL /MP /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"CatSystem2.exe" /fixed /ltcg
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "CatSystem2___Win32_Grisaia"
# PROP BASE Intermediate_Dir "CatSystem2___Win32_Grisaia"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /Gr /MD /W4 /GR- /O1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GS- /GL /MP /c
# ADD CPP /nologo /Gr /MD /W4 /GR- /O2 /Ob1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "CS2_GRISAIA" /FD /GS- /GL /MP /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"CatSystem2.exe" /fixed /ltcg
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"Grisaia_upk.exe" /fixed /ltcg
# SUBTRACT LINK32 /pdb:none /debug

!ENDIF 

# Begin Target

# Name "CatSystem2 - Win32 Release"
# Name "CatSystem2 - Win32 Grisaia"
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\adler32.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\compress.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\crc32.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\deflate.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\inffast.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\inflate.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\inftrees.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\trees.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\uncompr.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\..\Library\zlib\zutil.c

!IF  "$(CFG)" == "CatSystem2 - Win32 Release"

!ELSEIF  "$(CFG)" == "CatSystem2 - Win32 Grisaia"

# ADD CPP /W3

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\blowfish.cpp
# End Source File
# Begin Source File

SOURCE=.\blowfish.h
# End Source File
# Begin Source File

SOURCE=.\CatSystem2.cpp
# End Source File
# Begin Source File

SOURCE=.\CatSystem2.h
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\MTwister.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\MyLib\src\my_api.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\MyLib\src\my_crtadd.cpp
# End Source File
# End Target
# End Project
