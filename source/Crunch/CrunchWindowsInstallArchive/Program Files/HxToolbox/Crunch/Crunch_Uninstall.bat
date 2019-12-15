@ECHO off

CD /D C:\Documents and Settings\All Users\Desktop
DEL /F Crunch*

CD /D C:\Documents and Settings\All Users\Start Menu\Programs\HxToolbox
RD /S /Q Crunch

CD /D C:\Program Files\HxToolbox
RD /S /Q Crunch

ECHO.
echo "CRUNCH has been removed from your system"
ECHO.

PAUSE