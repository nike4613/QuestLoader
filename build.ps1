$ridylPath = "$PSScriptRoot/RiDyL"
$libmainPath = "$PSScriptRoot/libmain"

if (-not (Test-Path "$libmainPath/ndkpath.txt" -PathType Leaf)) {
    Copy-Item "$PSScriptRoot/ndkpath.txt" "$libmainPath/ndkpath.txt"
}
if (-not (Test-Path "$ridylPath/ndkpath.txt" -PathType Leaf)) {
    Copy-Item "$PSScriptRoot/ndkpath.txt" "$ridylPath/ndkpath.txt"
}

."$libmainPath/build.ps1"
."$ridylPath/build.ps1"