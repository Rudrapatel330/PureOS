# Kill Bochs processes
Write-Host "Killing Bochs processes..."
Stop-Process -Name "bochs" -Force -ErrorAction SilentlyContinue
Stop-Process -Name "bochsdbg" -Force -ErrorAction SilentlyContinue

# Delete lock files
Write-Host "Removing lock files..."
Get-ChildItem -Path . -Filter "*.lock" | foreach { 
    Write-Host "Removing $($_.Name)"
    Remove-Item -Path $_.FullName -Force -ErrorAction SilentlyContinue 
}

# Wait to ensure locks are released
Start-Sleep -Seconds 2

Write-Host "Cleanup complete."
