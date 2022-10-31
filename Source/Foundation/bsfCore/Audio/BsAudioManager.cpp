//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioManager.h"
#include "Utility/BsDynamicLibraryManager.h"
#include "Utility/BsDynamicLibrary.h"

using namespace bs;

AudioManager::AudioManager(const String& pluginName)
{
	mPlugin = DynamicLibraryManager::Instance().Load(pluginName);

	if(mPlugin != nullptr)
	{
		typedef AudioFactory* (*LoadPluginFunc)();

		LoadPluginFunc loadPluginFunc = (LoadPluginFunc)mPlugin->GetSymbol("loadPlugin");
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

			UnloadPluginFunc unloadPluginFunc = (UnloadPluginFunc)mPlugin->GetSymbol("unloadPlugin");

			mFactory->ShutDown();
			unloadPluginFunc(mFactory);
		}

		DynamicLibraryManager::Instance().Unload(mPlugin);
	}
}
