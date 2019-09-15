$NDKPath = Get-Content $PSScriptRoot/ndkpath.txt

& "$NDKPath/build/ndk-build.cmd" NDK_PROJECT_PATH=$PSScriptRoot APP_BUILD_SCRIPT=$PSScriptRoot/Android.mk NDK_APPLICATION_MK=$PSScriptRoot/Application.mk