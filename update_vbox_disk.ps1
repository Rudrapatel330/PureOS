param (
    [Parameter(Mandatory=$true, HelpMessage="Name of your VirtualBox VM")]
    [string]$VMName,
    
    [Parameter(Mandatory=$true, HelpMessage="Path to your new .vdi file")]
    [string]$NewVdiFile
)

$VBoxManage = "C:\Program Files\Oracle\VirtualBox\VBoxManage.exe"

if (-not (Test-Path $VBoxManage)) {
    Write-Error "VBoxManage not found. Is VirtualBox installed in the default location?"
    exit
}

if (-not (Test-Path $NewVdiFile)) {
    Write-Error "The specified VDI file does not exist: $NewVdiFile"
    exit
}

$NewVdiFile = (Resolve-Path $NewVdiFile).Path

Write-Host "Analyzing VM '$VMName'..."
$vmInfo = & $VBoxManage showvminfo "$VMName" --machinereadable

if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to get info for VM '$VMName'. Does it exist?"
    exit
}

$attachedDisk = $null
$controller = $null
$port = $null
$device = $null

# Parse the VM info to find the currently attached .vdi file
foreach ($line in $vmInfo) {
    if ($line -match '^"([^"]+)-(\d+)-(\d+)"="([^"]+\.vdi)"') {
        $controller = $matches[1]
        $port = $matches[2]
        $device = $matches[3]
        $attachedDisk = $matches[4]
        break
    }
}

if ($attachedDisk) {
    Write-Host "Found currently attached disk: $attachedDisk"
    Write-Host "Detaching from Controller: '$controller', Port: $port, Device: $device"
    
    # 1. Detach the old disk from the VM
    & $VBoxManage storageattach "$VMName" --storagectl "$controller" --port $port --device $device --type hdd --medium none
    
    # 2. Unregister and delete the old disk from the VirtualBox Media Registry
    Write-Host "Unregistering and deleting old VDI from VirtualBox..."
    & $VBoxManage closemedium disk "$attachedDisk" --delete
} else {
    Write-Host "No existing .vdi attached to VM '$VMName' found."
    $controller = Read-Host "Please type the Storage Controller Name to attach to (e.g. IDE, SATA)"
    $port = 0
    $device = 0
}

# 3. Attach the new disk
Write-Host "Attaching new disk '$NewVdiFile' to VM '$VMName'..."
& $VBoxManage storageattach "$VMName" --storagectl "$controller" --port $port --device $device --type hdd --medium "$NewVdiFile"

Write-Host ""
Write-Host "Success! The VM '$VMName' is now using '$NewVdiFile'." -ForegroundColor Green
