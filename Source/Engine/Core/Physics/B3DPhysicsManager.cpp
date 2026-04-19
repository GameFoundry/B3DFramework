//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Physics/B3DPhysicsManager.h"
#include "Plugin/B3DPluginLoader.h"

using namespace b3d;

PhysicsManager::PhysicsManager(const String& pluginName, bool cooking)
{
	mPlugin = PluginLoader::Load(pluginName);
	mFactory = static_cast<PhysicsFactory*>(mPlugin.ReturnValue);

	if(mFactory != nullptr)
		mFactory->StartUp(cooking);
}

PhysicsManager::~PhysicsManager()
{
	if(mFactory != nullptr)
		mFactory->ShutDown();

	PluginLoader::Unload(mPlugin);
	mFactory = nullptr;
}
