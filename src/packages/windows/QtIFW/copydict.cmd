if "%packages%" == "" packages=packages
md %packages%\ru.rwsoftware.eyecu.spellchecker.dict_%1\data\hunspell
copy c:\eyecu\hunspell\%2.* %packages%\ru.rwsoftware.eyecu.spellchecker.dict_%1\data\hunspell\*