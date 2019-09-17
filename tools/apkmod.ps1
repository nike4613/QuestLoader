param (
    [Parameter(Mandatory=$True)][string]$apkPath,
    [string]$apkOutPath = $apkPath,
    [Parameter(Mandatory=$True)][string[]]$addFile,
    [Parameter(Mandatory=$True)][string[]]$arcTarget,
    [switch]$noOverwrite = $false
)

function Select-Zip {
    [CmdletBinding()]
    Param(
        $First,
        $Second,
        $ResultSelector = { ,$args }
    )

    [System.Linq.Enumerable]::Zip($First, $Second, [Func[Object, Object, Object[]]]$ResultSelector)
}

$overwrite = (-not $noOverwrite)

if ($apkPath -ne $apkOutPath) {
    Copy-Item $apkPath $apkOutPath
    $apkPath = $apkOutPath
}

$apkifierRoot = "$PSScriptRoot/Apkifier"
$apkifierDir = "$apkifierRoot/bin/Release/net462"
$apkifierPath = "$apkifierDir/Apkifier.dll"

if (-not (Test-Path $apkifierPath -PathType Leaf)) {
    Write-Host "Compiling apkifier"

    $cwd = Get-Location
    Set-Location $apkifierRoot
    dotnet build -c Release
    Set-Location $cwd
}

if ($addFile.Count -ne $arcTarget.Count) {
    Write-Host "Length of AddFile must equal length of ArcTarget"
    Exit
}

Add-Type -Path $apkifierPath

$apk = New-Object "Emulamer.Utils.Apkifier" -ArgumentList ($apkPath)

foreach ($item in Select-Zip -First $addFile -Second $arcTarget) {
    Write-Host "Adding $($item[0]) as $($item[1])"
    $apk.Write($item[0], $item[1], $overwrite, $true)
}

Write-Host "Packing and signing"

$apk.Dispose()