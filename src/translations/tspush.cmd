@echo off

echo Pushing Russian translation to transifex
tx.exe push -t -f --no-interactive -l ru

echo Pushing source translation (en) to transifex
tx.exe push -s -f --no-interactive -l en