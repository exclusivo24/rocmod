@ECHO OFF

@set include=

@cd cgame

del /q vm
mkdir vm
cd vm
set cc=D:\prgrmmng\sof2sdk\bin\sof2lcc -DQ3_VM -D_SOF2_BOTS -DSMKWEAPONS -DMISSIONPACK -DCGAME -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

echo ---------------------------------

%cc% ../../game/bg_misc.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_weapons.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_player.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_pmove.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_slidemove.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_lib.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_gametype.c
@if errorlevel 1 goto quit
%cc% ../../game/q_math.c
@if errorlevel 1 goto quit
%cc% ../../game/q_shared.c
@if errorlevel 1 goto quit
%cc% ../cg_consolecmds.c
@if errorlevel 1 goto quit
%cc% ../cg_draw.c
@if errorlevel 1 goto quit
%cc% ../cg_drawtools.c
@if errorlevel 1 goto quit
%cc% ../cg_effects.c
@if errorlevel 1 goto quit
%cc% ../cg_ents.c
@if errorlevel 1 goto quit
%cc% ../cg_event.c
@if errorlevel 1 goto quit
%cc% ../cg_gametype.c
@if errorlevel 1 goto quit
%cc% ../cg_gore.c
@if errorlevel 1 goto quit
%cc% ../cg_info.c
@if errorlevel 1 goto quit
%cc% ../cg_light.c
@if errorlevel 1 goto quit
%cc% ../cg_localents.c
@if errorlevel 1 goto quit
%cc% ../cg_main.c
@if errorlevel 1 goto quit
%cc% ../cg_miscents.c
@if errorlevel 1 goto quit
%cc% ../cg_players.c
@if errorlevel 1 goto quit
%cc% ../cg_playerstate.c
@if errorlevel 1 goto quit
%cc% ../cg_predict.c
@if errorlevel 1 goto quit
%cc% ../cg_scoreboard.c
@if errorlevel 1 goto quit
%cc% ../cg_scoreboard2.c
@if errorlevel 1 goto quit
%cc% ../cg_servercmds.c
@if errorlevel 1 goto quit
%cc% ../cg_snapshot.c
@if errorlevel 1 goto quit
%cc% ../cg_view.c
@if errorlevel 1 goto quit
%cc% ../cg_weaponinit.c
@if errorlevel 1 goto quit
%cc% ../cg_weapons.c
@if errorlevel 1 goto quit
%cc% ../../ui/ui_shared.c
@if errorlevel 1 goto quit
%cc% ../cg_newDraw.c
@if errorlevel 1 goto quit
%cc% ../cg_stats.c
@if errorlevel 1 goto quit
%cc% ../cg_md5.c
@if errorlevel 1 goto quit

echo ---------------------------------

D:\prgrmmng\sof2sdk\bin\sof2asm -f ../cgame
@if errorlevel 1 goto quit

mkdir "..\..\..\base\vm-wp"
copy *.map "..\..\..\base\vm-wp"
copy *.qvm "..\..\..\base\vm-wp"
copy *.qvm "..\..\..\packages\ROCmod-WP-xxxx-client\vm"

:quit
@cd ..
@cd ..
