//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Handles all active implementations of IResourceListener interface and notifies them when events they're listening 
	 * to occur.
	 *
	 * @see		IResourceListener
	 */
	class BS_CORE_EXPORT ResourceListenerManager : public Module<ResourceListenerManager>
	{
	public:
		ResourceListenerManager();
		~ResourceListenerManager();

		/**	Register a new listener to notify for events. */
		void registerListener(IResourceListener* listener);

		/**	Unregister a listener so it will no longer receive notifications. */
		void unregisterListener(IResourceListener* listener);

		/**
		 * Marks the listener as dirty which forces the manager to updates its internal list of resources for the 
		 * listener.
		 */
		void markListenerDirty(IResourceListener* listener);

		/**	Refreshes the resource maps based on dirty listeners and sends out the necessary events. */
		void update();

		/**
		 * Forces the listener to send out events about the specified resource immediately, instead of waiting for the 
		 * next update() call.
		 */
		void notifyListeners(const String& resourceUUID);

	private:
		/**	Triggered by the resources system when a resource has finished loading. */
		void onResourceLoaded(const HResource& resource);

		/**	Triggered by the resources system after a resource handle is modified (points to a new resource). */
		void onResourceModified(const HResource& resource);

		/**	Sends resource loaded event to all listeners referencing this resource. */
		void sendResourceLoaded(const HResource& resource);

		/**	Sends resource modified event to all listeners referencing this resource. */
		void sendResourceModified(const HResource& resource);

		/**	Clears all the stored dependencies for the listener. */
		void clearDependencies(IResourceListener* listener);

		/**	Registers all the resource dependencies for the listener. */
		void addDependencies(IResourceListener* listener);

		HEvent mResourceLoadedConn;
		HEvent mResourceModifiedConn;
		
		Set<IResourceListener*> mDirtyListeners;
		Map<UINT64, Vector<IResourceListener*>> mResourceToListenerMap;
		Map<IResourceListener*, Vector<UINT64>> mListenerToResourceMap;

		Map<String, HResource> mLoadedResources;
		Map<String, HResource> mModifiedResources;

		Vector<HResource> mTempResourceBuffer;

		RecursiveMutex mMutex;

#if BS_DEBUG_MODE
		Set<IResourceListener*> mActiveListeners;
#endif
	};

	/** @} */
}