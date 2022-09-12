//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRendererManager.h"
#include "Renderer/BsRenderer.h"
#include "Error/BsException.h"
#include "Renderer/BsRendererFactory.h"

namespace bs
{
	RendererManager::~RendererManager()
	{
		if (mActiveRenderer != nullptr)
			mActiveRenderer->Destroy();
	}

	void RendererManager::SetActive(const String& name)
	{
		for(auto iter = mAvailableFactories.Begin(); iter != mAvailableFactories.end(); ++iter)
		{
			if((*iter)->Name() == name)
			{
				SPtr<ct::Renderer> newRenderer = (*iter)->Create();
				if(newRenderer != nullptr)
				{
					if (mActiveRenderer != nullptr)
						mActiveRenderer->Destroy();

					mActiveRenderer = newRenderer;
				}				
			}
		}

		if(mActiveRenderer == nullptr)
		{
			BS_EXCEPT(InternalErrorException,
				"Cannot initialize renderer. Renderer with the name '" + name + "' cannot be found.")
		}
	}

	void RendererManager::Initialize()
	{
		if (mActiveRenderer != nullptr)
			mActiveRenderer->Initialize();
	}

	void RendererManager::_registerFactory(SPtr<RendererFactory> factory)
	{
		assert(factory != nullptr);

		mAvailableFactories.push_back(factory);
	}
}
