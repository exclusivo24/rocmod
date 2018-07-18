@ECHO OFF


echo Copying QVMs

echo client
mkdir ..\packages\ROCmod-xxxx-client\vm
cd ..\packages\ROCmod-xxxx-client\vm
del *.qvm
copy ..\..\..\base\vm\sof2mp_cgame.qvm .
copy ..\..\..\base\vm\sof2mp_ui.qvm .
cd ..\..\..\code

echo client-wp
mkdir ..\packages\ROCmod-WP-xxxx-client\vm
cd ..\packages\ROCmod-WP-xxxx-client\vm
del *.qvm
copy ..\..\..\base\vm-wp\sof2mp_cgame.qvm .
copy ..\..\..\base\vm-wp\sof2mp_ui.qvm .
cd ..\..\..\code

echo Copying UI

echo client
cd ..\packages\ROCmod-xxxx-client
xcopy /E ..\..\client .
cd ..\..\code

echo client-wp
cd ..\packages\ROCmod-WP-xxxx-client
xcopy /E ..\..\client .
cd ..\..\code

