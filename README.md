frok-server
===========

face recognition odnoklassniki - server side

How to install in Linux:

Requirements:
1. Ubuntu 12.04 or higher (Linux Mint is made of the same core as ubuntu 12.04 so everything will be similar for Mint)
	Installing Ubuntu:
	1) Download ubuntu from http://www.ubuntu.com/download/desktop
	2a) VirtualBox:
		2.1. Download Oracle VirtualBox from https://www.virtualbox.org/wiki/Downloads
		2.2. Follow instruction http://www.psychocats.net/ubuntu/virtualbox
	2b) PC:
		2.1. Make bootable usb or dvd. Tool for making bootablie usb - http://unetbootin.net/download-unetbootin/
		2.2. Make some free space for installation (RAM size * 2 (for swap area) + ~20GB)
			how to "diskmanagement in Windows 7" http://airstream.pro/2011/09/23/how-to-use-disk-management-on-windows7/
		2.3. Boot ubuntu image (Check your BIOS->BOOT settings for booting usb)
		2.4. Follow installation tips. Choose 2 * RAM size of free space for swap area and rest of free space for being mounted to "/"
		2.5. Installation complete. Use google for help.
2. compilation tools 4.8.2 or higher
	sudo apt-get update
	sudo apt-get dist-upgrade	#should be used for Ubuntu 14.04 Desktop only
	sudo apt-get install gcc
	sudo apt-get install g++
3. git 1.9.1 or higher
	sudo apt-get install git
4. QT creator 4.8 or higher (I use 4.8 on Linux Mint and 5.3 on Ubuntu)
	3.1. Download "Qt Online Installer for Linux" from http://qt-project.org/downloads
	3.2. Run downloaded script, choose all tools and gcc only installation (we don't need cross-platform compilers). Destination folder should be /opt
	3.3. Run "/opt/Qt/Tools/QtCretor/bin/qtcreator" to launch qtcreator

5. cmake
	sudo apt-get install cmake
6. GTK+2.0 or higher
	apt-get install libgtk2.0-dev
7. opencv-2.4.9
	3.1. download .zip from http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip/download
	3.2. Unpack archieve to /opt/
		sudo unzip opencv-2.4.9.zip -d /opt/
	3.3. Make opencv from sources:
		cd /opt/opencv-2.4.9/
		mkdir static
		sudo cmake -D CMAKE_BUILD_TYPE=RELEASE -D BUILD_SHARED_LIBS=NO -D CMAKE_INSTALL_PREFIX=/opt/opencv-2.4.9/static -D WITH_QT=NO ./
		sudo make
		sudo make install
    3.4. Set opencv paths
        gedit ~/.bashrc
        add 'export LD_LIBRARY_PATH=/opt/opencv-2.4.9/static/lib/:$LD_LIBRARY_PATH' to the end of file, save and exit
        reboot

Issues:
	1. undefined reference to symbol 'pthread_mutexattr_settype@@GLIBC_2.2.5'
		Solution: 	
			change '-L/usr/lib/x86_64-linux-gnu' in *.mk files to your linux-gnu path
			Use this command to see your linux-gnu path:
				find /usr/lib | grep linux-gnu
    2. When running from QtCreatod: error while oading shared libraries: libopencv_contrib.so ...
        Solution:
            Open Projects tab->build->Build Environment (on the bottom)
            add environment variable:
            name - LD_LIBRARY_PATH, value - /opt/opencv-2.4.9/install/lib/

Build steps:        [TBD] more info + QtCreator info
1. in terminal go to FaceDetection library
    cd ./build
    make build 

How to install in Windows:
1. Donwload and install to C:\ opencv from http://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.8/opencv-2.4.8.exe/download
2. Download dll-installer https://www.dropbox.com/s/ue6ub71k53tvxwi/opencv_inst.rar
3. Launch dll-installer as admin (MS Visual studiio 2008+ must be installed before launching) with following parameters:
    <path to dll-installer> <32/64 version of Windows> <MS Visual studio version>
        Example: D:\dll-installer D:\ 64 13
4. Pull this repository.
5. Check following properties of project:
    C/C++: 
        Additional Include Directories = C:\opencv\build\include\opencv; C:\opencv\build\include;
    Linker:
        Additional Library Directories = C:\opencv\build\x64\vc12\lib;C:\opencv\build\include\opencv;C:\opencv\build\include;
        Additional Dependances = opencv_core248d.lib opencv_imgproc248d.lib opencv_highgui248d.lib opencv_video248d.lib opencv_objdetect248d.lib opencv_calib3d248d.lib opencv_features2d248d.lib opencv_nonfree248d.lib opencv_flann248d.lib opencv_legacy248d.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib
        Import Library = opencv_core248d.lib opencv_imgproc248d.lib opencv_highgui248d.lib opencv_video248d.lib opencv_objdetect248d.lib opencv_calib3d248d.lib opencv_features2d248d.lib opencv_nonfree248d.lib opencv_flann248d.lib opencv_legacy248d.lib $(TargetDir)$(TargetName).lib,user32.lib,kernel32.lib,user32.lib,gdi32.lib,winspool.lib,comdlg32.lib,advapi32.lib,shell32.lib,ole32.lib,oleaut32.lib,uuid.lib,odbc32.lib,odbccp32.lib
    P.S. note that Additional Library Directories should consider path to libraries with current configuration (C:\opencv\build\x86... for 64 bin, C:\opencv\build\x86 for 32)
6. Download and install Doxygen http://www.stack.nl/~dimitri/doxygen/download.html
7. Run as admin in cmd
    "REG ADD HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System /v EnableLUA /t REG_DWORD /d 0 /f"
8.  Указать пути к папкам где будут сохраняться фотки и модели юзеров в файле FaceDetectionLib/activities.h
    Например:
    define ID_PATH				"Z:\\frok\\"
    define TARGET_PATH			"Z:\\frok\\1\\"
9. Build project and see FaceDetection_manual for additional information
