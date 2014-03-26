frok-server
===========

face recognition odnoklassniki - server side

How to install in Windows:
1) Donwload and install to C:\ opencv from http://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.8/opencv-2.4.8.exe/download
2) Download dll-installer https://vk.com/away.php?to=https%3A%2F%2Fwww.dropbox.com%2Fs%2F69t1nmsk54v7ga8%2Fopencv_inst.rar
3) Launch dll-installer (MS Visual studiio 2008+ must be installed before launching) with following parameters:
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

6) Build project and run with parameters <path to image>
