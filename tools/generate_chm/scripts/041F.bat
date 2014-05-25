cd ..

cd ..\lang
for /r %%i in (trk*.hhp) do if exist %%i (hhc.exe %%i)

reg import C:\scripts\backup.reg
del C:\scripts\backup.reg 

cd C:\lang
for /r %%i in (*.chm) do (cd %%~pi & cd .. & md chm & cd chm & copy %%i & del %%i)

set my=

C:\scripts\last.bat

