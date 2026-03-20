$ErrorActionPreference = "Stop"
if (!(Test-Path -Path "build\bearssl")) {
    New-Item -ItemType Directory -Force -Path "build\bearssl" | Out-Null
}

$files = Get-ChildItem "src\net\lib\bearssl\src" -Recurse -Filter *.c | Where-Object { $_.Name -notmatch "bundle" -and $_.Name -notmatch "aarch64" -and $_.Name -notmatch "pwr8" -and $_.Name -notmatch "sysrng" }
foreach ($f in $files) {
    if ([string]::IsNullOrWhiteSpace($f.FullName)) { continue }
    $src = $f.FullName
    $obj = "build\bearssl\" + $f.BaseName + ".o"
    
    # Only recompile if source is newer than object (simple make behavior)
    if (!(Test-Path $obj) -or (Get-Item $src).LastWriteTime -gt (Get-Item $obj).LastWriteTime) {
        Write-Host "Compiling $($f.Name)..."
        $process = Start-Process ".\tools\bin\x86_64-elf-gcc.exe" -ArgumentList "-ffreestanding -mno-red-zone -mno-mmx -O2 -mcmodel=large -c `"$src`" -o `"$obj`" -Iinclude -Isrc\kernel -Isrc\drivers -Isrc\net -Isrc\net\lib\bearssl\inc -Isrc\net\lib\bearssl\src" -NoNewWindow -Wait -PassThru
        if ($process.ExitCode -ne 0) {
            Write-Error "Failed to compile $($f.Name)"
            exit 1
        }
    }
}

Write-Host "Archiving BearSSL into bearssl.a..."
if (Test-Path "build\bearssl.a") {
    Remove-Item "build\bearssl.a"
}
$objFiles = (Get-ChildItem "build\bearssl\*.o" | Select-Object -ExpandProperty Name | Foreach-Object { "build\bearssl\$_" }) -join " "
$process = Start-Process ".\tools\bin\x86_64-elf-ar.exe" -ArgumentList "rcs build\bearssl.a $objFiles" -NoNewWindow -Wait -PassThru
if ($process.ExitCode -ne 0) {
    Write-Error "Failed to archive BearSSL"
    exit 1
}

Write-Host "BearSSL built successfully."
exit 0
