@echo off
del app.exe

gcc -w -I D:\dist_pkg\third_party\opencl-1.2.0-win64\include -o ./app.exe ./ocl.c  -L D:\dist_pkg\third_party\opencl-1.2.0-win64\lib -lOpenCL
call "app.exe"
del app.exe

g++ -w -o ./app.exe ./vox.cpp
call "app.exe"
del app.exe

set /p DUMMY=done