$NDKPath = Get-Content ./ndkpath.txt

& "$NDKPath/build/ndk-build.cmd" NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk