if [%packages%] == [] packages=packages
mkdir %packages%\%1\data\resources\%2
for %%r in (%resources%) do copy c:\eyecu\resources\%2\%%r %packages%\%1\data\resources\%2 /Y