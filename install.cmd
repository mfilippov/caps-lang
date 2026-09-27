@echo off
rem Install or update Caps Lang for the current user: checks the Authenticode signature,
rem copies the exe for this machine's architecture to C:\ProgramData\caps-lang\capslang.exe
rem and registers (or replaces) the "Caps Lang" logon task.
rem Run it from the folder with capslang-x64.exe / capslang-arm64.exe and capslang-task.xml.
setlocal
rem When started from PowerShell 7, its PSModulePath breaks module loading in Windows
rem PowerShell 5.1 (Get-AuthenticodeSignature); let powershell.exe use its defaults.
set "PSModulePath="

set "ARCH=%PROCESSOR_ARCHITECTURE%"
if defined PROCESSOR_ARCHITEW6432 set "ARCH=%PROCESSOR_ARCHITEW6432%"
set "EXE="
if /i "%ARCH%"=="AMD64" set "EXE=capslang-x64.exe"
if /i "%ARCH%"=="ARM64" set "EXE=capslang-arm64.exe"
if not defined EXE goto :unsupported

set "SRC=%~dp0%EXE%"
set "XML=%~dp0capslang-task.xml"
set "DEST=%ProgramData%\caps-lang"
if not exist "%SRC%" goto :missing
if not exist "%XML%" goto :missing

rem Refuse anything that is not validly signed by us.
powershell -NoProfile -ExecutionPolicy Bypass -Command "$s = Get-AuthenticodeSignature -LiteralPath $env:SRC; if ($s.Status -ne 'Valid' -or $s.SignerCertificate.Subject -notlike 'CN=Mikhail Filippov,*') { Write-Host ('Signature check failed: ' + $s.Status + ' ' + $s.SignerCertificate.Subject); exit 1 }"
if errorlevel 1 exit /b 1

rem A running copy locks the exe.
taskkill /im capslang.exe /f >nul 2>&1
if not exist "%DEST%" mkdir "%DEST%" || exit /b 1
copy /y "%SRC%" "%DEST%\capslang.exe" >nul || exit /b 1
rem Drop the Mark of the Web (the signature was checked above).
powershell -NoProfile -ExecutionPolicy Bypass -Command "Unblock-File -LiteralPath (Join-Path $env:DEST 'capslang.exe')"

schtasks /create /xml "%XML%" /tn "Caps Lang" /f || exit /b 1
schtasks /run /tn "Caps Lang" >nul || exit /b 1
echo Caps Lang installed: %DEST%\capslang.exe
exit /b 0

:unsupported
echo Unsupported architecture: %ARCH%
exit /b 1

:missing
echo Put %EXE% and capslang-task.xml next to install.cmd.
exit /b 1
