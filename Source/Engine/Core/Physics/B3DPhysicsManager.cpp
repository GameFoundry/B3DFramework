//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/B3DPhysicsManager.h"
#include "Utility/B3DDynamicLibraryManager.h"
#include "Utility/B3DDynamicLibrary.h"

using namespace b3d;

PhysicsManager::PhysicsManager(const String& pluginName, bool cooking)
	: mPlugin(nullptr), mFactory(nullptr)
{
	mPlugin = DynamicLibraryManager::Instance().Load(pluginName);

	if(mPlugin != nullptr)
	{
		typedef PhysicsFactory* (*LoadPluginFunc)();

		LoadPluginFunc loadPluginFunction = (LoadPluginFunc)mPlugin->GetSymbol("LoadPlugin");
		mFactory = loadPluginFunction();

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

			UnloadPluginFunc unloadPluginFunction = (UnloadPluginFunc)mPlugin->GetSymbol("UnloadPlugin");

			mFactory->ShutDown();
			unloadPluginFunction(mFactory);
		}

		DynamicLibraryManager::Instance().Unload(mPlugin);
	}
}
