pushd %~dp0
del   /S /Q *.user
rmdir /S /Q Debug
rmdir /S /Q Release
del /Q /F /A:H *.suo
del /Q *.ncb
popd