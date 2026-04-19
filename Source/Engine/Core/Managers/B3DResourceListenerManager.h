//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
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
	class B3D_EXPORT ResourceListenerManager : public Module<ResourceListenerManager>
	{
	public:
		ResourceListenerManager();
		~ResourceListenerManager();

		/**
		 * Register a new listener to notify for events.
		 *
		 * @note	Thread safe
		 */
		void RegisterListener(IResourceListener* listener);

		/**
		 * Unregister a listener so it will no longer receive notifications.
		 *
		 * @note	Thread safe
		 */
		void UnregisterListener(IResourceListener* listener);

		/**
		 * Marks the listener as dirty which forces the manager to updates its internal list of resources for the
		 * listener.
		 *
		 * @note	Thread safe
		 */
		void MarkListenerDirty(IResourceListener* listener);

		/**	Refreshes the resource maps based on dirty listeners and sends out the necessary events. */
		void Update();

		/**
		 * Forces the listener to send out events about the specified resource immediately, instead of waiting for the
		 * next Update() call.
		 */
		void NotifyListeners(const UUID& resourceUUID);

	private:
		/** Refreshes the listener mapping for any listeners marked as dirty. */
		void UpdateListeners();

		/**	Triggered by the resources system when a resource has finished loading. */
		void OnResourceLoaded(const HResource& resource);

		/**	Triggered by the resources system after a resource handle is modified (points to a new resource). */
		void OnResourceModified(const HResource& resource);

		/**	Sends resource loaded event to all listeners referencing this resource. */
		void SendResourceLoaded(const HResource& resource);

		/**	Sends resource modified event to all listeners referencing this resource. */
		void SendResourceModified(const HResource& resource);

		/**	Clears all the stored dependencies for the listener. */
		void ClearDependencies(IResourceListener* listener);

		/**	Registers all the resource dependencies for the listener. */
		void AddDependencies(IResourceListener* listener);

		HEvent mResourceLoadedConn;
		HEvent mResourceModifiedConn;

		Set<IResourceListener*> mDirtyListeners;
		Map<u64, Vector<IResourceListener*>> mResourceToListenerMap;
		Map<IResourceListener*, Vector<u64>> mListenerToResourceMap;

		Map<UUID, HResource> mLoadedResources;
		Map<UUID, HResource> mModifiedResources;

		Vector<HResource> mTempResourceBuffer;
		Vector<IResourceListener*> mTempListenerBuffer;

		RecursiveMutex mMutex;

#if B3D_DEBUG
		Set<IResourceListener*> mActiveListeners;
#endif
	};

	/** @} */
} // namespace b3d
