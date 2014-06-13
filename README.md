frok-server
===========

face recognition odnoklassniki - server side

How to install in Windows:
1) Donwload and install to C:\ opencv from http://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.8/opencv-2.4.8.exe/download

2) Download dll-installer https://www.dropbox.com/s/ue6ub71k53tvxwi/opencv_inst.rar
3) Launch dll-installer as admin (MS Visual studiio 2008+ must be installed before launching) with following parameters:
<path to dll-installer> <32/64 version of Windows> <MS Visual studio version>
Example: D:\dll-installer D:\ 64 13

4) Pull this repository.
5) Check following properties of project:
C/C++: Additional Include Directories = C:\opencv\build\include\opencv; C:\opencv\build\include;
Linker:
Additional Library Directories = C:\opencv\build\x64\vc12\lib;C:\opencv\build\include\opencv;C:\opencv\build\include;
Additional Dependances = opencv_core248d.lib opencv_imgproc248d.lib opencv_highgui248d.lib opencv_video248d.lib opencv_objdetect248d.lib opencv_calib3d248d.lib opencv_features2d248d.lib opencv_nonfree248d.lib opencv_flann248d.lib opencv_legacy248d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
Import Library = opencv_core248d.lib opencv_imgproc248d.lib opencv_highgui248d.lib opencv_video248d.lib opencv_objdetect248d.lib opencv_calib3d248d.lib opencv_features2d248d.lib opencv_nonfree248d.lib opencv_flann248d.lib opencv_legacy248d.lib $(TargetDir)$(TargetName).lib,user32.lib,kernel32.lib,user32.lib,gdi32.lib,winspool.lib,comdlg32.lib,advapi32.lib,shell32.lib,ole32.lib,oleaut32.lib,uuid.lib,odbc32.lib,odbccp32.lib


P.S. note that Additional Library Directories should consider path to libraries with current configuration (C:\opencv\build\x86... for 64 bin, C:\opencv\build\x86 for 32)

6) Download and install Doxygen http://www.stack.nl/~dimitri/doxygen/download.html
7) Run as admin in cmd
 "REG ADD HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System /v EnableLUA /t REG_DWORD /d 0 /f"
 
8)  Указать пути к папкам где будут сохраняться фотки и модели юзеров в файле FaceDetectionLib/activities.h
#define ID_PATH				"Z:\\frok\\"
#define TARGET_PATH			"Z:\\frok\\1\\"

9) Build project and see FaceDetection_manual for additional information
