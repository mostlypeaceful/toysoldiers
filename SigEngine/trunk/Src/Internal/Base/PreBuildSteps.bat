powershell.exe -command Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
powershell.exe -command Push-Location $SigEngine ; ".\PreBuildSteps.ps1" ; Pop-Location