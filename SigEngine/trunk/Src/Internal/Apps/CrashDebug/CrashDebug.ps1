$folder = '\\shares\Shared\QA\Rise\Crashdumps' # Root path you want to monitor. 
$Dfolder = '\\shares\Shared\QA\Rise\Crashdumps\kocchiuto-xbox\Crash_Analyzer\Crash_Debug' # Folder to create crash logs
$filter = '*.dmp' # Wildcard filter here.

#Initiate File System Watcher
$fsw = New-Object IO.FileSystemWatcher $folder, $filter -Property @{
IncludeSubdirectories = $true
NotifyFilter = [IO.NotifyFilters]'LastWrite'
EnableRaisingEvents = $true
}

$onCreated = Register-ObjectEvent $fsw Changed -SourceIdentifier FileChanged1 -Action { 
$path = $Event.SourceEventArgs.FullPath
$timeStamp = $Event.TimeGenerated 

$today = (Get-Date -Format MM-dd-yyyy)
$callS = New-Item $Dfolder\$today -type directory -Force # Check for todays dir and create if necessary
$outfile = (Get-Date -Format MM-dd-yyyy-hh-mm-ss) + '_CallStack.txt' # Text file with crash info

# Count number of new crash files today
$fcount = $((Get-ChildItem $Dfolder\$today).count)

sleep -Seconds 3

# Running WinDbg and creating callstack file
Set-Alias WinDbg "C:\SignalStudios\Rise\SigEngine\Src\External\Microsoft Xbox 360 SDK\bin\win32\windbg.exe"
$debugcommands = "`"k;q`""

# Execute WinDbg with commands 
WinDbg -c $debugcommands -noshell -logo $callS\$outfile -z $path
sleep -Seconds 5
(gc $callS\$outfile | select -Skip 30) | Foreach-Object {$_ -replace "quit:", ""} | sc $callS\$outfile -Force
sleep -Seconds 5

# E-mail crash call stack

$To = "kocchiuto@signalstudios.net", "cbramwell@signalstudios.net";
$from = "bugs@signalstudios.net"
$smtpServer = "smtp.office365.com"
$subject = "A crash has occured at $timeStamp $path"
$body = (Get-Content $callS\$outfile | out-string) + " $fcount new crashes have occured on $today "

#SendEmailFunction  
foreach ($To in $To)  
{  
Write-Host "Sending Email notification to $To" 

$SMTPMessage = New-Object System.Net.Mail.MailMessage($From,$To,$Subject,$Body)
$smtpclient = New-Object Net.Mail.SmtpClient($smtpServer)
$SMTPClient.Credentials = New-Object System.Net.NetworkCredential("bugs@signalstudios.net", "6aJ1jaRzkzgo");
$smtpClient.EnableSsl = $true
$smtpclient.Send($From,$To,$subject,$body)
}
}

#Show-BalloonTip -Title “WARNING!!!” -MessageType Error -Message “A Crash Has Occured” -Duration 1000

# To stop the monitoring, run the following commands:
#Unregister-Event FileChanged1