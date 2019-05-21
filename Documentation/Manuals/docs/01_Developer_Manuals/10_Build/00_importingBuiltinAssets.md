---
title: Importing built-in assets
---

The framework comes with a variety of assets that are considered built-in. This includes the default GUI skin (including all related fonts and textures), shaders used by the renderer, built-in mesh primitives (box, cube) and others. 

These assets are located in the `bsf/Data` folder, while their raw (un-imported) versions are located in the `bsf/Data/Raw` folder.

If you even need to modify these assets (e.g. change the source code for a built-in shader), you would modify the asset in `bsf/Data/Raw/` (e.g. shader code in `bsf/Data/Raw/Shaders`) and then run the import tool to compile that raw asset into a form usable by the engine. The import tool  basically calls @bs::Importer::import same as if you were importing assets manually in your application.

# Compiling & running the import tool
The import tool executable is not provided out of the box, and must be manually compiled. This can be done by compiling the `bsfImportTool` target included in `bsf`. Once compiled the tool should be installed to `bsf/Dependencies/tools/bsfImportTool/`.

Once the import tool dependency is present, you can enable asset import target in `bsf` by setting `-DINCLUDE_ASSET_PACKAGING_SCRIPTS=ON` in CMake command line. CMake will check for existance of the import tool and report an error if it cannot find it.

If the import tool has been found, a new build target `RunAssetImport_bsfBuiltinAssets` will be provided. Once you run the target the build tool will automatically detect any changes to the raw assets and re-import them as needed.

Optionally you can also run the tool manually as `bsfImportTool INPUT_FOLDER OUTPUT_FOLDER`, e.g. `bsfImportTool bsf/Data/Raw bsf/Data`.

# Forcing reimport
Normally the tool will only trigger import on raw assets that it detects have been modified (based on file modify dates). You can force reimport of all assets by deleting the `bsf/Data/Timestamp.asset` file and then running the import tool.

> You might also be forced to do a full reimport if you have added brand new raw assets, as the tool might not detect them otherwise.