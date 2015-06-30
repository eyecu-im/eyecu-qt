if not "%2" == "" set resources=%2
for %%r in (%resources%) do xcopy c:\eyecu\resources\%%r\* %1\resources\%%r\ /S