IF EXIST "%MONO_INSTALL_DIR%\bin\pdb2mdb.bat" (
 IF %2 == "Debug" CALL "%MONO_INSTALL_DIR%\bin\pdb2mdb.bat" %1
)