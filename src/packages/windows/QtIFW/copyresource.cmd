@echo off
if "%packages%"="" packages=packages
xcopy c:\eyecu\resources\%2\* %packages%\%1\data\ /S /Y