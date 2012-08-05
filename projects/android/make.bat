@echo on

@set BASHPATH="E:\Programming\Cygwin\bin\bash"
@set PROJECTDIR="/cygdrive/C:/Users/paul/Documents/Eclipse/Workspace/NdkTest"
@set NDKDIR="/cygdrive/C:/Programming/android-ndk-r8/ndk-build"

%BASHPATH% --login -c "cd %PROJECTDIR% && %NDKDIR%"

@pause: