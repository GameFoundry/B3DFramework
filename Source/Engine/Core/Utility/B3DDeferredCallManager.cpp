//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DDeferredCallManager.h"

using namespace b3d;

void DeferredCallManager::QueueDeferredCall(std::function<void()> func)
{
	mCallbacks.push_back(func);
}

void DeferredCallManager::UpdateInternal()
{
	while(!mCallbacks.empty())
	{
		// Copy because callbacks can be queued within callbacks
		Vector<std::function<void()>> callbackCopy = mCallbacks;
		mCallbacks.clear();

		for(auto& call : callbackCopy)
		{
			call();
		}
	}
}

namespace b3d
{
// Declared in B3DPrerequisites.h
void DeferredCall(std::function<void()> callback)
{
	DeferredCallManager::Instance().QueueDeferredCall(callback);
}
} // namespace b3d
