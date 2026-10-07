$compilerDir = "C:\Users\mio\Documents\projects\mod\arefulauncher\tools\w64devkit\bin"
$env:PATH = "$compilerDir;" + $env:PATH

$src = "src\main.cpp"
$out = "zestedlauncher.exe"

Write-Host "Compiling Zested Launcher C++..."
& "$compilerDir\g++.exe" -std=c++17 -O2 -mwindows $src -I sdk\include -L sdk\lib\x64 -lWebView2Loader -lwinhttp -lole32 -lshlwapi -lversion -loleaut32 -luser32 -lshell32 -lcomdlg32 -o $out

if ($LASTEXITCODE -eq 0) {
    Write-Host "Build Successful! Output: $out"
    Copy-Item "sdk\lib\x64\WebView2Loader.dll" -Destination "." -Force
    Copy-Item $out -Destination "arefulauncher.exe" -Force
    Write-Host "WebView2Loader.dll and binaries updated."
} else {
    Write-Error "Build Failed with exit code $LASTEXITCODE"
}
