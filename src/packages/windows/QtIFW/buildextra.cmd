echo on

set platform=x86
set packagename=eyecu-win-extra
set version=2.0.0.20190515
set packagefilename=%packagename%-%version%
set devpackagefilename=%devpackagename%-%version%
set packages=packages-extra

call copyplugins ru.rwsoftware.eyecu.scheduler scheduler
call copyresources ru.rwsoftware.eyecu.statusicons.qip statusicons\qip

set repository=repository-extra
if %platform%==x64 set repository=%repository%.x64
repogen.exe -p %packages% %repository%

pause