//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsIResourceListener.h"
#include "BsResourceListenerManager.h"

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

	void IResourceListener::markListenerResourcesDirty()
	{
		ResourceListenerManager::instance().markListenerDirty(this);
	}
}