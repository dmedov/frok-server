frok-server
===========

face recognition odnoklassniki - server side

How to install in Windows:
1) Donwload and install to C:\ opencv from http://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.8/opencv-2.4.8.exe/download
2) Download dll-installer http://vk.com/away.php?to=http%3A%2F%2Fge.tt%2F7w4g3IR1%2Fv%2F0%3Fc
3) Launch dll-installer (MS Visual studiio 2008+ must be installed before launching) with following parameters:
<path to dll-installer> <32/64 version of Windows> <MS Visual studio version>
Example: D:\dll-installer D:\ 64 13

4) Pull this repository.
5) Check following properties of project:
C/C++: Additional Include Directories = C:\opencv\build\include\opencv; C:\opencv\build\include;
Linker:
Additional Library Directories = C:\opencv\build\x64\vc12\lib;C:\opencv\build\include\opencv;C:\opencv\build\include;

