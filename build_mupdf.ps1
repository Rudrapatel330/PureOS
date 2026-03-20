$ErrorActionPreference = "Stop"

$CC = ".\tools\bin\x86_64-elf-gcc.exe"
$AR = ".\tools\bin\x86_64-elf-ar.exe"

$OUT_DIR = "build\mupdf"
# if (Test-Path $OUT_DIR) { Remove-Item -Recurse -Force $OUT_DIR }
if (!(Test-Path $OUT_DIR)) { New-Item -ItemType Directory -Path $OUT_DIR | Out-Null }

$CFLAGS = "-ffreestanding -mno-red-zone -mno-mmx -O2 -Wall -mcmodel=large -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter -Wno-int-conversion -Wno-incompatible-pointer-types"
$INCLUDES = "-Iinclude -Isrc/kernel -Isrc/kernel/hal -Imupdf/include"

$ZLIB_SRC = @(
    "mupdf/thirdparty/zlib/adler32.c", "mupdf/thirdparty/zlib/compress.c", "mupdf/thirdparty/zlib/crc32.c",
    "mupdf/thirdparty/zlib/deflate.c", "mupdf/thirdparty/zlib/inffast.c", "mupdf/thirdparty/zlib/inflate.c",
    "mupdf/thirdparty/zlib/inftrees.c", "mupdf/thirdparty/zlib/trees.c", "mupdf/thirdparty/zlib/uncompr.c",
    "mupdf/thirdparty/zlib/zutil.c"
)

$LIBJPEG_SRC = @(
    "mupdf/thirdparty/libjpeg/jaricom.c", "mupdf/thirdparty/libjpeg/jcapimin.c", "mupdf/thirdparty/libjpeg/jcapistd.c", "mupdf/thirdparty/libjpeg/jcarith.c", "mupdf/thirdparty/libjpeg/jccoefct.c", "mupdf/thirdparty/libjpeg/jccolor.c", "mupdf/thirdparty/libjpeg/jcdctmgr.c", "mupdf/thirdparty/libjpeg/jchuff.c", "mupdf/thirdparty/libjpeg/jcinit.c", "mupdf/thirdparty/libjpeg/jcmainct.c", "mupdf/thirdparty/libjpeg/jcmarker.c", "mupdf/thirdparty/libjpeg/jcmaster.c", "mupdf/thirdparty/libjpeg/jcomapi.c", "mupdf/thirdparty/libjpeg/jcparam.c", "mupdf/thirdparty/libjpeg/jcprepct.c", "mupdf/thirdparty/libjpeg/jcsample.c", "mupdf/thirdparty/libjpeg/jdapimin.c", "mupdf/thirdparty/libjpeg/jdapistd.c", "mupdf/thirdparty/libjpeg/jdarith.c", "mupdf/thirdparty/libjpeg/jdatadst.c", "mupdf/thirdparty/libjpeg/jdatasrc.c", "mupdf/thirdparty/libjpeg/jdcoefct.c", "mupdf/thirdparty/libjpeg/jdcolor.c", "mupdf/thirdparty/libjpeg/jddctmgr.c", "mupdf/thirdparty/libjpeg/jdhuff.c", "mupdf/thirdparty/libjpeg/jdinput.c", "mupdf/thirdparty/libjpeg/jdmainct.c", "mupdf/thirdparty/libjpeg/jdmarker.c", "mupdf/thirdparty/libjpeg/jdmaster.c", "mupdf/thirdparty/libjpeg/jdmerge.c", "mupdf/thirdparty/libjpeg/jdpostct.c", "mupdf/thirdparty/libjpeg/jdsample.c", "mupdf/thirdparty/libjpeg/jdtrans.c", "mupdf/thirdparty/libjpeg/jerror.c", "mupdf/thirdparty/libjpeg/jfdctflt.c", "mupdf/thirdparty/libjpeg/jfdctfst.c", "mupdf/thirdparty/libjpeg/jfdctint.c", "mupdf/thirdparty/libjpeg/jidctflt.c", "mupdf/thirdparty/libjpeg/jidctfst.c", "mupdf/thirdparty/libjpeg/jidctint.c", "mupdf/thirdparty/libjpeg/jmemmgr.c", "mupdf/thirdparty/libjpeg/jquant1.c", "mupdf/thirdparty/libjpeg/jquant2.c", "mupdf/thirdparty/libjpeg/jutils.c"
)

$FREETYPE_SRC = @(
    "mupdf/thirdparty/freetype/src/base/ftbase.c", "mupdf/thirdparty/freetype/src/base/ftbbox.c", "mupdf/thirdparty/freetype/src/base/ftbitmap.c", "mupdf/thirdparty/freetype/src/base/ftdebug.c", "mupdf/thirdparty/freetype/src/base/ftfstype.c", "mupdf/thirdparty/freetype/src/base/ftgasp.c", "mupdf/thirdparty/freetype/src/base/ftglyph.c", "mupdf/thirdparty/freetype/src/base/ftinit.c", "mupdf/thirdparty/freetype/src/base/ftstroke.c", "mupdf/thirdparty/freetype/src/base/ftsynth.c", "mupdf/thirdparty/freetype/src/base/ftsystem.c", "mupdf/thirdparty/freetype/src/base/fttype1.c",
    "mupdf/thirdparty/freetype/src/cff/cff.c", "mupdf/thirdparty/freetype/src/cid/type1cid.c", "mupdf/thirdparty/freetype/src/psaux/psaux.c", "mupdf/thirdparty/freetype/src/pshinter/pshinter.c", "mupdf/thirdparty/freetype/src/psnames/psnames.c", "mupdf/thirdparty/freetype/src/raster/raster.c", "mupdf/thirdparty/freetype/src/sfnt/sfnt.c", "mupdf/thirdparty/freetype/src/smooth/smooth.c", "mupdf/thirdparty/freetype/src/truetype/truetype.c", "mupdf/thirdparty/freetype/src/type1/type1.c"
)

$JBIG2DEC_SRC = @(
    "mupdf/thirdparty/jbig2dec/jbig2.c", "mupdf/thirdparty/jbig2dec/jbig2_arith.c", "mupdf/thirdparty/jbig2dec/jbig2_arith_iaid.c", "mupdf/thirdparty/jbig2dec/jbig2_arith_int.c", "mupdf/thirdparty/jbig2dec/jbig2_generic.c", "mupdf/thirdparty/jbig2dec/jbig2_halftone.c", "mupdf/thirdparty/jbig2dec/jbig2_huffman.c", "mupdf/thirdparty/jbig2dec/jbig2_hufftab.c", "mupdf/thirdparty/jbig2dec/jbig2_image.c", "mupdf/thirdparty/jbig2dec/jbig2_mmr.c", "mupdf/thirdparty/jbig2dec/jbig2_page.c", "mupdf/thirdparty/jbig2dec/jbig2_refinement.c", "mupdf/thirdparty/jbig2dec/jbig2_segment.c", "mupdf/thirdparty/jbig2dec/jbig2_symbol_dict.c", "mupdf/thirdparty/jbig2dec/jbig2_text.c"
)

$OPENJPEG_SRC = @(
    "mupdf/thirdparty/openjpeg/src/lib/openjp2/bio.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/cio.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/dwt.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/event.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/function_list.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/ht_dec.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/image.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/invert.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/j2k.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/jp2.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/mct.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/mqc.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/openjpeg.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/pi.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/sparse_array.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/t1.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/t2.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/tcd.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/tgt.c", "mupdf/thirdparty/openjpeg/src/lib/openjp2/thread.c"
)

$MUPDF_FITZ = Get-ChildItem -Path "mupdf/source/fitz/*.c" | Select-Object -ExpandProperty FullName
$MUPDF_PDF = Get-ChildItem -Path "mupdf/source/pdf/*.c" | Select-Object -ExpandProperty FullName

$ALL_SRC = $ZLIB_SRC + $LIBJPEG_SRC + $FREETYPE_SRC + $JBIG2DEC_SRC + $OPENJPEG_SRC + $MUPDF_FITZ + $MUPDF_PDF

$OBJ_FILES = @()

foreach ($src in $ALL_SRC) {
    # Convert absolute paths to relative paths if necessary
    if ($src -match "^([a-zA-Z]:)") {
        $src = Resolve-Path -Relative $src
    }

    # Filter out files we don't want to build for this target
    if ($src -match "harfbuzz") { continue }
    if ($src -match "\\xml\.c$") { continue }
    if ($src -match "output-docx") { continue }
    if ($src -match "output-odt") { continue }

    # Create a stable, unique object filename from the source path
    $stable_name = ($src -replace '[/\\]', '_' -replace '\.c$', '.o')
    $obj_path = Join-Path $OUT_DIR $stable_name
    $OBJ_FILES += $obj_path

    if (Test-Path $obj_path) {
        Write-Host "Skipping $src (already exists)"
        continue
    }

    # Compute specfic compilation flags based on source folder
    $EXTRA_FLAGS = @()
    if ($src -match "zlib") { $EXTRA_FLAGS += "-Imupdf/thirdparty/zlib" }
    elseif ($src -match "libjpeg") { $EXTRA_FLAGS += "-Imupdf/thirdparty/libjpeg"; $EXTRA_FLAGS += "-Imupdf/scripts/libjpeg" }
    elseif ($src -match "freetype") { 
        $EXTRA_FLAGS += "-Imupdf/thirdparty/freetype/include"
        $EXTRA_FLAGS += "-Imupdf/scripts/freetype"
        "-DFT_CONFIG_MODULES_H=<slimftmodules.h>`n-DFT_CONFIG_OPTIONS_H=<slimftoptions.h>`n-DFT2_BUILD_LIBRARY" | Out-File "ft_flags.rsp" -Encoding ASCII
        $EXTRA_FLAGS += "@ft_flags.rsp"
    }
    elseif ($src -match "jbig2dec") { $EXTRA_FLAGS += "-Imupdf/thirdparty/jbig2dec"; $EXTRA_FLAGS += "-Imupdf/include" }
    elseif ($src -match "openjp2") { $EXTRA_FLAGS += "-Imupdf/thirdparty/openjpeg/src/lib/openjp2"; $EXTRA_FLAGS += "-DOPJ_STATIC"; $EXTRA_FLAGS += "-DOPJ_HAVE_INTTYPES_H"; $EXTRA_FLAGS += "-DOPJ_HAVE_STDINT_H" }
    else { 
        $EXTRA_FLAGS += "-Imupdf/thirdparty/freetype/include"
        $EXTRA_FLAGS += "-Imupdf/thirdparty/zlib"
        $EXTRA_FLAGS += "-Imupdf/thirdparty/libjpeg"
        $EXTRA_FLAGS += "-Imupdf/thirdparty/openjpeg/src/lib/openjp2"
        $EXTRA_FLAGS += "-Imupdf/thirdparty/jbig2dec"
        $EXTRA_FLAGS += "-DFZ_ENABLE_ICC=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_JS=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_SVG=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_EPUB=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_HTML=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_CBZ=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_XPS=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_DOCX_OUTPUT=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_ODT_OUTPUT=0"
        $EXTRA_FLAGS += "-DFZ_ENABLE_OCR_OUTPUT=0"
        $EXTRA_FLAGS += "-DNOCJK"
        $EXTRA_FLAGS += "-DOPJ_STATIC"
        $EXTRA_FLAGS += "-DOPJ_HAVE_INTTYPES_H"
        $EXTRA_FLAGS += "-DOPJ_HAVE_STDINT_H"
    } # fitz & pdf

    $ALL_ARGS = @("-ffreestanding", "-mno-red-zone", "-mno-mmx", "-O2", "-Wall", "-mcmodel=large", "-Wno-unused-function", "-Wno-unused-variable", "-Wno-unused-parameter", "-Wno-int-conversion", "-Wno-incompatible-pointer-types", "-Iinclude", "-Isrc/kernel", "-Isrc/kernel/hal", "-Imupdf/include")
    $ALL_ARGS += $EXTRA_FLAGS
    $ALL_ARGS += "-c", $src, "-o", $obj_path

    Write-Host "Compiling $src..."
    
    $process = Start-Process -FilePath $CC -ArgumentList $ALL_ARGS -Wait -NoNewWindow -PassThru
    if ($process.ExitCode -ne 0) {
        Write-Host "Failed to compile $src"
        exit 1
    }
}

Write-Host "Archiving objects into mupdf.a..."
$archive = "build\mupdf.a"
if (Test-Path $archive) { Remove-Item $archive }

# Archive using the & operator - splat array so each path is a separate argument
$full_paths = $OBJ_FILES | ForEach-Object { (Resolve-Path $_).Path }
& $AR rcs $archive @full_paths
if ($LASTEXITCODE -ne 0) {
    Write-Host "Archive failed!"
    exit 1
}

Write-Host "MuPDF compiled successfully! Archive: $archive"
