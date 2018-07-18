@ECHO OFF

if not "%nocount%" == "1" call buildcounter.bat

@set include=

@cd game

del /q vm
mkdir vm
cd vm
set cc=D:\prgrmmng\sof2sdk\bin\sof2lcc -A -DQ3_VM -DROCMOD_BUILD=%buildcount% -D_SOF2_BOTS -DSMKWEAPONS -DMISSIONPACK -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

echo ---------------------------------

%cc%  ../g_main.c
@if errorlevel 1 goto quit

%cc%  ../g_syscalls.c
@if errorlevel 1 goto quit

%cc%  ../bg_misc.c
@if errorlevel 1 goto quit
%cc%  ../bg_lib.c
@if errorlevel 1 goto quit
%cc%  ../bg_pmove.c
@if errorlevel 1 goto quit
%cc%  ../bg_slidemove.c
@if errorlevel 1 goto quit
%cc%  ../bg_player.c
@if errorlevel 1 goto quit
%cc%  ../bg_weapons.c
@if errorlevel 1 goto quit
%cc%  ../bg_gametype.c
@if errorlevel 1 goto quit
%cc%  ../q_math.c
@if errorlevel 1 goto quit
%cc%  ../q_shared.c
@if errorlevel 1 goto quit

%cc%  ../ai_main.c
@if errorlevel 1 goto quit
%cc%  ../ai_util.c
@if errorlevel 1 goto quit
%cc%  ../ai_wpnav.c
@if errorlevel 1 goto quit

%cc%  ../g_active.c
@if errorlevel 1 goto quit
%cc%  ../g_antilag.c
@if errorlevel 1 goto quit
%cc%  ../g_bot.c
@if errorlevel 1 goto quit
%cc%  ../g_client.c
@if errorlevel 1 goto quit
%cc%  ../g_cmds.c
@if errorlevel 1 goto quit
%cc%  ../g_combat.c
@if errorlevel 1 goto quit
%cc%  ../g_gametype.c
@if errorlevel 1 goto quit
%cc%  ../g_items.c
@if errorlevel 1 goto quit
%cc%  ../g_misc.c
@if errorlevel 1 goto quit
%cc%  ../g_missile.c
@if errorlevel 1 goto quit
%cc%  ../g_mover.c
@if errorlevel 1 goto quit
%cc%  ../g_session.c
@if errorlevel 1 goto quit
%cc%  ../g_spawn.c
@if errorlevel 1 goto quit
%cc%  ../g_svcmds.c
@if errorlevel 1 goto quit
%cc%  ../g_target.c
@if errorlevel 1 goto quit
%cc%  ../g_team.c
@if errorlevel 1 goto quit
%cc%  ../g_trigger.c
@if errorlevel 1 goto quit
%cc%  ../g_utils.c
@if errorlevel 1 goto quit
%cc%  ../g_weapon.c
@if errorlevel 1 goto quit

%cc%  ../g_admin.c
@if errorlevel 1 goto quit
%cc%  ../g_admincmds.c
@if errorlevel 1 goto quit

echo ---------------------------------

D:\prgrmmng\sof2sdk\bin\sof2asm -f ../game
@if errorlevel 1 goto quit

mkdir "..\..\..\base\vm-wp-bot"
copy *.map "..\..\..\base\vm-wp-bot"
copy *.qvm "..\..\..\base\vm-wp-bot"
copy *.qvm "..\..\..\packages\ROCmod-WP-xxxx-server-bots\rocmod-wp\vm"

:quit
@cd ..
@cd ..
