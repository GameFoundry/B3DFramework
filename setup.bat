@ECHO OFF
echo.
echo.
echo --------------------------------------------------------------------------------
echo --------------------Downloading dependencies, please wait...--------------------
echo --------------------------------------------------------------------------------
IF EXIST Temp rmdir Temp /S /Q

mkdir Temp
cd Temp
"../Scripts/tools/curl/curl" http://bearishsun.thalassa.feralhosting.com/BansheeDependencies_VS2015_Master.zip -o Dependencies.zip

echo.
echo.
echo --------------------------------------------------------------------------------
echo -----------------------Download successful! Extracting...-----------------------
echo --------------------------------------------------------------------------------
"../Scripts/tools/7z/7za" x Dependencies.zip

IF EXIST "../bin" rmdir "../bin" /S /Q
move /Y Built/bin ../

IF EXIST "../Dependencies" rmdir "../Dependencies" /S /Q
move /Y Built/Dependencies ../

cd ..
rmdir Temp /S /Q

echo.
echo.
echo --------------------------------------------------------------------------------
echo -----------------Extraction done! Generating Visual Studio solution...----------
echo --------------------------------------------------------------------------------
IF NOT EXIST Build mkdir Build
cd Build
IF NOT EXIST VS2015 mkdir VS2015
cd VS2015

"../../Dependencies/tools/cmake/bin/cmake" -G "Visual Studio 14 2015 Win64" ../../Source
cd ../..

echo.
echo.
echo All done! Visual Studio solution provided at %cd%/Build/VS2015/Banshee.sln

pause