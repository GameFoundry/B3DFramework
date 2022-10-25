//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsIResourceListener.h"
#include "Managers/BsResourceListenerManager.h"

using namespace bs;

IResourceListener::IResourceListener()
{
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
