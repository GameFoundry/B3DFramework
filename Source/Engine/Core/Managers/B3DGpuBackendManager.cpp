//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/B3DGpuBackendManager.h"
#include "RenderAPI/B3DGpuBackend.h"
#include "Utility/B3DDynamicLibrary.h"
#include "Utility/B3DDynamicLibraryManager.h"

using namespace b3d;

GpuBackendManager::~GpuBackendManager()
{
	if(mRenderAPIInitialized)
		GpuBackend::ShutDown();
}

void GpuBackendManager::Initialize(const String& pluginFilename)
{
	if (!B3D_ENSURE(!mRenderAPIInitialized))
		return;

	DynamicLibrary* loadedLibrary = GetDynamicLibraryManager().Load(pluginFilename);
	const char* name = "";

	if(loadedLibrary != nullptr)
	{
		typedef const char* (*FnGetPluginName)();

		FnGetPluginName fnGetPluginName = (FnGetPluginName)loadedLibrary->GetSymbol("GetPluginName");
		name = fnGetPluginName();
	}

	for(auto iter = mAvailableFactories.begin(); iter != mAvailableFactories.end(); ++iter)
	{
		if(strcmp((*iter)->Name(), name) == 0)
		{
			(*iter)->Create();
			mRenderAPIInitialized = true;
		}
	}
}

void GpuBackendManager::RegisterFactory(SPtr<GpuBackendFactory> factory)
{
	B3D_ASSERT(factory != nullptr);

	mAvailableFactories.push_back(factory);
}
