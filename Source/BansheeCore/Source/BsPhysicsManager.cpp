//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPhysicsManager.h"
#include "BsDynLibManager.h"
#include "BsDynLib.h"

namespace bs
{
	PhysicsManager::PhysicsManager(const String& pluginName, bool cooking)
		:mPlugin(nullptr), mFactory(nullptr)
	{
		mPlugin = DynLibManager::instance().load(pluginName);

		if(mPlugin != nullptr)
		{
			typedef PhysicsFactory* (*LoadPluginFunc)();

			LoadPluginFunc loadPluginFunc = (LoadPluginFunc)mPlugin->getSymbol("loadPlugin");
			mFactory = loadPluginFunc();

			if (mFactory != nullptr)
				mFactory->startUp(cooking);
		}
	}

	PhysicsManager::~PhysicsManager()
	{
		if (mPlugin != nullptr)
		{
			if (mFactory != nullptr)
			{
				typedef void (*UnloadPluginFunc)(PhysicsFactory*);

				UnloadPluginFunc unloadPluginFunc = (UnloadPluginFunc)mPlugin->getSymbol("unloadPlugin");

				mFactory->shutDown();
				unloadPluginFunc(mFactory);
			}

			DynLibManager::instance().unload(mPlugin);
		}
	}
}