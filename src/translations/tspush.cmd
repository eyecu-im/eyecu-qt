@echo off

echo Pushing Russian translation to transifex
tx.exe push -t -f --no-interactive -l ru
