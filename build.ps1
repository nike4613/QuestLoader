$ridylPath = "$PSScriptRoot/RiDyL"
$libmainPath = "$PSScriptRoot/libmain"
$libmodloaderPath = "$PSScriptRoot/libmodloader"

if (-not (Test-Path "$libmainPath/ndkpath.txt" -PathType Leaf)) {
    Copy-Item "$PSScriptRoot/ndkpath.txt" "$libmainPath/ndkpath.txt"
}
if (-not (Test-Path "$ridylPath/ndkpath.txt" -PathType Leaf)) {
    Copy-Item "$PSScriptRoot/ndkpath.txt" "$ridylPath/ndkpath.txt"
}
if (-not (Test-Path "$libmodloaderPath/ndkpath.txt" -PathType Leaf)) {
    Copy-Item "$PSScriptRoot/ndkpath.txt" "$libmodloaderPath/ndkpath.txt"
}


Set-Location $libmainPath
."$libmainPath/build.ps1"
Set-Location $PSScriptRoot

Set-Location $ridylPath
."$ridylPath/build.ps1"
Set-Location $PSScriptRoot

Set-Location $libmodloaderPath
."$libmodloaderPath/build.ps1"
Set-Location $PSScriptRoot