# Build wolfSSL manually for PureOS
# Selects only necessary source files (similar to BearSSL build)

$ErrorActionPreference = "Stop"

$WolfSSLDir = "D:\1os_copy_imp2_current_one_1os\src\net\lib\wolfssl\wolfssl"
$BuildDir = "D:\1os_copy_imp2_current_one_1os\build\wolfssl"
$ObjDir = "$BuildDir\obj"

# Fix paths for spaces
$WolfSSLDir = "D:\1os copy\imp2 current one\1os\src\net\lib\wolfssl\wolfssl"
$BuildDir = "D:\1os copy\imp2 current one\1os\build\wolfssl"
$ObjDir = "$BuildDir\obj"

Write-Host "=== Manual wolfSSL Build for PureOS ===" -ForegroundColor Cyan
Write-Host "WolfSSL Dir: $WolfSSLDir" -ForegroundColor Yellow
Write-Host "Obj Dir: $ObjDir" -ForegroundColor Yellow

# Create build directories
New-Item -ItemType Directory -Force -Path $ObjDir | Out-Null

# Key source files needed for TLS 1.3 client
$SourceFiles = @(
    # Crypto - Hash (SHA256, SHA384)
    "wolfcrypt\src\hash.c",
    "wolfcrypt\src\sha256.c",
    "wolfcrypt\src\sha512.c",
    
    # Crypto - HMAC
    "wolfcrypt\src\hmac.c",
    
    # Crypto - AES (for TLS)
    "wolfcrypt\src\aes.c",
    
    # Crypto - Random
    "wolfcrypt\src\random.c",
    
    # Crypto - RSA
    "wolfcrypt\src\rsa.c",
    
    # Crypto - ECC (for ECDHE)
    "wolfcrypt\src\ecc.c",
    
    # Crypto - Math (big integers)
    "wolfcrypt\src\integer.c",
    
    # Crypto - Memory
    "wolfcrypt\src\memory.c",
    
    # ASN.1 / X.509
    "wolfcrypt\src\asn.c",
    
    # TLS Core
    "wolfssl\src\tls.c",
    "wolfssl\src\internal.c",
    "wolfssl\src\io.c",
    
    # Error strings
    "wolfssl\src\error.c"
)

Write-Host "Source files to compile: $($SourceFiles.Count)" -ForegroundColor Yellow

# Compile each file
$CC = "i686-elf-gcc"
$CFlags = @("-m32", "-ffreestanding", "-O2", "-DWOLFSSL_USER_SETTINGS", 
             "-I`"$WolfSSLDir`"",
             "-I`"$WolfSSLDir\wolfssl`"",
             "-I`"$WolfSSLDir\wolfcrypt`"",
             "-I`"$WolfSSLDir\include`"")

$success = 0
$failed = 0

foreach ($src in $SourceFiles) {
    $srcPath = Join-Path $WolfSSLDir $src
    $baseName = [System.IO.Path]::GetFileNameWithoutExtension($src)
    $objPath = Join-Path $ObjDir "$baseName.o"
    
    Write-Host "Compiling: $src..." -NoNewline
    
    if (Test-Path $srcPath) {
        $cmd = "$CC -m32 -ffreestanding -O2 -DWOLFSSL_USER_SETTINGS -I`"$WolfSSLDir`" -I`"$WolfSSLDir\wolfssl`" -I`"$WolfSSLDir\wolfcrypt`" -I`"$WolfSSLDir\include`" -c `"$srcPath`" -o `"$objPath`""
        
        $result = Invoke-Expression $cmd 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host " OK" -ForegroundColor Green
            $success++
        } else {
            Write-Host " FAILED ($LASTEXITCODE)" -ForegroundColor Red
            Write-Host $result | Select-Object -First 2
            $failed++
        }
    } else {
        Write-Host " NOT FOUND ($srcPath)" -ForegroundColor Yellow
        $failed++
    }
}

Write-Host ""
Write-Host "=== Build Results ===" -ForegroundColor Cyan
Write-Host "Success: $success" -ForegroundColor Green
Write-Host "Failed: $failed" -ForegroundColor $(if ($failed -gt 0) { "Red" } else { "Green" })

# Create static library
if ($success -gt 0) {
    Write-Host "Creating library..." -ForegroundColor Yellow
    
    # Get all .o files
    $objFiles = Get-ChildItem $ObjDir -Filter "*.o" -ErrorAction SilentlyContinue | ForEach-Object { "`"" + $_.FullName + "`"" }
    $objList = $objFiles -join " "
    
    if ($objList) {
        $arCmd = "i686-elf-ar rcs `"$BuildDir\wolfssl.a`" $objList"
        Write-Host "AR Command: $arCmd" -ForegroundColor Gray
        
        Invoke-Expression $arCmd
        
        if ($LASTEXITCODE -eq 0 -and (Test-Path "$BuildDir\wolfssl.a")) {
            Write-Host "Library created: $BuildDir\wolfssl.a" -ForegroundColor Green
            Get-Item "$BuildDir\wolfssl.a" | Select-Object Name, Length
        } else {
            Write-Host "Library creation failed (exit: $LASTEXITCODE)" -ForegroundColor Red
        }
    } else {
        Write-Host "No object files found!" -ForegroundColor Red
    }
}

Write-Host "=== Build Complete ===" -ForegroundColor Cyan
