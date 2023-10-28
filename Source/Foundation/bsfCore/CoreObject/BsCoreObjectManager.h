//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRenderThread.h"
#include "CoreObject/BsRenderProxy.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup CoreThread-Internal
	 *  @{
	 */

	// TODO Low priority - Add debug option that would remember a call stack for each resource initialization,
	// so when we fail to release one we know which one it is.

	/**
	 * Manager that keeps track of all active CoreObject%s.
	 *
	 * @note	Internal class.
	 * @note	Thread safe unless specified otherwise.
	 */
	class B3D_CORE_EXPORT CoreObjectManager : public Module<CoreObjectManager>
	{
		/**
		 * Stores dirty data that is to be transferred from sim  thread to core thread part of a CoreObject, for a single
		 * object.
		 */
		struct CoreStoredSyncObjData
		{
			CoreStoredSyncObjData()
				: InternalId(0)
			{}

			CoreStoredSyncObjData(const SPtr<ct::RenderProxy> destObj, u64 internalId, const CoreSyncData& syncData)
				: DestinationObj(destObj), SyncData(syncData), InternalId(internalId)
			{}

			SPtr<ct::RenderProxy> DestinationObj;
			CoreSyncData SyncData;
			u64 InternalId;
		};

		/**
		 * Stores dirty data that is to be transferred from sim thread to core thread part of a CoreObject, for all dirty
		 * objects in one frame.
		 */
		struct CoreStoredSyncData
		{
			FrameAllocator* Alloc = nullptr;
			Vector<CoreStoredSyncObjData> Entries;
			Vector<SPtr<ct::RenderProxy>> DestroyedObjects;
		};

		/** Contains information about a dirty CoreObject that requires syncing to the core thread. */
		struct DirtyObjectData
		{
			CoreObject* Object;
			i32 SyncDataId;
		};

	public:
		CoreObjectManager();
		~CoreObjectManager();

		/** Generates a new unique ID for a core object. */
		u64 GenerateId();

		/** Registers a new CoreObject notifying the manager the object	is created. */
		void RegisterObject(CoreObject* object);

		/** Unregisters a CoreObject notifying the manager the object is destroyed. */
		void UnregisterObject(CoreObject* object);

		/**	Notifies the system that a CoreObject is dirty and needs to be synced with the core thread. */
		void NotifyCoreDirty(CoreObject* object);

		/**	Notifies the system that CoreObject dependencies are dirty and should be updated. */
		void NotifyDependenciesDirty(CoreObject* object);

		/**
		 * Synchronizes all dirty CoreObjects with the core thread. Their dirty data will be allocated using the global
		 * frame allocator and then queued for update using the core thread queue for the calling thread.
		 *
		 *	@param swapBuffers		Switch ownership of the current buffer from the main thread to the core thread. All data written during sync download
		 *							will now become owned by the core thread, and a new buffer will be made available on the main thread. Note that
		 *							there is a limited number of buffers (as specified by CoreThread::kSyncBufferCount), and the caller must ensure
		 *							that the core thread is still not using the oldest buffer. Generally this is done by ensuring that the core thread
		 *							never runs more than `CoreThread::kSyncBufferCount - 1` frames ahead of the main thread).
		 *
		 * @note	Sim thread only.
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		void SyncToCore(bool swapBuffers);

		/**
		 * Synchronizes an individual dirty CoreObject with the core thread. Its dirty data will be allocated using the
		 * global frame allocator and then queued for update the core thread queue for the calling thread.
		 *
		 * @note	Sim thread only.
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		void SyncToCore(CoreObject* object);

	private:
		/**
		 * Stores all syncable data from dirty core objects into memory allocated by the provided allocator. Additional
		 * meta-data is stored internally to be used by call to syncUpload().
		 *
		 * @param[in]	allocator Allocator to use for allocating memory for stored data.
		 *
		 * @note	Sim thread only.
		 * @note	Must be followed by a call to syncUpload() with the same type.
		 */
		void SyncDownload(FrameAllocator* allocator);

		/**
		 * Copies all the data stored by previous call to syncDownload() into core thread versions of CoreObjects.
		 *
		 * @note	Core thread only.
		 * @note	Must be preceded by a call to syncDownload().
		 */
		void SyncUpload();

		/**
		 * Updates the cached list of dependencies and dependants for the specified object.
		 *
		 * @param[in]	object			Update to update dependencies for.
		 * @param[in]	dependencies	New set of dependencies, or null to clear all dependencies.
		 */
		void UpdateDependencies(CoreObject* object, Vector<CoreObject*>* dependencies);

		u64 mNextAvailableID;
		Map<u64, CoreObject*> mObjects;
		Map<u64, DirtyObjectData> mDirtyObjects;
		Map<u64, Vector<CoreObject*>> mDependencies;
		Map<u64, Vector<CoreObject*>> mDependants;

		Vector<CoreStoredSyncObjData> mDestroyedSyncData;
		List<CoreStoredSyncData> mCoreSyncData;

		/**
		 * Allocators used for passing temporary data from main thread to the core thread every frame. As external code
		 * guarantees that core thread will never go more than CoreThread::kSyncBufferCount frames ahead of the main thread,
		 * we use a ring-buffer of allocators. We use one extra buffer as one buffer could currently be in progress of
		 * being passed from main to core thread.
		 */
		FrameAllocator* mSyncAllocators[CoreThread::kSyncBufferCount + 1];
		u32 mActiveFrameAllocatorIndex = 0;

		Mutex mObjectsMutex;
	};

	/** @} */
} // namespace bs
