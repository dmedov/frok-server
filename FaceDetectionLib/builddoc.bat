set DOXYGEN_FILE_DIRECTORY=%1
set DOXYGEN_FILE=%2

echo %DOXYGEN_FILE_DIRECTORY%
echo %DOXYGEN_FILE%

pushd %DOXYGEN_FILE_DIRECTORY%

..\tools\dOxygen\bin\doxygen.exe %DOXYGEN_FILE%
..\tools\generate_chm\scripts\hhc.exe .\doc\index.hhp
del /F ".\..\FaceDetection_manual.chm"
copy /Y ".\doc\index.chm" ".\..\FaceDetection_manual.chm"

popd