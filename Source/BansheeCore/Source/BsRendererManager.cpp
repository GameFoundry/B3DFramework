//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererManager.h"
#include "BsCoreRenderer.h"
#include "BsException.h"
#include "BsRendererFactory.h"

namespace bs
{
	RendererManager::~RendererManager()
	{
		if (mActiveRenderer != nullptr)
			mActiveRenderer->destroy();
	}

	void RendererManager::setActive(const String& name)
	{
		for(auto iter = mAvailableFactories.begin(); iter != mAvailableFactories.end(); ++iter)
		{
			if((*iter)->name() == name)
			{
				SPtr<CoreRenderer> newRenderer = (*iter)->create();
				if(newRenderer != nullptr)
				{
					if (mActiveRenderer != nullptr)
						mActiveRenderer->destroy();

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

	void RendererManager::initialize()
	{
		if (mActiveRenderer != nullptr)
			mActiveRenderer->initialize();
	}

	void RendererManager::_registerFactory(SPtr<RendererFactory> factory)
	{
		assert(factory != nullptr);

		mAvailableFactories.push_back(factory);
	}
}