//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsRenderAPIManager.h"
#include "Error/BsException.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Utility/BsDynamicLibrary.h"
#include "Utility/BsDynamicLibraryManager.h"

using namespace bs;

RenderAPIManager::~RenderAPIManager()
{
	if(mRenderAPIInitialized)
	{
		ct::RenderAPI::Instance().Destroy();
		ct::RenderAPI::ShutDown();
	}
}

SPtr<RenderWindow> RenderAPIManager::Initialize(const String& pluginFilename, RENDER_WINDOW_DESC& primaryWindowDesc)
{
	if(mRenderAPIInitialized)
		return nullptr;

	DynamicLibrary* loadedLibrary = GetDynamicLibraryManager().Load(pluginFilename);
	const char* name = "";

	if(loadedLibrary != nullptr)
	{
		typedef const char* (*GetPluginNameFunc)();

		GetPluginNameFunc getPluginNameFunc = (GetPluginNameFunc)loadedLibrary->GetSymbol("getPluginName");
		name = getPluginNameFunc();
	}

	for(auto iter = mAvailableFactories.begin(); iter != mAvailableFactories.end(); ++iter)
	{
		if(strcmp((*iter)->Name(), name) == 0)
		{
			(*iter)->Create();
			mRenderAPIInitialized = true;
			return ct::RenderAPI::Instance().Initialize(primaryWindowDesc);
		}
	}

	return nullptr;
}

void RenderAPIManager::RegisterFactory(SPtr<RenderAPIFactory> factory)
{
	assert(factory != nullptr);

	mAvailableFactories.push_back(factory);
}
