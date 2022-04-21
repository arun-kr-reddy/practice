@echo off
del app.exe

gcc -I D:\dist_pkg\third_party\opencl-1.2.0-win64\include -o ./app.exe ./main.c -L D:\dist_pkg\third_party\opencl-1.2.0-win64\lib -lOpenCL
set /p DUMMY=compilation done

call "app.exe"