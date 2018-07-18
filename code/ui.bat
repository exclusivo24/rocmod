@ECHO OFF

@set include=

cd ui

del /q vm
mkdir vm
cd vm

set cc=D:\PRGRMMNG\SoF2SDK\bin\sof2lcc -DQ3_VM -S -Wf-target=bytecode -Wf-g -I..\..\cgame -I..\..\game -I..\..\ui %1

echo ---------------------------------

%cc% ../ui_main.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_misc.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_player.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_weapons.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_lib.c
@if errorlevel 1 goto quit
%cc% ../../game/bg_gametype.c
@if errorlevel 1 goto quit
%cc% ../../game/q_math.c
@if errorlevel 1 goto quit
%cc% ../../game/q_shared.c
@if errorlevel 1 goto quit
%cc% ../ui_atoms.c
@if errorlevel 1 goto quit
%cc% ../ui_players.c
@if errorlevel 1 goto quit
%cc% ../ui_shared.c
@if errorlevel 1 goto quit
%cc% ../ui_gameinfo.c
@if errorlevel 1 goto quit

echo ---------------------------------

D:\PRGRMMNG\SoF2SDK\bin\sof2asm -f ../ui
@if errorlevel 1 goto quit

mkdir "..\..\..\base\vm"
copy *.map "..\..\..\base\vm"
copy *.qvm "..\..\..\base\vm"
copy *.qvm "..\..\..\packages\ROCmod-xxxx-client\vm"

:quit
cd ..
cd ..
