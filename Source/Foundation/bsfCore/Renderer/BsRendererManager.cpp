//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRendererManager.h"

#include "CoreObject/BsRenderThread.h"
#include "Renderer/BsRenderer.h"
#include "Error/BsException.h"
#include "Renderer/BsRendererFactory.h"

using namespace bs;

RendererManager::~RendererManager()
{
	if(mActiveRenderer != nullptr)
		mActiveRenderer->Destroy();
}

void RendererManager::SetActive(const String& name)
{
	for(auto iter = mAvailableFactories.begin(); iter != mAvailableFactories.end(); ++iter)
	{
		if((*iter)->Name() == name)
		{
			SPtr<ct::Renderer> newRenderer = (*iter)->Create();
			if(newRenderer != nullptr)
			{
				if(mActiveRenderer != nullptr)
					mActiveRenderer->Destroy();

				mActiveRenderer = newRenderer;
			}
		}
	}

	if(mActiveRenderer == nullptr)
	{
		B3D_EXCEPT(InternalErrorException, "Cannot initialize renderer. Renderer with the name '" + name + "' cannot be found.")
	}
}

void RendererManager::Initialize(const SPtr<GpuDevice>& gpuDevice)
{
	if(mActiveRenderer != nullptr)
		mActiveRenderer->Initialize(gpuDevice);
}

void RendererManager::RegisterFactoryInternal(SPtr<RendererFactory> factory)
{
	B3D_ASSERT(factory != nullptr);

	mAvailableFactories.push_back(factory);
}

void RendererManager::RequestFrameCapture()
{
	GetRenderThread().PostCommand([this] { mActiveRenderer->RequestFrameCapture(); }, "RendererManager::RequestFrameCapture");
}

