//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DRenderThread.h"
#include "CoreObject/B3DRenderProxy.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup RenderThread-Internal
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
	class B3D_EXPORT CoreObjectManager : public Module<CoreObjectManager>
	{
		/**
		 * Stores dirty data that is to be transferred from main thread to render thread part of a CoreObject, for a single
		 * object.
		 */
		struct PerObjectSyncData
		{
			PerObjectSyncData()
				: InternalId(0)
			{}

			PerObjectSyncData(const SPtr<render::RenderProxy> destinationObject, u64 internalId, const CoreSyncData& syncData)
				: RenderProxy(destinationObject), SyncData(syncData), InternalId(internalId)
			{}

			SPtr<render::RenderProxy> RenderProxy;
			CoreSyncData SyncData;
			u64 InternalId;
		};

		/**
		 * Stores dirty data that is to be transferred from main thread to thread thread part of a CoreObject, for all dirty
		 * objects in one frame.
		 */
		struct PerFrameSyncData
		{
			FrameAllocator* Allocator = nullptr;
			Vector<PerObjectSyncData> Entries;
			Vector<SPtr<render::RenderProxy>> DestroyedObjects;
		};

		/** Contains information about a dirty CoreObject that requires syncing to the render thread. */
		struct DirtyObjectData
		{
			CoreObject* Object;
			i32 SyncDataId;
		};

	public:
		CoreObjectManager();
		~CoreObjectManager();

		/** Generates a new unique ID for a core object. */
		static u64 GenerateId();

		/** Registers a new CoreObject notifying the manager the object	is created. */
		void RegisterObject(CoreObject* object);

		/** Unregisters a CoreObject notifying the manager the object is destroyed. */
		void UnregisterObject(CoreObject* object);

		/**	Notifies the system that a CoreObject's render proxy data is dirty and needs to be synced with the render thread. */
		void NotifyRenderProxyDirty(CoreObject* object);

		/**	Notifies the system that CoreObject dependencies are dirty and should be updated. */
		void NotifyDependenciesDirty(CoreObject* object);

		/**
		 * Synchronizes all CoreObjects with dirty render proxy data with the render thread. Their dirty data will be allocated using the global
		 * frame allocator and then queued for update using the render thread queue for the calling thread.
		 *
		 * @param swapBuffers		Switch ownership of the current buffer from the main thread to the render thread. All data written during sync download
		 *							will now become owned by the render thread, and a new buffer will be made available on the main thread. Note that
		 *							there is a limited number of buffers (as specified by RenderThread::kSyncBufferCount), and the caller must ensure
		 *							that the render thread is still not using the oldest buffer. Generally this is done by ensuring that the render thread
		 *							never runs more than `RenderThread::kSyncBufferCount - 1` frames ahead of the main thread).
		 *
		 * @note	Main thread only.
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		void SyncToRenderThread(bool swapBuffers);

		/**
		 * Synchronizes an individual dirty CoreObject with the render thread. Its dirty data will be allocated using the
		 * global frame allocator and then queued for update the render thread queue for the calling thread.
		 *
		 * @note	Main thread only.
		 * @note	This is an @ref asyncMethod "asynchronous method".
		 */
		void SyncToRenderThread(CoreObject* object);

	private:
		/**
		 * Stores all syncable data from dirty core objects into memory allocated by the provided allocator. Additional
		 * meta-data is stored internally to be used by call to SyncUpload().
		 *
		 * @param allocator		Allocator to use for allocating memory for stored data.
		 *
		 * @note	Main thread only.
		 * @note	Must be followed by a call to SyncUpload() with the same type.
		 */
		void SyncDownload(FrameAllocator* allocator);

		/**
		 * Copies all the data stored by previous call to SyncDownload() into destination render proxies.
		 *
		 * @note	Render thread only.
		 * @note	Must be preceded by a call to SyncDownload().
		 */
		void SyncUpload();

		/**
		 * Updates the cached list of dependencies and dependants for the specified object.
		 *
		 * @param object			Object to update dependencies for.
		 * @param dependencies		New set of dependencies, or null to clear all dependencies.
		 */
		void UpdateDependencies(CoreObject* object, Vector<CoreObject*>* dependencies);

		Map<u64, CoreObject*> mObjects;
		Map<u64, DirtyObjectData> mDirtyObjects;
		Map<u64, Vector<CoreObject*>> mDependencies;
		Map<u64, Vector<CoreObject*>> mDependants;

		Vector<PerObjectSyncData> mDestroyedSyncData;
		List<PerFrameSyncData> mPerFrameSyncData;

		/**
		 * Allocators used for passing temporary data from main thread to the render thread every frame. As external code
		 * guarantees that render thread will never go more than RenderThread::kSyncBufferCount frames ahead of the main thread,
		 * we use a ring-buffer of allocators. We use one extra buffer as one buffer could currently be in progress of
		 * being passed from main to render thread.
		 */
		FrameAllocator* mSyncAllocators[RenderThread::kSyncBufferCount + 1];
		u32 mActiveFrameAllocatorIndex = 0;

		Mutex mObjectsMutex;

		static std::atomic<u64> NextAvailableId;

	};

	/** @} */
} // namespace b3d
