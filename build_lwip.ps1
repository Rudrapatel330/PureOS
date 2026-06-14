$gcc = ".\tools\bin\x86_64-elf-gcc.exe"
$ar = ".\tools\bin\x86_64-elf-ar.exe"
$cflags = "-ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -Wno-implicit-function-declaration -Wno-int-conversion -Wno-incompatible-pointer-types -mcmodel=large -Iinclude -Isrc/net/lwip/src/include -Isrc/net/lwip_port"

$srcDirs = @(
    "src/net/lwip/src/api",
    "src/net/lwip/src/core",
    "src/net/lwip/src/core/ipv4",
    "src/net/lwip/src/netif",
    "src/net/lwip_port"
)

$objFiles = @()

foreach ($dir in $srcDirs) {
    $cFiles = Get-ChildItem -Path $dir -Filter "*.c" -File
    foreach ($file in $cFiles) {
        $outFile = "build\" + $file.BaseName + "_lwip.o"
        $objFiles += $outFile
        Write-Host "Compiling $($file.Name)..."
        $cmd = "$gcc $cflags -c `"$($file.FullName)`" -o `"$outFile`""
        Invoke-Expression $cmd
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Failed to compile $($file.Name)"
            exit 1
        }
    }
}

Write-Host "Archiving to build\lwip.a..."
$arCmd = "$ar rcs build\lwip.a " + ($objFiles -join " ")
Invoke-Expression $arCmd
if ($LASTEXITCODE -ne 0) {
    Write-Error "Failed to create lwip.a"
    exit 1
}

Write-Host "lwIP Build Complete."
