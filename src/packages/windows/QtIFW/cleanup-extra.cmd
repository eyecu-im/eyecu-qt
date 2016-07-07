@echo off
for /d %%p in (packages-extra\*) do rmdir %%p\data /S /Q
