if "%packages%" == "" packages=packages
if not "%2" == "" set resources=%2
if not "%3" == "" set files=%3
for %%r in (%resources%) do for %%f in (%files%) do xcopy c:\eyecu\resources\%%r\%%f %packages%\%1\data\resources\%%r\ /S /Y