."$PSScriptRoot/build.ps1"

$apkPath = Get-Content "$PSScriptRoot/apkpath.txt"
$apkDir = [System.IO.Path]::GetDirectoryName($apkPath)

$bindir = "$PSScriptRoot/libmodloader/libs/arm64-v8a/"

$fsFiles = [System.Collections.ArrayList]@()
$targetFiles = [System.Collections.ArrayList]@()

foreach ($item in Get-ChildItem $bindir) {
    $ignore = $fsFiles.Add($item.FullName)
    $basename = $item.Name
    $ignore = $targetFiles.Add("lib/arm64-v8a/$basename")
}

."$PSScriptRoot/tools/apkmod.ps1" -ApkPath $apkPath -ApkOutPath "$apkDir/patched.apk" `
    -AddFile $fsFiles -ArcTarget $targetFiles