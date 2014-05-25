cd ..

cd ..\lang

for /r %%i in (rus*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce /v Name /t reg_sz /d c:\scripts\041E.bat /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v ACP /t reg_sz /d 874 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v OEMCP /t reg_sz /d 874 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v MACCP /t reg_sz /d 10021 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 041E /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 0000041E /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v ACP /t reg_sz /d 874 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v OEMCP /t reg_sz /d 874 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v MACCP /t reg_sz /d 10021 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 041E /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 0000041E /f

set my=0
for /r %%i in (tha*.hhp) do if exist %%i (set my=1)
if "%my%" EQU "1" (shutdown -r) else (C:\scripts\041E.bat)
