@echo off

cd /d  %~dp0/downloadTool/skin

set svg_path=%~dp0/downloadTool/skin/svgEx

for /f %sPath% %%i in ('dir /b %svg_path%') do (
    setlocal enabledelayedexpansion
    set file=%%i

    set filename=!file:.svg=!

    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d  96 svgEx\!filename!.svg -o images\!filename!.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d  96 svgEx\!filename!.svg -o images\!filename!.png
    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 120 svgEx\!filename!.svg -o images\!filename!@125.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 120 svgEx\!filename!.svg -o images\!filename!@125.png
    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 144 svgEx\!filename!.svg -o images\!filename!@150.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 144 svgEx\!filename!.svg -o images\!filename!@150.png
    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 168 svgEx\!filename!.svg -o images\!filename!@175.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 168 svgEx\!filename!.svg -o images\!filename!@175.png
    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 192 svgEx\!filename!.svg -o images\!filename!@200.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 192 svgEx\!filename!.svg -o images\!filename!@200.png
    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 216 svgEx\!filename!.svg -o images\!filename!@225.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 216 svgEx\!filename!.svg -o images\!filename!@225.png
    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 240 svgEx\!filename!.svg -o images\!filename!@250.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 240 svgEx\!filename!.svg -o images\!filename!@250.png
    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 264 svgEx\!filename!.svg -o images\!filename!@275.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 264 svgEx\!filename!.svg -o images\!filename!@275.png
    echo call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 288 svgEx\!filename!.svg -o images\!filename!@300.png
    call "C:\Program Files\Inkscape\bin\inkscape.exe" -d 288 svgEx\!filename!.svg -o images\!filename!@300.png

    del svgEx\%%i
)

cd /d  %~dp0
cd /d %~dp0/downloadTool/skin
..\..\third_party\7z\7z.exe a "..\skin.zip" "*" -r