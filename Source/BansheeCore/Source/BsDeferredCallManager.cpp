//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDeferredCallManager.h"

namespace bs
{
	DeferredCallManager::DeferredCallManager()
	{

	}

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