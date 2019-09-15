."$PSScriptRoot/build.ps1"

$apkPath = Get-Content "$PSScriptRoot/apkpath.txt"
$apkDir = [System.IO.Path]::GetDirectoryName($apkPath)

."$PSScriptRoot/tools/apkmod.ps1" -ApkPath $apkPath -ApkOutPath "$apkDir/patched.apk" -AddFile "$PSScriptRoot/libmain/libs/arm64-v8a/libmain.so" -ArcTarget "lib/arm64-v8a/libmain.so"