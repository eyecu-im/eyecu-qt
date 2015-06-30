@echo off
if "%packages%" == "" packages=packages
md %packages%\%1\data\plugins
set locales=de es ja nl pl ru uk
if not "%2" == "" set pluginlist=%2
FOR %%P IN (%pluginlist%) DO copy c:\eyecu\plugins\%%P.dll %packages%\%1\data\plugins\ /Y
set transdir=%1
if "%1"=="ru.rwsoftware.eyecu" set transdir=%1.translations
if "%1"=="ru.rwsoftware.eyecu" set pluginlist=pluginlist% eyecu eyecuutils
FOR %%L IN (%locales%) DO md %packages%\%transdir%.%%L\data\translations\%%L
FOR %%L IN (%locales%) DO FOR %%P IN (%pluginlist%) DO copy c:\eyecu\translations\%%L\%%P.qm %packages%\%transdir%.%%L\data\translations\%%L\ /Y
