@ECHO OFF
cls
echo Downloading dependencies, please wait...
IF EXIST Temp (
  echo !!!Error: Temp folder already exists at path %cd%\Temp. Please delete or rename it before continuing.
  exit /B
)

mkdir Temp
cd Temp
"../Scripts/tools/curl/curl" http://bearishsun.thalassa.feralhosting.com/BansheeDependencies_VS2015_Master.zip -o Dependencies.zip

cls
echo Download successful! Extracting...
"../Scripts/tools/7z/7za" x Dependencies.zip
move /Y Built/bin ../
move /Y Built/Dependencies ../

cd ..
rmdir Temp /S /Q

cls
echo Extraction done! Generating Visual Studio solution...
IF NOT EXIST Build mkdir Build
cd Build
IF NOT EXIST VS2015 mkdir VS2015
cd VS2015

"../../Dependencies/tools/cmake/bin/cmake" -G "Visual Studio 14 2015 Win64" ../../Source
cd ../..

cls
echo All done! Visual Studio solution provided at %cd%/Build/VS2015/Banshee.sln