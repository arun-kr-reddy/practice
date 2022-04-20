@echo off
del app.exe
clear
gcc -o ./exe_main.exe ./main.c -lOpenCL
call "exe_main.exe"
