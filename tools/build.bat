@ECHO OFF
REM This file is a part of cdev project
REM https://github.com/adm244/cdev

SETLOCAL
REM [customize those variables]
SET libs=kernel32.lib user32.lib Psapi.lib
SET files=%source%\main.cpp
SET deffile=%source%\exports.def
SET hookname=dinput8

SET debug=/Od /Zi /Fe%hookname% /nologo /DDEBUG
SET release=/O2 /WX /Fe%hookname% /nologo
SET args=%debug% %files% /LD /link /DEF:%deffile% %libs%

SET compiler=CL
REM ###########################

SET edit=edit
SET setprjname=setname

IF [%1]==[%setprjname%] GOTO SetProjectName
IF [%1]==[%edit%] GOTO EditBuildFile
IF [%1]==[] GOTO Build
GOTO Error

:Build
ECHO: Build started...

IF NOT EXIST "%bin%" MKDIR "%bin%"
PUSHD "%bin%"
"%compiler%" %args%
POPD

ECHO: Build finished.
GOTO:EOF

:SetProjectName
IF [%2]==[] ECHO: ERROR: Name for a project was NOT specified! && GOTO:EOF

ECHO: Changing project name to %2...
ENDLOCAL
SET project=%2
ECHO: Done!
GOTO:EOF

:EditBuildFile
"%editor%" "%tools%\%~n0.bat"
GOTO:EOF

:Error
ECHO: ERROR: wrong arguments passed!
GOTO:EOF