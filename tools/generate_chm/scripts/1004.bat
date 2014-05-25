cd ..
cd ..\lang

for /r %%i in (cht*.hhp) do if exist %%i (hhc.exe %%i)

for /r %%i in (hkg*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0804 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000804 /f
for /r %%i in (chs*.hhp) do if exist %%i (hhc.exe %%i)


reg add HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce /v Name /t reg_sz /d c:\scripts\1255.bat /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v ACP /t reg_sz /d 1255 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v OEMCP /t reg_sz /d 862 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v MACCP /t reg_sz /d 10005 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 040D /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 0000040D /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v ACP /t reg_sz /d 1255 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v OEMCP /t reg_sz /d 862 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v MACCP /t reg_sz /d 10005 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 040D /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 0000040D /f

set my=0
for /r %%i in (heb*.hhp) do if exist %%i (set my=1)
if "%my%" EQU "1" (shutdown -r) else (C:\scripts\1255.bat)