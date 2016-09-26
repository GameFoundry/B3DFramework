//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAudioManager.h"
#include "BsDynLibManager.h"
#include "BsDynLib.h"

namespace BansheeEngine
{
	AudioManager::AudioManager(const String& pluginName)
		:mPlugin(nullptr), mFactory(nullptr)
	{
		mPlugin = DynLibManager::instance().load(pluginName);

		if(mPlugin != nullptr)
		{
			typedef AudioFactory* (*LoadPluginFunc)();

			LoadPluginFunc loadPluginFunc = (LoadPluginFunc)mPlugin->getSymbol("loadPlugin");
			mFactory = loadPluginFunc();

			if (mFactory != nullptr)
				mFactory->startUp();
		}
	}

	AudioManager::~AudioManager()
	{
		if (mPlugin != nullptr)
		{
			if (mFactory != nullptr)
			{
				typedef void (*UnloadPluginFunc)(AudioFactory*);

				UnloadPluginFunc unloadPluginFunc = (UnloadPluginFunc)mPlugin->getSymbol("unloadPlugin");

				mFactory->shutDown();
				unloadPluginFunc(mFactory);
			}

			DynLibManager::instance().unload(mPlugin);
		}
	}
}