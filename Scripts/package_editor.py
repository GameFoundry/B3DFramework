#!/usr/bin/python

# Packages the editor by copying all relevant files to the Builds directory.
# This will not execute the editor build, and is implied the editor has been
# been built before executing this script.

# Usage: "package_editor $Configuration"
# Where: $Configuration - e.g. Debug, OptimizedDebug

import os
import sys
import shutil

configuration = 'OptimizedDebug' #sys.argv[1]
dataEntriesToIgnore = ['Examples', 'Raw', 'Settings.asset']
binEntriesToIgnore = ['Game.exe', 'Game.pdb', 'ExampleProject.exe', 'ExampleProject.pdb',
                      'BansheeD3D9RenderAPI.dll', 'BansheeD3D9RenderAPI.pdb']

dataFolder = 'Data'
assembliesFolder = 'Assemblies'
monoFolder = 'Mono'
libFolder = 'x64\\' + configuration + '\\'

inputDataFolder = '..\\' + dataFolder
inputBinFolder = '..\\bin\\'
inputAssembliesFolder = inputBinFolder + assembliesFolder
inputMonoFolder = inputBinFolder + monoFolder
inputLibFolder = inputBinFolder + libFolder

outputBaseFolder = '..\\Builds\Banshee Editor\\'
outputDataFolder = outputBaseFolder + dataFolder
outputBinFolder = outputBaseFolder + '\\bin\\'
outputAssembliesFolder = outputBinFolder + assembliesFolder
outputMonoFolder = outputBinFolder + monoFolder
outputLibFolder = outputBaseFolder

def ignore_data(path, entries):
    if path != inputDataFolder:
        return []

    return list(set(dataEntriesToIgnore) & set(entries))    

def package_editor():
    if os.path.exists(outputBaseFolder):
        shutil.rmtree(outputBaseFolder)

    os.makedirs(outputBaseFolder)
    shutil.copytree(inputDataFolder, outputDataFolder, False, ignore_data)
    shutil.copytree(inputAssembliesFolder, outputAssembliesFolder)
    shutil.copytree(inputMonoFolder, outputMonoFolder)

    for root, dirs, files in os.walk(inputLibFolder):
        for file in files:
            if(file.lower().endswith(('.dll', '.exe', '.pdb', '.so'))):
                if(not file in binEntriesToIgnore):
                    filePath = os.path.join(root, file)
                    shutil.copy(filePath, outputLibFolder)

package_editor()
