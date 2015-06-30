md %1
md %1\plugins
set locales=de es ja nl pl ru uk
if not "%2" == "" set pluginlist=%2
FOR %%P IN (%pluginlist%) DO copy c:\eyecu\plugins\%%P.dll %1\plugins\
md %1\translations
FOR %%L IN (%locales%) DO md %1\translations\%%L
FOR %%L IN (%locales%) DO FOR %%P IN (%pluginlist%) DO copy c:\eyecu\translations\%%L\%%P.qm %1\translations\%%L\
