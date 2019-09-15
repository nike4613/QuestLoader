param (
    [Parameter(Mandatory=$True)][string]$apkPath,
    [string]$apkOutPath = $apkPath,
    [Parameter(Mandatory=$True)][string]$addFile,
    [Parameter(Mandatory=$True)][string]$arcTarget,
    [switch]$noOverwrite = $false
)

$overwrite = (-not $noOverwrite)

if ($apkPath -ne $apkOutPath) {
    Copy-Item $apkPath $apkOutPath
    $apkPath = $apkOutPath
}

$apkifierRoot = "$PSScriptRoot/Apkifier"
$apkifierDir = "$apkifierRoot/bin/Release/net462"
$apkifierPath = "$apkifierDir/Apkifier.dll"

if (-not (Test-Path $apkifierPath -PathType Leaf)) {
    $cwd = Get-Location
    Set-Location $apkifierRoot
    dotnet build -c Release
    Set-Location $cwd
}

Add-Type -Path $apkifierPath

$apk = New-Object "Emulamer.Utils.Apkifier" -ArgumentList ($apkPath)

$apk.Write($addFile, $arcTarget, $overwrite, $true)

$apk.Dispose()