//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererManager.h"

#include "CoreObject/B3DCoreObjectManager.h"
#include "CoreObject/B3DRenderThread.h"
#include "Plugin/B3DPluginLoader.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererFactory.h"

using namespace b3d;

RendererManager::~RendererManager()
{
	const bool hadPlugin = mPlugin.Library != nullptr || mPlugin.ReturnValue != nullptr;

	if(mActiveRenderer != nullptr)
	{
		mActiveRenderer->Destroy();
		mActiveRenderer = nullptr;
	}

	// Flush any render thread work queued by renderer teardown before unloading
	// the plugin, because the renderer's destructors live in the plugin DLL.
	if(hadPlugin)
	{
		CoreObjectManager::Instance().SyncToRenderThread(true);
		GetRenderThread().PostCommand([] {}, "RendererManager plugin unload flush", true);
	}

	PluginLoader::Unload(mPlugin);
	mFactory = nullptr;
}

void RendererManager::SetActive(const String& pluginName)
{
	mPlugin = PluginLoader::Load(pluginName);
	mFactory = static_cast<RendererFactory*>(mPlugin.ReturnValue);

	if(mFactory != nullptr)
	{
		SPtr<render::Renderer> newRenderer = mFactory->Create();
		if(newRenderer != nullptr)
		{
			if(mActiveRenderer != nullptr)
				mActiveRenderer->Destroy();

			mActiveRenderer = newRenderer;
		}
	}

	B3D_ENSURE_LOG(mActiveRenderer != nullptr, "Cannot initialize renderer. Renderer plugin '{0}' cannot be found.", pluginName);
}

void RendererManager::Initialize(const SPtr<GpuDevice>& gpuDevice)
{
	if(mActiveRenderer != nullptr)
		mActiveRenderer->Initialize(gpuDevice);
}

void RendererManager::RequestFrameCapture()
{
	GetRenderThread().PostCommand([this] { mActiveRenderer->RequestDebugFrameCapture(); }, "RendererManager::RequestFrameCapture");
}

