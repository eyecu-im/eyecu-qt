@echo off
del packages\ru.rwsoftware.eyecu\meta\LICENSE.TXT
for /d %%p in (packages\*) do rmdir %%p\data /S /Q
