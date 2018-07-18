for /f %%i in (buildcount.txt) do set buildcount=%%i
set /a buildcount=buildcount+1
echo %buildcount%
echo %buildcount% > buildcount.txt
