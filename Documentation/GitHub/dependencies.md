# Compiling dependencies manually

Below you will find a list of dependencies that Banshee relies on, as well as links to their source code and/or binaries. If a dependency isn't provided in binary form you will have to manually compile it (this is the case for the large majority or them). Make sure to compile the exact version of the dependency listed below. Newer versions *might* work, but haven't been tested. 

Once you have the dependency development files (headers and binaries) you will need to make sure they placed in the appropriate folders so Banshee can find them during the build process. 

The dependencies are searched for in these locations:
- The `/Dependency` folder within Banshee's source. See below for the exact breakdown of how this folder is supposed to look. Usually you want to put all your dependencies here.
- If dependency cannot be found in the `/Dependency` folder, its default install path is searched for instead. For example `usr/local` on Linux/macOS or default install path if the dependency comes with an installer. 

Note that on Windows most dependencies do not have default install paths and should therefore be placed in the `/Dependency` folder. In order to avoid problems with dependency versions this should be the preferred behaviour on Linux/macOS as well. 

`/Dependency` folder breakdown:
- Static & shared libraries (.lib, .a, .so): 
  - Pick one of:
    - (BansheeSource)/Dependencies/(DepName)/lib
	- (BansheeSource)/Dependencies/(DepName)/lib/(Platform)
	- (BansheeSource)/Dependencies/(DepName)/lib/(Platform)/(Configuration)
	- (BansheeSource)/Dependencies/(DepName)/lib/(Configuration)
- Dynamic libraries (.dll, .dylib)
  - Place in (BansheeSource)/bin/(Platform)/(Configuration)
- Includes
  - Place in (BansheeSource)/Dependencies/(DepName)/include
- Tools (executables)
  - Place in (BansheeSource)/Dependencies/tools/(DepName)  
  
Legend:
- (BansheeSource) - root directory of Banshee
- (DepName) - name of the dependency (title of each dependency shown below)
- (Platform) - x86 for 32-bit builds, x64 for 64-bit builds
- (Configuration) - Debug, OptimizedDebug or Release  
  
Each library is accompanied by a Find***.cmake CMake module that is responsible for finding the library. These modules are located under `Source/CMake/Modules`. They follow the rules described above, but if you are unsure where library outputs should be placed you can look at the source code for those modules to find their search paths.
   
Additionally, if the dependency structure still isn't clear, download one of the pre-compiled dependency packages to see an example.  
      
## Library list 
	  
**snappy**
- Google's Snappy compressor/decompressor
- https://github.com/BearishSun/snappy
- Required by BansheeUtility
- Compile as a static library
	  
**nvtt**
- NVIDIA Texture Tools 2.1.0
- https://github.com/BearishSun/nvidia-texture-tools
- Required by BansheeCore
- Compile as a static library
 
**FBXSDK**
- FBX SDK 2016.1
- http://usa.autodesk.com/fbx
- Required by BansheeFBXImporter
- No compilation required, libraries are provided pre-compiled
 
**freetype**
- Freetype 2.3.5
- https://github.com/BearishSun/freetype (branch *banshee*)
- Required by BansheeFontImporter
- Compile as a static library
   
**freeimg**
- FreeImage 3.13.1
- http://freeimage.sourceforge.net
- Required by BansheeFreeImgImporter
- Compile as a static library
   
**mono**
- Mono 4.4.0
- http://www.mono-project.com/
- Required by BansheeMono
- Compile as a dynamic library
 - See Mono-4.4.0-IntegrationGuide.txt for additional notes
   
**PhysX**
- PhysX 3.3
- https://github.com/NVIDIAGameWorks/PhysX-3.3
- Required by BansheePhysX
- Compile as a dynamic library
	
**OpenAL**
- OpenAL Soft 1.17.2
- https://github.com/kcat/openal-soft
- Required by BansheeOpenAudio
- **Linux only**
 - Make sure to get audio backend libraries before compiling: PulseAudio, OSS, ALSA and JACK
 - On Debian/Ubuntu run: *apt-get install libpulse libasound2-dev libjack-dev* 
- Compile as a dynamic library
   
**libogg**
- libogg v1.3.2
- https://xiph.org/downloads/
- Required by BansheeOpenAudio and BansheeFMOD
- Compile as a static library
  - Switch runtime library to dynamic to avoid linker warnings when adding it to Banshee
  - This is also required when compiling libvorbis and libflac (below). See readme files included with those libraries.
  
**libvorbis**
- libvorbis commit:8a8f8589e19c5016f6548d877a8fda231fce4f93
- https://git.xiph.org/?p=vorbis.git
- Required by BansheeOpenAudio and BansheeFMOD
- Compile as a dynamic library
  - Requires libogg, as described in its readme file.
   
**libFLAC**
- libflac commit: f7cd466c24fb5d1966943f3ea36a1f4a37858597
- https://git.xiph.org/?p=flac.git
- Required by BansheeOpenAudio
- Compile as a dynamic library
  - Requires libogg, as described in its readme file.
   
**glslang**
- glslang commit: 19bdf90eba71390f04bb85226337517df65d73e2
- https://github.com/KhronosGroup/glslang
- Required by BansheeVulkanRenderAPI
- Compile as a static library
   
**XShaderCompiler**
- https://github.com/BearishSun/XShaderCompiler (branch *banshee*)
- Required by BansheeSL
- Compile as a static library
   
**bison**
- Bison 3.0.4
- **Windows**
  - http://sourceforge.net/projects/winflexbison/files/
- **Linux/Mac**
  - http://ftp.gnu.org/gnu/bison/
- Required by BansheeSL
   
**flex**
- Flex 2.6.1
- **Windows**
  - http://sourceforge.net/projects/winflexbison/files/
- **Linux/Mac**
  - https://github.com/westes/flex/releases
- Required by BansheeSL
	
**BansheeSBGen**
 - Banshee Script Binding Generator 1.0
 - https://github.com/BearishSun/BansheeSBGen
 - Required for generation of C# script binding files. Not required if not using the scripting sub-system.
 - Executable (tool)
