@ECHO OFF

call buildcounter.bat
set nocount=1

call game.bat
pause
call game-bots.bat
pause
call game-wp.bat
pause
call game-wp-bots.bat
pause

call gametypes.bat
pause

call cgame.bat
pause
call cgame-wp.bat
pause

call ui.bat
pause
call ui-wp.bat

set nocount=
