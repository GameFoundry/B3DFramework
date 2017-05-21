# Compiling dependencies manually
To compile/retrieve dependencies manually follow the guides provided by each individual library. Below you will find a list of all the libraries, links to their download and outputs expected by Banshee.

Rename and place the compiled/retrieved outputs into proper folders depending on their type:
- Static libraries (.lib, .a): 
  - Place outputs in (BansheeSource)/Dependencies/(LibName)/lib/(Platform)
- Dynamic libraries (.dll, .so, .dylib)
  - Place outputs in (BansheeSource)/bin/(Platform)/(Configuration)
- Includes
  - Place outputs in (BansheeSource)/Dependencies/(LibName)/include
- Tools (executables)
  - Place outputs in (BansheeSource)/Dependencies/tools/(LibName)  
   
Each library below lists a set of outputs required by Banshee. After you compile/retrieve the library rename the outputs to what Banshee expects and place them relative to the above paths, depending on their type. Depending on which platform you're compiling for you will need to compile 32-bit or 64-bit versions of the libraries (or both).
   
Legend:
- (BansheeSource) - root directory of Banshee's source code
- (LibName) - name of the library (title of each library shown below)
- (Platform) - x86 for 32-bit builds, x64 for 64-bit builds
- (Configuration) - Debug, OptimizedDebug or Release
   
If the library structure still isn't clear, download one of the pre-compiled dependency packages to see an example.   
      
## Library list 
	  
**snappy**
- Google's Snappy compressor/decompressor
- https://github.com/BearishSun/snappy
- Required by BansheeUtility
- Outputs:
  - Windows (Static library):
    - /Debug/Snappy.lib (Debug configuration)
    - /Release/Snappy.lib (Release configuration)
	  
**nvtt**
- NVIDIA Texture Tools 2.1.0 (commit e85d851cd9502d77c6e20769d8c21baa3a94ac18)
- https://github.com/castano/nvidia-texture-tools
- Compilation notes:
  - Compile using the "no-cuda" configurations
- Required by BansheeCore
- Outputs: 
  - Windows (Dynamic library):
    - /Debug/nvtt.lib (Debug configuration)
    - /Release/nvtt.lib (Release configuration)
    - nvtt.dll (All configurations)
 
**FBXSDK**
- FBX SDK 2016.1
- http://usa.autodesk.com/fbx
- Compilation notes:
  - No compilation required, libraries are provided pre-compiled
- Required by BansheeFBXImporter
- Outputs:  
  - Windows (Static library):
    - /Debug/libfbxsdk-md.lib (Debug configuration)
    - /Release/libfbxsdk-md.lib (Release configuration)
 
**freetype**
- Freetype 2.3.5
- http://www.freetype.org
- Compilation notes:
  - Make sure to define FREETYPE2_STATIC as it might not be defined by default
- Required by BansheeFontImporter
- Outputs:
  - Windows (Static library):
    - /Debug/freetype.lib (Debug configuration)
    - /Release/freetype.lib (Release configuration)
   
**freeimg**
- FreeImage 3.13.1
- http://freeimage.sourceforge.net
- Required by BansheeFreeImgImporter
- Outputs:
  - Windows (Static library):
    - /Debug/FreeImage.lib (Debug configuration)
    - /Release/FreeImage.lib (Release configuration)
   
**OIS**
- BansheeOIS - Slightly modified version of OIS 1.3
- Source code included in the dependencies package
- Required by BansheeOISInput
- Outputs:
  - Windows (Dynamic library):
    - /Debug/BansheeOIS.lib (Debug configuration)
    - /Release/BansheeOIS.lib (Release configuration)
    - BansheeOIS.dll (All configurations)
   
**mono**
- Mono 4.4.0
- http://www.mono-project.com/
- Compilation notes:
  - See Mono-4.4.0-IntegrationGuide.txt
- Required by BansheeMono
- Outputs:
  - Windows (Dynamic library):
    - /Debug/mono-2.0.lib (Debug configuration)
    - /Release/mono-2.0.lib (Release configuration)
    - mono-2.0.dll (All configurations)
   
**PhysX**
- PhysX 3.3
- https://github.com/NVIDIAGameWorks/PhysX-3.3
- Required by BansheePhysX
- Outputs:
  - Windows (Dynamic library):
    - x64
      - /Debug/PhysX3CharacterKinematicCHECKED_x64.lib (Compile using "checked" configuration)
      - /Debug/PhysX3CHECKED_x64.lib (Compile using "checked" configuration)
      - /Debug/PhysX3CommonCHECKED_x64.lib (Compile using "checked" configuration)
      - /Debug/PhysX3CookingCHECKED_x64.lib (Compile using "checked" configuration)
      - /Debug/PhysX3ExtensionsCHECKED.lib (Compile using "checked" configuration)
      - /Release/PhysX3CharacterKinematic_x64.lib (Compile using "release" configuration)
      - /Release/PhysX3_x64.lib (Compile using "release" configuration)
      - /Release/PhysX3Common_x64.lib (Compile using "release" configuration)
      - /Release/PhysX3Cooking_x64.lib (Compile using "release" configuration)
      - /Release/PhysX3Extensions.lib (Compile using "release" configuration)
      - PhysX3CharacterKinematicCHECKED_x64.dll (For Debug configuration)
      - PhysX3CHECKED_x64.dll (For Debug configuration)
      - PhysX3CommonCHECKED_x64.dll (For Debug configuration)
      - PhysX3CookingCHECKED_x64.dll (For Debug configuration)
      - PhysX3CharacterKinematic_x64.dll (For Release/OptimizedDebug configuration)
      - PhysX3_x64.dll (For Release/OptimizedDebug configuration)
      - PhysX3Common_x64.dll (For Release/OptimizedDebug configuration)
      - PhysX3Cooking_x64.dll (For Release/OptimizedDebug configuration)
    - x86
      - /Debug/PhysX3CharacterKinematicCHECKED_x86.lib (Compile using "checked" configuration)
      - /Debug/PhysX3CHECKED_x86.lib (Compile using "checked" configuration)
      - /Debug/PhysX3CommonCHECKED_x86.lib (Compile using "checked" configuration)
      - /Debug/PhysX3CookingCHECKED_x86.lib (Compile using "checked" configuration)
      - /Debug/PhysX3ExtensionsCHECKED.lib (Compile using "checked" configuration)
      - /Release/PhysX3CharacterKinematic_x86.lib (Compile using "release" configuration)
      - /Release/PhysX3_x86.lib (Compile using "release" configuration)
      - /Release/PhysX3Common_x86.lib (Compile using "release" configuration)
      - /Release/PhysX3Cooking_x86.lib (Compile using "release" configuration)
      - /Release/PhysX3Extensions.lib (Compile using "release" configuration)
      - PhysX3CharacterKinematicCHECKED_x86.dll (For Debug configuration)
      - PhysX3CHECKED_x86.dll (For Debug configuration)
      - PhysX3CommonCHECKED_x86.dll (For Debug configuration)
      - PhysX3CookingCHECKED_x86.dll (For Debug configuration)
      - PhysX3CharacterKinematic_x86.dll (For Release/OptimizedDebug configuration)
      - PhysX3_x86.dll (For Release/OptimizedDebug configuration)
      - PhysX3Common_x86.dll (For Release/OptimizedDebug configuration)
      - PhysX3Cooking_x86.dll (For Release/OptimizedDebug configuration)	
	
**OpenAL**
- OpenAL Soft 1.17.2
- https://github.com/kcat/openal-soft
- Required by BansheeOpenAudio
- Outputs:
  - Windows (Dynamic library):
    - OpenAL32.lib (Compile using "release" configuration)
    - OpenAL32.dll (All configurations)
   
**libogg**
- libogg v1.3.2
- https://xiph.org/downloads/
- Compilation notes:
  - Switch runtime library to dynamic to avoid linker warnings when adding it to Banshee
  - This is also required when compiling libvorbis and libflac (below). See readme files included with those libraries.
- Required by BansheeOpenAudio and BansheeFMOD
- Outputs:
  - Windows (Static library):
    - Debug/libogg.lib (Debug configuration)
    - Release/libogg.lib (Release configuration)
   
**libvorbis**
- libvorbis commit:5bbe49dc947ea9f1532273644348c53054ca214a
- https://git.xiph.org/?p=vorbis.git
- Compilation notes:
  - Requires libogg, as described in its readme file.
- Required by BansheeOpenAudio and BansheeFMOD
- Outputs:
  - Windows (Dynamic library):
    - libvorbis.lib (Compile using "release" configuration)
    - libvorbisfile.lib (Compile using "release" configuration)
    - libvorbis.dll (All configurations)
    - libvorbisfile.dll (All configurations)
   
**libFLAC**
- libflac commit: 94a61241b02064c7d9fe508f72a742f2a90b8492
- https://git.xiph.org/?p=flac.git
- Compilation notes:
  - Requires libogg, as described in its readme file.
- Required by BansheeOpenAudio
- Outputs:
  - Windows (Dynamic library):
    - libFLAC.lib (Compile using "release" configuration)
    - libFLAC_dynamic.dll (All configurations)
   
**glslang**
- glslang commit: 19bdf90eba71390f04bb85226337517df65d73e2
- https://github.com/KhronosGroup/glslang
- Required by BansheeVulkanRenderAPI
- Outputs:
  - Windows (Static library):
    - glslang.lib (Compile using "release" configuration)
    - glslangd.lib (Compile using "debug" configuration)
    - HLSL.lib (Compile using "release" configuration)
    - HLSLd.lib (Compile using "debug" configuration)
    - OGLCompiler.lib (Compile using "release" configuration)
    - OGLCompilerd.lib (Compile using "debug" configuration)
    - OSDependent.lib (Compile using "release" configuration)
    - OSDependentd.lib (Compile using "debug" configuration)
   
**XShaderCompiler**
- Distributed with Banshee source in Source/External/XShaderCompiler
- Required by BansheeSL
   
**bison**
- Bison 2.7
- http://sourceforge.net/projects/winflexbison/files/
- Required by BansheeSL
- Outputs:
  - Windows (tool):
    - bison/bison.exe (Including all other installation files)
   
**flex**
- Flex 2.5.37
- http://sourceforge.net/projects/winflexbison/files/
- Required by BansheeSL
- Outputs:
  - Windows (tool):
    - flex/flex.exe (Including all other installation files)
	
**BansheeSBGen**
 - Banshee Script Binding Generator 1.0
 - https://github.com/BearishSun/BansheeSBGen
 - Required for generation of C# script binding files. Not required if not using the scripting sub-system.
 - Outputs:
   - Windows (tool):
     - BansheeSBGen/BansheeSBGen_v1.0.0.exe
