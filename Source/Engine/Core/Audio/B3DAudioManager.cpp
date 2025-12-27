//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/B3DAudioManager.h"
#include "Utility/B3DDynamicLibraryManager.h"
#include "Utility/B3DDynamicLibrary.h"

using namespace b3d;

AudioManager::AudioManager(const String& pluginName)
{
	mPlugin = DynamicLibraryManager::Instance().Load(pluginName);

	if(mPlugin != nullptr)
	{
		typedef AudioFactory* (*LoadPluginFunc)();

		LoadPluginFunc loadPluginFunc = (LoadPluginFunc)mPlugin->GetSymbol("LoadPlugin");
		mFactory = loadPluginFunc();

		if(mFactory != nullptr)
			mFactory->StartUp();
	}
}

AudioManager::~AudioManager()
{
	if(mPlugin != nullptr)
	{
		if(mFactory != nullptr)
		{
			typedef void (*UnloadPluginFunc)(AudioFactory*);

			UnloadPluginFunc unloadPluginFunc = (UnloadPluginFunc)mPlugin->GetSymbol("UnloadPlugin");

			mFactory->ShutDown();
			unloadPluginFunc(mFactory);
		}

		DynamicLibraryManager::Instance().Unload(mPlugin);
	}
}
