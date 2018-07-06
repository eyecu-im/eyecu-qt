@echo off

del config\config.xml
del config\controlscript.js

del packages\ru.rwsoftware.eyecu\meta\LICENSE.TXT

del packages\com.microsoft.vcredist\meta\* /Q

for /d %%p in (packages\org.digia.qt5.* packages\org.icuproject.icu packages\org.digia.qt4.*) do rmdir %%p /S /Q

for /d %%p in (packages\*) do rmdir %%p\data /S /Q

for %%f in (adiummessagestyle mapshearch.navitel messagearchiver mmplayer pepmanager.geoloc.positioning.serialport) do del packages\ru.rwsoftware.eyecu.%%f\meta\package.xml /Q

for %%f in (qtpurple qtpurple.ffmpeg) do del packages\ru.purplesoft.%%f\meta\package.xml /Q
