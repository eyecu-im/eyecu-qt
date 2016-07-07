echo off
set packagename=eyecu-win-extra
set version=1.3.0
set packagefilename=%packagename%-%version%
set devpackagefilename=%devpackagename%-%version%
set packages=packages-extra

call copyplugins ru.rwsoftware.eyecu.scheduler scheduler

repogen.exe -p %packages% repository-extra