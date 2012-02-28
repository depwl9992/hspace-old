# Microsoft Developer Studio Project File - Name="hspace" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=hspace - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "hspace.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "hspace.mak" CFG="hspace - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "hspace - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "hspace - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "hspace"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "hspace - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\.." /I "..\..\hdrs" /I "hsnetwork" /I "hscommon" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "PENNMUSH" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"hspace.lib"

!ELSEIF  "$(CFG)" == "hspace - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "..\.." /I "..\..\hdrs" /I "hsnetwork" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "PENNMUSH" /FR /YX"pch.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"hspaced.lib"

!ENDIF 

# Begin Target

# Name "hspace - Win32 Release"
# Name "hspace - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\hscelestial.cpp
# End Source File
# Begin Source File

SOURCE=.\hsclass.cpp
# End Source File
# Begin Source File

SOURCE=.\hscloaking.cpp
# End Source File
# Begin Source File

SOURCE=.\hscmds.cpp
# End Source File
# Begin Source File

SOURCE=.\hscomm.cpp
# End Source File
# Begin Source File

SOURCE=.\hscommunications.cpp
# End Source File
# Begin Source File

SOURCE=.\hscomputer.cpp
# End Source File
# Begin Source File

SOURCE=.\hsconf.cpp
# End Source File
# Begin Source File

SOURCE=.\hsconsole.cpp
# End Source File
# Begin Source File

SOURCE=.\hsconsolecmds.cpp
# End Source File
# Begin Source File

SOURCE=.\hsdamagecontrol.cpp
# End Source File
# Begin Source File

SOURCE=.\hsdb.cpp
# End Source File
# Begin Source File

SOURCE=.\hseng.cpp
# End Source File
# Begin Source File

SOURCE=.\hsengcmds.cpp
# End Source File
# Begin Source File

SOURCE=.\hsengines.cpp
# End Source File
# Begin Source File

SOURCE=.\HSFileDatabase.cpp
# End Source File
# Begin Source File

SOURCE=.\hsfuel.cpp
# End Source File
# Begin Source File

SOURCE=.\hsfuncs.cpp
# End Source File
# Begin Source File

SOURCE=.\hsinterface.cpp
# End Source File
# Begin Source File

SOURCE=.\hsjammer.cpp
# End Source File
# Begin Source File

SOURCE=.\hslifesupport.cpp
# End Source File
# Begin Source File

SOURCE=.\hsmain.cpp
# End Source File
# Begin Source File

SOURCE=.\hsmissile.cpp
# End Source File
# Begin Source File

SOURCE=.\hsnavcmds.cpp
# End Source File
# Begin Source File

SOURCE=.\hsobjects.cpp
# End Source File
# Begin Source File

SOURCE=.\hspace.cpp
# End Source File
# Begin Source File

SOURCE=.\hsplanet.cpp
# End Source File
# Begin Source File

SOURCE=.\hsreactor.cpp
# End Source File
# Begin Source File

SOURCE=.\hssensors.cpp
# End Source File
# Begin Source File

SOURCE=.\hsserver.cpp
# End Source File
# Begin Source File

SOURCE=.\hsshields.cpp
# End Source File
# Begin Source File

SOURCE=.\hsship.cpp
# End Source File
# Begin Source File

SOURCE=.\hsshipeng.cpp
# End Source File
# Begin Source File

SOURCE=.\hsshipnav.cpp
# End Source File
# Begin Source File

SOURCE=.\hstachyon.cpp
# End Source File
# Begin Source File

SOURCE=.\hsterritory.cpp
# End Source File
# Begin Source File

SOURCE=.\hsthrusters.cpp
# End Source File
# Begin Source File

SOURCE=.\hstractor.cpp
# End Source File
# Begin Source File

SOURCE=.\hstrig.cpp
# End Source File
# Begin Source File

SOURCE=.\hsuniverse.cpp
# End Source File
# Begin Source File

SOURCE=.\hsuniversedb.cpp
# End Source File
# Begin Source File

SOURCE=.\hsutils.cpp
# End Source File
# Begin Source File

SOURCE=.\hsweapon.cpp
# End Source File
# Begin Source File

SOURCE=.\pch.cpp

!IF  "$(CFG)" == "hspace - Win32 Release"

!ELSEIF  "$(CFG)" == "hspace - Win32 Debug"

# ADD CPP /Yc"pch.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\hsansi.h
# End Source File
# Begin Source File

SOURCE=.\hscelestial.h
# End Source File
# Begin Source File

SOURCE=.\hsclass.h
# End Source File
# Begin Source File

SOURCE=.\hscloaking.h
# End Source File
# Begin Source File

SOURCE=.\hscmds.h
# End Source File
# Begin Source File

SOURCE=.\hscomm.h
# End Source File
# Begin Source File

SOURCE=.\hscommunications.h
# End Source File
# Begin Source File

SOURCE=.\hscomputer.h
# End Source File
# Begin Source File

SOURCE=.\hsconf.h
# End Source File
# Begin Source File

SOURCE=.\hsconsole.h
# End Source File
# Begin Source File

SOURCE=.\hscopyright.h
# End Source File
# Begin Source File

SOURCE=.\hsdamagecontrol.h
# End Source File
# Begin Source File

SOURCE=.\hsdb.h
# End Source File
# Begin Source File

SOURCE=.\hseng.h
# End Source File
# Begin Source File

SOURCE=.\hsengines.h
# End Source File
# Begin Source File

SOURCE=.\HSEngTypes.h
# End Source File
# Begin Source File

SOURCE=.\HSFileDatabase.h
# End Source File
# Begin Source File

SOURCE=.\hsflags.h
# End Source File
# Begin Source File

SOURCE=.\hsfuel.h
# End Source File
# Begin Source File

SOURCE=.\hsfuncs.h
# End Source File
# Begin Source File

SOURCE=.\hsinterface.h
# End Source File
# Begin Source File

SOURCE=.\hsjammer.h
# End Source File
# Begin Source File

SOURCE=.\hslifesupport.h
# End Source File
# Begin Source File

SOURCE=.\hsmissile.h
# End Source File
# Begin Source File

SOURCE=.\hsobject.h
# End Source File
# Begin Source File

SOURCE=.\hsobjects.h
# End Source File
# Begin Source File

SOURCE=.\hspace.h
# End Source File
# Begin Source File

SOURCE=.\hsreactor.h
# End Source File
# Begin Source File

SOURCE=.\hssensors.h
# End Source File
# Begin Source File

SOURCE=.\hsserver.h
# End Source File
# Begin Source File

SOURCE=.\hsshields.h
# End Source File
# Begin Source File

SOURCE=.\HSSingleton.h
# End Source File
# Begin Source File

SOURCE=.\hstachyon.h
# End Source File
# Begin Source File

SOURCE=.\hsterritory.h
# End Source File
# Begin Source File

SOURCE=.\hsthrusters.h
# End Source File
# Begin Source File

SOURCE=.\hstractor.h
# End Source File
# Begin Source File

SOURCE=.\hstypes.h
# End Source File
# Begin Source File

SOURCE=.\hsuniverse.h
# End Source File
# Begin Source File

SOURCE=.\hsuniversedb.h
# End Source File
# Begin Source File

SOURCE=.\hsutils.h
# End Source File
# Begin Source File

SOURCE=.\HSVariant.h
# End Source File
# Begin Source File

SOURCE=.\hsversion.h
# End Source File
# Begin Source File

SOURCE=.\hsweapon.h
# End Source File
# Begin Source File

SOURCE=.\pch.h
# End Source File
# End Group
# Begin Group "Help"

# PROP Default_Filter "hlp"
# Begin Source File

SOURCE=.\help_files\hsadmcmds.hlp
# End Source File
# Begin Source File

SOURCE=.\help_files\hsadmin.hlp
# End Source File
# Begin Source File

SOURCE=.\help_files\hscmds.hlp
# End Source File
# Begin Source File

SOURCE=.\help_files\hsfuncs.hlp
# End Source File
# Begin Source File

SOURCE=.\help_files\hspace.hlp
# End Source File
# End Group
# Begin Group "Handlers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\HSHandlerClass.cpp
# End Source File
# Begin Source File

SOURCE=.\HSHandlerClass.h
# End Source File
# Begin Source File

SOURCE=.\HSHandlerLogin.cpp
# End Source File
# Begin Source File

SOURCE=.\HSHandlerLogin.h
# End Source File
# Begin Source File

SOURCE=.\HSHandlerMisc.cpp
# End Source File
# Begin Source File

SOURCE=.\HSHandlerMisc.h
# End Source File
# Begin Source File

SOURCE=.\HSHandlerObject.cpp
# End Source File
# Begin Source File

SOURCE=.\HSHandlerObject.h
# End Source File
# Begin Source File

SOURCE=.\HSHandlerUniverse.cpp
# End Source File
# Begin Source File

SOURCE=.\HSHandlerUniverse.h
# End Source File
# Begin Source File

SOURCE=.\HSHandlerWeapon.cpp
# End Source File
# Begin Source File

SOURCE=.\HSHandlerWeapon.h
# End Source File
# Begin Source File

SOURCE=.\HSPacketHandler.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ADMIN_TOOLKIT.txt
# End Source File
# Begin Source File

SOURCE=.\INSTALLATION_GUIDE.txt
# End Source File
# End Target
# End Project
