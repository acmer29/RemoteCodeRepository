cd Debug
start RemoteRepository.exe
cd ../GUI/bin/Debug
start GUI.exe --p 8081 --u Administrator --d true
@echo off 
choice /t 2 /d y /n >nul 
start GUI.exe --p 8082 --u Tianyu --d true
cd ../../../