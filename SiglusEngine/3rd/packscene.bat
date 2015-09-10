@echo off
mkdir Scene\SceneNew  >nul 2>&1
for %%s in (Scene\\*.ss) do ssinsert.exe "%%s" "Scene\\%%~ns-str.txt" "Scene\\SceneNew\\%%~nxs"
copy /Y Scene\vars.dat Scene\SceneNew\vars.dat >nul 2>&1
@echo Finished inserting scripts
createpck.exe "Scene\\SceneNew" "Scene.pck"
@echo on
@echo Finished packing new Scene.pck
pause