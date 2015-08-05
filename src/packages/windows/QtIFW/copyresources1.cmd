if "%packages%" == "" packages=packages
for %%r in (%resources%) do xcopy c:\eyecu\resources\%2\%%r %packages%\%1\data\resources\%2 /S /Y