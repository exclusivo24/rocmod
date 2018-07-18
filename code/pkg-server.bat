@ECHO OFF


echo Copying QVMs

echo rocmod
cd ..\packages\ROCmod-xxxx-server\rocmod\vm
del *.qvm
copy ..\..\..\..\base\vm\sof2mp_game.qvm .
copy ..\..\..\..\base\vm\gt_ctf.qvm .
copy ..\..\..\..\base\vm\gt_dm.qvm .
copy ..\..\..\..\base\vm\gt_tdm.qvm .
copy ..\..\..\..\base\vm\gt_cctf.qvm .
copy ..\..\..\..\base\vm\gt_dem.qvm .
copy ..\..\..\..\base\vm\gt_elim.qvm .
copy ..\..\..\..\base\vm\gt_inf.qvm .
copy ..\..\..\..\base\vm\gt_stq.qvm .
copy ..\..\..\..\base\vm\gt_tstq.qvm .
cd ..
copy ..\..\..\*.txt .
copy ..\..\..\*.cfg .
cd docs
copy ..\..\..\..\docs\*.txt .
cd ..\..\..\..\code

echo rocmod-bots
cd ..\packages\ROCmod-xxxx-server-bots\rocmod\vm
del *.qvm
copy ..\..\..\..\base\vm-bot\sof2mp_game.qvm .
copy ..\..\..\..\base\vm\gt_ctf.qvm .
copy ..\..\..\..\base\vm\gt_dm.qvm .
copy ..\..\..\..\base\vm\gt_tdm.qvm .
copy ..\..\..\..\base\vm\gt_cctf.qvm .
copy ..\..\..\..\base\vm\gt_dem.qvm .
copy ..\..\..\..\base\vm\gt_elim.qvm .
copy ..\..\..\..\base\vm\gt_inf.qvm .
copy ..\..\..\..\base\vm\gt_stq.qvm .
copy ..\..\..\..\base\vm\gt_tstq.qvm .
cd ..
copy ..\..\..\*.txt .
copy ..\..\..\*.cfg .
cd docs
copy ..\..\..\..\docs\*.txt .
cd ..\..\..\..\code

echo rocmod-wp
cd ..\packages\ROCmod-WP-xxxx-server\rocmod-wp\vm
del *.qvm
copy ..\..\..\..\base\vm-wp\sof2mp_game.qvm .
copy ..\..\..\..\base\vm\gt_ctf.qvm .
copy ..\..\..\..\base\vm\gt_dm.qvm .
copy ..\..\..\..\base\vm\gt_tdm.qvm .
copy ..\..\..\..\base\vm\gt_cctf.qvm .
copy ..\..\..\..\base\vm\gt_dem.qvm .
copy ..\..\..\..\base\vm\gt_elim.qvm .
copy ..\..\..\..\base\vm\gt_inf.qvm .
copy ..\..\..\..\base\vm\gt_stq.qvm .
copy ..\..\..\..\base\vm\gt_tstq.qvm .
cd ..
copy ..\..\..\*.txt .
copy ..\..\..\*.cfg .
cd docs
copy ..\..\..\..\docs\*.txt .
cd ..\..\..\..\code

echo rocmod-wp-bots
cd ..\packages\ROCmod-WP-xxxx-server-bots\rocmod-wp\vm
del *.qvm
copy ..\..\..\..\base\vm-wp-bot\sof2mp_game.qvm .
copy ..\..\..\..\base\vm\gt_ctf.qvm .
copy ..\..\..\..\base\vm\gt_dm.qvm .
copy ..\..\..\..\base\vm\gt_tdm.qvm .
copy ..\..\..\..\base\vm\gt_cctf.qvm .
copy ..\..\..\..\base\vm\gt_dem.qvm .
copy ..\..\..\..\base\vm\gt_elim.qvm .
copy ..\..\..\..\base\vm\gt_inf.qvm .
copy ..\..\..\..\base\vm\gt_stq.qvm .
copy ..\..\..\..\base\vm\gt_tstq.qvm .
cd ..
copy ..\..\..\*.txt .
copy ..\..\..\*.cfg .
cd docs
copy ..\..\..\..\docs\*.txt .
cd ..\..\..\..\code


echo Copying PK3s

echo rocmod
cd ..\packages\ROCmod-xxxx-server\rocmod
del *.pk3
copy ..\..\ROCmod-xxxx-client\*.pk3 .
cd ..\..\..\code

echo rocmod-bots
cd ..\packages\ROCmod-xxxx-server-bots\rocmod
del *.pk3
copy ..\..\ROCmod-xxxx-client\*.pk3 .
cd ..\..\..\code

echo rocmod-wp
cd ..\packages\ROCmod-WP-xxxx-server\rocmod-wp
del *.pk3
copy ..\..\ROCmod-WP-xxxx-client\*.pk3 .
cd ..\..\..\code

echo rocmod-wp-bots
cd ..\packages\ROCmod-WP-xxxx-server-bots\rocmod-wp
del *.pk3
copy ..\..\ROCmod-WP-xxxx-client\*.pk3 .
cd ..\..\..\code

