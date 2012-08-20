# Microsoft Developer Studio Project File - Name="ctiapi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=ctiapi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ctiapi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ctiapi.mak" CFG="ctiapi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ctiapi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ctiapi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ctiapi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\Bin\Release\DLL"
# PROP Intermediate_Dir "..\Obj\ctiapi\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ctiapi_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\hollylib\include" /I "..\hollylib\inc\ace" /D "WIN32" /D "_WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_VC" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 /nologo /dll /machine:I386 /libpath:"..\hollylib\lib" /libpath:"..\hollylib\inc\ace\lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copylibr
PostBuild_Cmds=copy ctiapidefs.h .\include\ctiapidefs.h	copy ctiapi.h .\include\ctiapi.h	copy ..\Bin\Release\DLL\ctiapi.lib .\lib\ctiapi.lib	copy ..\Bin\Release\DLL\ctiapi.lib ..\TelService\CMU\lib\ctiapi.lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ctiapi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Bin\Debug\DLL"
# PROP Intermediate_Dir "..\Obj\ctiapi\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ctiapi_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\hollylib\include" /I "..\hollylib\inc\ace" /D "_CONSOLE" /D "_WIN32" /D "_DEBUG" /D "_MBCS" /D "WIN32" /D "_VC" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /dll /debug /machine:I386 /out:"..\Bin\Debug\HollyContact\ctiapi.dll" /pdbtype:sept /libpath:"..\hollylib\lib" /libpath:"..\hollylib\inc\ace\lib"
# SUBTRACT LINK32 /nodefaultlib
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=copylib
PostBuild_Cmds=copy ctiapidefs.h .\include\ctiapidefs.h	copy ctiapi.h .\include\ctiapi.h	copy .\bin\Debug\DLL\ctiapi.lib .\lib\ctiapi.lib	copy .\bin\Debug\DLL\ctiapi.lib .\ctiapiTest
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ctiapi - Win32 Release"
# Name "ctiapi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cmuapi.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\ctiapi.def
# End Source File
# Begin Source File

SOURCE=.\ctiapi.rc
# End Source File
# Begin Source File

SOURCE=.\TConfig.cpp

!IF  "$(CFG)" == "ctiapi - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "ctiapi - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\TUser.cpp

!IF  "$(CFG)" == "ctiapi - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "ctiapi - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ctiapi.h
# End Source File
# Begin Source File

SOURCE=.\ctiapidefs.h
# End Source File
# Begin Source File

SOURCE=..\HollyLib\include\CMUInterface.h
# End Source File
# Begin Source File

SOURCE=.\TConfig.h
# End Source File
# Begin Source File

SOURCE=.\TUser.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=".\ctiapiÐÞ¸ÄËµÃ÷.txt"
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
