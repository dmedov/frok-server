cd ..

cd ..\lang

cd chs && (hhc.exe chs.hhp & cd ..)

reg add HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce /v Name /t reg_sz /d c:\scripts\1255.bat /f

reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v ACP /t reg_sz /d 1255 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v OEMCP /t reg_sz /d 862 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\CodePage /v MACCP /t reg_sz /d 10005 /f
reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Nls\Language /v Default /t reg_sz /d 040D /f

cd heb && (shutdown -r)
cd heb ||(C:\scripts\1255.bat)