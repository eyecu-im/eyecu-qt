@echo off
del packages\ru.rwsoftware.eyecu\meta\LICENSE.TXT
del packages\ru.rwsoftware.eyecu\meta\installscript.js
for /d %%p in (packages\*) do rmdir %%p\data /S /Q
