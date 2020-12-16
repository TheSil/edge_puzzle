@echo off
for %%i in ("%~dp0*.svg") do (
    echo %%i to %%~ni.png
    "C:\Program Files\Inkscape\bin\inkscape.exe" "%%i" -w 256 -h 256 --export-filename="%%~ni.png" --export-area=0:0:120:120
)