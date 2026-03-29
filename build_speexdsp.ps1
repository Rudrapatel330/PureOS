$ErrorActionPreference = "Stop"
if (!(Test-Path -Path "build\speexdsp")) {
    New-Item -ItemType Directory -Force -Path "build\speexdsp" | Out-Null
}

$files = Get-ChildItem "src\lib\speexdsp\libspeexdsp" -Filter *.c | Where-Object { $_.Name -notmatch "^test" }
foreach ($f in $files) {
    if ([string]::IsNullOrWhiteSpace($f.FullName)) { continue }
    $src = $f.FullName
    $obj = "build\speexdsp\" + $f.BaseName + ".o"
    
    if (!(Test-Path $obj) -or (Get-Item $src).LastWriteTime -gt (Get-Item $obj).LastWriteTime) {
        Write-Host "Compiling $($f.Name)..."
        $process = Start-Process ".\tools\bin\x86_64-elf-gcc.exe" -ArgumentList "-ffreestanding -mno-red-zone -mno-mmx -O2 -mcmodel=large -DFIXED_POINT -DUSE_KISS_FFT -DEXPORT=`"`" -c `"$src`" -o `"$obj`" -Iinclude -Isrc\kernel -Isrc\lib\speexdsp\include -Isrc\lib\speexdsp\libspeexdsp" -NoNewWindow -Wait -PassThru
        if ($process.ExitCode -ne 0) {
            Write-Error "Failed to compile $($f.Name)"
            exit 1
        }
    }
}

Write-Host "Archiving SpeexDSP into speexdsp.a..."
if (Test-Path "build\speexdsp.a") {
    Remove-Item "build\speexdsp.a"
}
$objFiles = (Get-ChildItem "build\speexdsp\*.o" | Select-Object -ExpandProperty Name | Foreach-Object { "build\speexdsp\$_" }) -join " "
$process = Start-Process ".\tools\bin\x86_64-elf-ar.exe" -ArgumentList "rcs build\speexdsp.a $objFiles" -NoNewWindow -Wait -PassThru
if ($process.ExitCode -ne 0) {
    Write-Error "Failed to archive SpeexDSP"
    exit 1
}

Write-Host "SpeexDSP built successfully."
exit 0
