# Build wolfSSL for PureOS
# This script downloads and builds wolfSSL for i686-elf cross-compilation

$ErrorActionPreference = "Stop"

$WolfSSLDir = "D:\1os copy\imp2 current one\1os\src\net\lib\wolfssl"
$WolfSSLSrcDir = "$WolfSSLDir\wolfssl"
$BuildDir = "$WolfSSLDir\build"

Write-Host "=== Building wolfSSL for PureOS ===" -ForegroundColor Cyan

# Check if already downloaded
if (Test-Path $WolfSSLSrcDir) {
    Write-Host "wolfSSL source already exists, skipping download" -ForegroundColor Yellow
} else {
    # Download wolfSSL
    Write-Host "Downloading wolfSSL..." -ForegroundColor Green
    $zipUrl = "https://github.com/wolfSSL/wolfssl/archive/refs/tags/v5.7.2.zip"
    $zipFile = "$WolfSSLDir\wolfssl.zip"
    
    try {
        Invoke-WebRequest -Uri $zipUrl -OutFile $zipFile -UseBasicParsing
        Write-Host "Downloaded wolfSSL v5.7.2" -ForegroundColor Green
    } catch {
        Write-Host "Failed to download, trying master branch..." -ForegroundColor Yellow
        $zipUrl = "https://github.com/wolfSSL/wolfssl/archive/refs/heads/master.zip"
        Invoke-WebRequest -Uri $zipUrl -OutFile $zipFile -UseBasicParsing
    }
    
    # Extract
    Write-Host "Extracting..." -ForegroundColor Green
    Expand-Archive -Path $zipFile -DestinationPath $WolfSSLDir -Force
    
    # Rename directory
    Get-ChildItem $WolfSSLDir -Directory | Where-Object { $_.Name -like "wolfssl*" } | ForEach-Object {
        Move-Item $_.FullName $WolfSSLSrcDir -Force
    }
    
    Remove-Item $zipFile -ErrorAction SilentlyContinue
}

# Copy user_settings.h
Write-Host "Copying user_settings.h..." -ForegroundColor Green
Copy-Item "$WolfSSLDir\user_settings.h" "$WolfSSLSrcDir\user_settings.h" -Force

# Try to configure (this may fail on Windows)
Write-Host "Attempting autotools configuration..." -ForegroundColor Yellow
Set-Location $WolfSSLSrcDir

# Check if configure exists
if (Test-Path "configure") {
    Write-Host "Running configure..." -ForegroundColor Green
    .\configure --help 2>&1 | Out-Null
    
    # Try to configure for i686-elf
    $configureResult = & .\configure --host=i686-elf --prefix=$BuildDir `
        --enable-usersettings `
        --disable-shared `
        --enable-static `
        --disable-debug `
        --disable-documents `
        --enable-all-crypto 2>&1
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Configure succeeded!" -ForegroundColor Green
        Write-Host "Running make..." -ForegroundColor Green
        make -j4 2>&1
    } else {
        Write-Host "Configure failed, trying alternate method..." -ForegroundColor Yellow
        Write-Host $configureResult
    }
} else {
    Write-Host "No configure script found (needs autotools on Linux/WSL)" -ForegroundColor Yellow
}

Write-Host "=== Build attempt complete ===" -ForegroundColor Cyan
Write-Host "Manual configuration may be needed" -ForegroundColor Yellow
