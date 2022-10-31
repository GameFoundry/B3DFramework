//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/BsPhysicsManager.h"
#include "Utility/BsDynamicLibraryManager.h"
#include "Utility/BsDynamicLibrary.h"

using namespace bs;

PhysicsManager::PhysicsManager(const String& pluginName, bool cooking)
	: mPlugin(nullptr), mFactory(nullptr)
{
	mPlugin = DynamicLibraryManager::Instance().Load(pluginName);

	if(mPlugin != nullptr)
	{
		typedef PhysicsFactory* (*LoadPluginFunc)();

		LoadPluginFunc loadPluginFunc = (LoadPluginFunc)mPlugin->GetSymbol("loadPlugin");
		mFactory = loadPluginFunc();

		if(mFactory != nullptr)
			mFactory->StartUp(cooking);
	}
}

PhysicsManager::~PhysicsManager()
{
	if(mPlugin != nullptr)
	{
		if(mFactory != nullptr)
		{
			typedef void (*UnloadPluginFunc)(PhysicsFactory*);

			UnloadPluginFunc unloadPluginFunc = (UnloadPluginFunc)mPlugin->GetSymbol("unloadPlugin");

			mFactory->ShutDown();
			unloadPluginFunc(mFactory);
		}

		DynamicLibraryManager::Instance().Unload(mPlugin);
	}
}
