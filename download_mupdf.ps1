$ErrorActionPreference = "Stop"

if (!(Test-Path "src/lib/mupdf-src")) {
    Write-Host "Downloading MuPDF 1.23.0 Source..."
    # MuPDF archives are typically at mupdf.com/downloads/archive/
    Invoke-WebRequest -Uri "https://mupdf.com/downloads/archive/mupdf-1.23.0-source.tar.gz" -OutFile "mupdf.tar.gz"
    
    Write-Host "Extracting MuPDF Source..."
    tar -xf mupdf.tar.gz
    Move-Item -Path "mupdf-1.23.0-source" -Destination "src/lib/mupdf-src" -Force
    Remove-Item "mupdf.tar.gz"
}

Write-Host "Source downloaded and extracted."
