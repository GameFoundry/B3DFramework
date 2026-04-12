//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/B3DAudioManager.h"
#include "Plugin/B3DPluginLoader.h"

using namespace b3d;

AudioManager::AudioManager(const String& pluginName)
{
	mPlugin = PluginLoader::Load(pluginName);
	mFactory = static_cast<AudioFactory*>(mPlugin.ReturnValue);

	if(mFactory != nullptr)
		mFactory->StartUp();
}

AudioManager::~AudioManager()
{
	if(mFactory != nullptr)
		mFactory->ShutDown();

	PluginLoader::Unload(mPlugin);
	mFactory = nullptr;
}
