$AR = ".\tools\bin\x86_64-elf-ar.exe"
$objs = Get-ChildItem "build\mupdf\*.o"
Write-Host "Total .o files: $($objs.Count)"

$archive = "build\mupdf.a"
if (Test-Path $archive) { Remove-Item $archive }

# Try archiving all at once using full paths
$paths = $objs | ForEach-Object { $_.FullName }
Write-Host "First path: $($paths[0])"
Write-Host "Last path: $($paths[$paths.Count - 1])"

& $AR rcs $archive @paths
Write-Host "Exit code: $LASTEXITCODE"

if ($LASTEXITCODE -eq 0) {
    $size = (Get-Item $archive).Length
    Write-Host "Archive created! Size: $size bytes"
}
else {
    Write-Host "Archive FAILED. Trying with quoted paths..."
    # Fall back - write paths to file and use @file
    $paths | Out-File -Encoding ASCII "build\ar_files.txt"
    & cmd.exe /c "`"$AR`" rcs `"$archive`" @build\ar_files.txt"
    Write-Host "Cmd exit: $LASTEXITCODE"
}
