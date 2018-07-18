@ECHO OFF

call buildcounter.bat
set nocount=1

call game.bat

call game-bots.bat

call game-wp.bat

call game-wp-bots.bat

call gametypes.bat

set nocount=
