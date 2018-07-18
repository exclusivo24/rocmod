@ECHO OFF

cd gametype\gt_dem

@set include=
del /q vm
mkdir vm
cd vm
set cc=D:\PRGRMMNG\SoF2SDK\bin\sof2lcc -A -DQ3_VM -DMISSIONPACK -S -Wf-target=bytecode -Wf-g -I..\..\..\gametype -I..\..\gt_dem -I..\..\..\game %1

echo ---------------------------------

%cc%  ../gt_main.c
@if errorlevel 1 goto quit

%cc%  ../../gt_syscalls.c
@if errorlevel 1 goto quit

%cc%  ../../../game/bg_lib.c
@if errorlevel 1 goto quit

%cc%  ../../../game/q_shared.c
@if errorlevel 1 goto quit

%cc%  ../../../game/q_math.c
@if errorlevel 1 goto quit

echo ---------------------------------

D:\PRGRMMNG\SoF2SDK\bin\sof2asm -f ../gt_dem
@if errorlevel 1 goto quit

mkdir "..\..\..\..\base\vm"
copy *.map "..\..\..\..\base\vm"
copy *.qvm "..\..\..\..\base\vm"
copy *.qvm "..\..\..\..\packages\ROCmod-xxxx-server\rocmod\vm"
copy *.qvm "..\..\..\..\packages\ROCmod-xxxx-server-bots\rocmod\vm"
copy *.qvm "..\..\..\..\packages\ROCmod-WP-xxxx-server\rocmod-wp\vm"
copy *.qvm "..\..\..\..\packages\ROCmod-WP-xxxx-server-bots\rocmod-wp\vm"

:quit
cd ..
cd ..
cd ..
