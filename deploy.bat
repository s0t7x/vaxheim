@echo off
setlocal enableDelayedExpansion
set "baseName=Vaxheim_b"
set /p n=<buildNumber

rm -rf ./x64/Deploy/
cd x64
mkdir Deploy
cd Deploy

robocopy "../Release/" "./" "Vaxheim.exe"
robocopy "../Release/" "./" "Vaxheim_QT.exe"
mkdir assets
robocopy "../../assets" "./assets"
windeployqt.exe Vaxheim_QT.exe

for /f "delims=" %%F in (
  '2^>nul dir /b /ad "%baseName%*"^|findstr /xri /c:"%baseName%[0-9]*"'
) do (
  set "name=%%F"
  set "name=!name:*%baseName%=!"
  if !name! gtr !n! set "n=!name!"
)
set /a n+=1
cd ../../

7z a -tzip -r ./x64/%baseName%%n%_%date%.zip ./x64/Deploy/*
echo %n% > buildNumber

pause