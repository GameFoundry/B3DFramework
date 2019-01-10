//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsDeferredCallManager.h"

namespace bs
{
	void DeferredCallManager::queueDeferredCall(std::function<void()> func)
	{
		mCallbacks.push_back(func);
	}

	void DeferredCallManager::_update()
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

	// Declared in BsPrerequisites.h
	void deferredCall(std::function<void()> callback)
	{
		DeferredCallManager::instance().queueDeferredCall(callback);
	}
}