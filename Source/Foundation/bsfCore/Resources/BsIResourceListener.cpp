//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsIResourceListener.h"
#include "Managers/BsResourceListenerManager.h"

namespace bs
{
	IResourceListener::IResourceListener()
	{
		ResourceListenerManager::instance().registerListener(this);
	}

	IResourceListener::~IResourceListener()
	{
		ResourceListenerManager::instance().unregisterListener(this);
	}

	void IResourceListener::MarkListenerResourcesDirty()
	{
		ResourceListenerManager::instance().markListenerDirty(this);
	}
}
