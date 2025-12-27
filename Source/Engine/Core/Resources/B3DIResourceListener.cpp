//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DIResourceListener.h"
#include "Managers/B3DResourceListenerManager.h"

using namespace b3d;

IResourceListener::IResourceListener()
{
	if(ResourceListenerManager::IsStarted()) // May not be started in the default object case, which gets constructed early
		ResourceListenerManager::Instance().RegisterListener(this);
}

IResourceListener::~IResourceListener()
{
	ResourceListenerManager::Instance().UnregisterListener(this);
}

void IResourceListener::MarkListenerResourcesDirty()
{
	ResourceListenerManager::Instance().MarkListenerDirty(this);
}
