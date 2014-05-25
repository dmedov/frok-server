cd ..
cd ..\lang

for /r %%i in (dan*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0413 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000413 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0413 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000413 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 0413 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 00000413 /f

for /r %%i in (nld*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 040B /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 0000040B /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 040B /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 0000040B /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 040B /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 0000040B /f
for /r %%i in (fin*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 040C /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 0000040C /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 040C /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 0000040C /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 040C /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 0000040C /f
for /r %%i in (fra*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0407 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000407 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0407 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000407 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 0407 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 00000407 /f
for /r %%i in (deu*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0410 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000410 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0410 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000410 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 0410 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 00000410 /f
for /r %%i in (ita*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0414 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000414 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0414 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000414 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 0414 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 00000414 /f
for /r %%i in (nor*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0416 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000416 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0416 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000416 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 0416 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 00000416 /f
for /r %%i in (ptb*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0816 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000816 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0816 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000816 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 0816 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 00000816 /f
for /r %%i in (ptg*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0C0A /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000C0A /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0C0A /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000C0A /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 0C0A /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 00000C0A /f
for /r %%i in (esn*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 041D /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 0000041D /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 041D /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 0000041D /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 041D /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 0000041D /f
for /r %%i in (sve*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0809 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000809 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0809 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000809 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Language /v Default /t reg_sz /d 0809 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet002\Control\Nls\Locale /v (Default) /t reg_sz /d 00000809 /f
for /r %%i in (eng*.hhp) do if exist %%i (hhc.exe %%i)

reg add HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce /v Name /t reg_sz /d c:\scripts\1250.bat /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v ACP /t reg_sz /d 1250 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v OEMCP /t reg_sz /d 852 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v MACCP /t reg_sz /d 10029 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 0405 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Locale /v (Default) /t reg_sz /d 00000405 /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v ACP /t reg_sz /d 1250 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v OEMCP /t reg_sz /d 852 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\CodePage /v MACCP /t reg_sz /d 10029 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Language /v Default /t reg_sz /d 0405 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\ControlSet001\Control\Nls\Locale /v (Default) /t reg_sz /d 00000405 /f


set my=0
for /r %%i in (csy*.hhp hun*.hhp plk*.hhp hrv*.hhp ron*.hhp) do if exist %%i (set my=1)
if "%my%" EQU "1" (shutdown -r) else (C:\scripts\1250.bat)
