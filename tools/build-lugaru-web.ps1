# Build script for Lugaru WebGL port.
# Helper tools (pkg-config.bat, gl_stub.js) live in source/cmake/emscripten/.
# Helper scripts (.py, .bat, etc.) live in tools/scripts/.

$ErrorActionPreference = "Stop"

try {
    $root  = (Get-Item $PSScriptRoot).Parent.FullName
    $src   = "$root\source"
    $build = "$root\docs"
    $tools = "$root\tools"

    # Verify dependencies present (checked into repo, not downloaded)
    if (-not (Test-Path "$src\Dependencies\jsoncpp\include\json\value.h")) {
        throw "jsoncpp missing from source/Dependencies/jsoncpp/"
    }
    if (-not (Test-Path "$src\CMakeLists.txt")) {
        throw "CMakeLists.txt not found"
    }

    # Emscripten environment
    Set-Location "$root\emsdk"
    . .\emsdk_env.ps1 | Out-Null

    # Configure
    Set-Location $build
    New-Item -ItemType Directory -Force -Path $build | Out-Null
    Remove-Item * -Recurse -Force -Exclude icons -ErrorAction SilentlyContinue
    Copy-Item "$src\index.html" "index.html" -Force

    emcmake cmake $src -G "Ninja" -DCMAKE_BUILD_TYPE=Release `
      -DPKG_CONFIG_EXECUTABLE="$src\cmake\emscripten\pkg-config.bat" `
      -DCMAKE_CXX_FLAGS="-s USE_SDL=2 -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_LIBJPEG=1 -s USE_VORBIS=1 -s USE_OGG=1" `
      -DCMAKE_EXE_LINKER_FLAGS="-s LEGACY_GL_EMULATION=1 -s ALLOW_MEMORY_GROWTH=1 -s ASYNCIFY=1 -lIDBFS.js -lopenal --js-library $src/cmake/emscripten/gl_stub.js --preload-file ../source/Data@/Data -lpng" `
      -DPNG_LIBRARY=-lpng -DPNG_INCLUDE_DIR="" `
      -DJPEG_LIBRARY=-ljpeg -DJPEG_INCLUDE_DIR="$env:EMSDK\upstream\emscripten\cache\sysroot\include" `
      -DCMAKE_EXECUTABLE_SUFFIX=".html"
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

    # Build
    ninja
    if ($LASTEXITCODE -ne 0) { throw "Build failed" }

    Write-Host "Build complete. Output in docs/" -ForegroundColor Green
}
catch {
    Write-Host "[ERROR CAUGHT]: $_" -ForegroundColor Red
}

Write-Host "Press Enter to exit..." -ForegroundColor Cyan
[void][Console]::ReadLine()
