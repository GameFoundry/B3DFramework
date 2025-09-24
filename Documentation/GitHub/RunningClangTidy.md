**Running clang tidy over the entire codebase**
 
Using clang tidy on individual files or even entire solution through Clang Power Tools, usually doesn't yield good results for renames, as a single translation unit is refactored, and then following translation units using those same headers will end up with compile errors due to naming changes. You must use a compilation database, together with run-clang-tidy Python script do this this properly.

To do this:
  1. Generate a compile database by setting `CMAKE_EXPORT_COMPILE_COMMANDS=ON` when creating a build in CMake. Note this will only work with Ninja or Makefile generators. On Windows, it's easiest to pass the argument in CLion using its built-in Ninja/MinGW toolchain. The compile database will be present in your build folder.
  2. Run 'run-clang-tidy' Python script from LLVM/bin folder:  
    - `python run-clang-tidy -p F:\BansheeEngine\cmake-build-debug -header-filter 'B3D.*' -config="{Checks: '-*,readability-identifier-naming', CheckOptions: [{ key: readability-identifier-naming.PublicMemberCase, value: CamelCase }]}" -fix`  
    - `-p` determines the location of the compile database. `B3D.*` ensures only headers with the `B3D` prefix are parsed, and `-config` specifies the checks that need to be used. I've found that `run-clang-tidy` does not properly respect the .clang-tidy file, so I specify the checks explicity using the `-config` option.