if [%packages%] == [] packages=packages
if not [%2] == [] set resources=%2
for %%r in (%resources%) do xcopy c:\eyecu\resources\%%r\* %packages%\%1\data\resources\%%r\ /S /Y