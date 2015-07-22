@echo off

echo Pushing source translation (en) to transifex
tx.exe push -s -f --no-interactive -l en

if "%1" == "all" goto all

echo Pushing Russian translation to transifex
tx.exe push -t -f --no-interactive -l ru
goto end

:all
echo Pushing all translations to transifex
tx.exe push -t -f --no-interactive -l ru,pl,de,uk,es,nl,ja --skip

:end
echo Done!

