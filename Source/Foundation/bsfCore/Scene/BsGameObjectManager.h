//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Scene/BsGameObject.h"

namespace bs
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**	Possible modes to use when deserializing game objects. */
	enum GameObjectDeserializationModeFlags
	{
		/** All handles will point to old ID that were restored from the deserialized file. */
		GODM_UseOriginalIds = 0x01,
		/** All handles will point to new IDs that were given to the deserialized GameObjects. */
		GODM_UseNewIds = 0x02,
		/** Handles pointing to GameObjects outside of the currently deserialized set
		will attempt to be restored in case those objects are still active. */
		GODM_RestoreExternal = 0x04,
		/** Handles pointing to GameObjects outside of the currently deserialized set
		will be broken. */
		GODM_BreakExternal = 0x08,
		/** Handles pointing to GameObjects that cannot be found will not be set to null. */
		GODM_KeepMissing = 0x10,
		/** When enabled, new UUIDs will be generated for all deserialized game objects. */
		GODM_UseNewUUID = 0x20
	};

	/**
	 * Tracks GameObject creation and destructions. Also resolves GameObject references from GameObject handles.
	 *
	 * @note	Main thread only.
	 */
	class B3D_CORE_EXPORT GameObjectManager : public Module<GameObjectManager>
	{
	public:
		GameObjectManager() = default;
		~GameObjectManager();

		/**
		 * Registers a new GameObject and returns the handle to the object.
		 *
		 * @param[in]	object			Constructed GameObject to wrap in the handle and initialize.
		 * @return						Handle to the GameObject.
		 *
		 * @note	Thread safe.
		 */
		GameObjectHandleBase RegisterObject(const SPtr<GameObject>& object);

		/**
		 * Unregisters a GameObject. Handles to this object will no longer be valid after this call. This should be called
		 * whenever a GameObject is destroyed.
		 *
		 * @note	Thread safe.
		 */
		void UnregisterObject(GameObjectHandleBase& object);

		/**
		 * Attempts to find a GameObject handle based on the GameObject instance ID. Returns empty handle if ID cannot be
		 * found.
		 *
		 * @note	Thread safe.
		 */
		GameObjectHandleBase GetObject(u64 id) const;

		/**
		 * Attempts to find a GameObject handle based on the GameObject instance ID. Returns true if object with the
		 * specified ID is found, false otherwise.
		 *
		 * @note	Thread safe.
		 */
		bool TryGetObject(u64 id, GameObjectHandleBase& object) const;

		/**
		 * Checks if the GameObject with the specified instance ID exists.
		 *
		 * @note	Thread safe.
		 */
		bool ObjectExists(u64 id) const;

		/**
		 * Changes the instance ID by which an object can be retrieved by.
		 *
		 * @note	Caller is required to update the object itself with the new ID.
		 * @note	Thread safe.
		 */
		void RemapId(u64 oldId, u64 newId);

		/**
		 * Allocates a new unique game object ID.
		 *
		 * @note	Thread safe.
		 */
		u64 ReserveId();

		/**	Queues the object to be destroyed at the end of a GameObject update cycle. */
		void QueueForDestroy(const GameObjectHandleBase& object);

		/**	Destroys any GameObjects that were queued for destruction. */
		void DestroyQueuedObjects();

		/**	Triggered when a game object is being destroyed. */
		Event<void(const HGameObject&)> OnDestroyed;

	private:
		friend class GameObjectCollection;

		/** Notifies the manager that a new game object collection was created. */
		void RegisterGameObjectCollection(const SPtr<GameObjectCollection>& collection);

		/** Notifies the manager that a game object collection was about to be destroyed. */
		void UnregisterGameObjectCollection(const GameObjectCollection& collection);

		std::atomic<u64> mNextAvailableID = { 1 }; // 0 is not a valid ID
		Map<u64, GameObjectHandleBase> mObjects;
		Map<u64, GameObjectHandleBase> mQueuedForDestroy;
		UnorderedMap<UUID, WeakSPtr<GameObjectCollection>> mGameObjectCollections;

		mutable Mutex mMutex;
	};

	/** Resolves game object handles and ID during deserialization of a game object hierarchy. */
	class B3D_CORE_EXPORT GameObjectDeserializationState
	{
	private:
		/**	Contains data for an yet unresolved game object handle. */
		struct UnresolvedHandle
		{
			u64 OriginalInstanceId;
			GameObjectHandleBase Handle;
		};

	public:
		/**
		 * Starts game object deserialization.
		 *
		 * @param[in]	options		One or a combination of GameObjectDeserializationModeFlags, controlling how
		 *							are game objects deserialized.
		 */
		GameObjectDeserializationState(u32 options = GODM_UseNewIds | GODM_BreakExternal);
		~GameObjectDeserializationState();

		/**	Queues the specified handle and resolves it when deserialization ends. */
		void RegisterUnresolvedHandle(u64 originalId, GameObjectHandleBase& object);

		/** Notifies the system about a new deserialized game object and its original ID. */
		void RegisterObject(u64 originalId, GameObjectHandleBase& object);

		/**	Registers a callback that will be triggered when GameObject serialization ends. */
		void RegisterOnDeserializationEndCallback(std::function<void()> callback);

		/** Resolves all registered handles and objects, and triggers end callbacks. */
		void Resolve();

		/** Determines should new UUID's be generated for deserialized objects, or existing ones kept. */
		bool GetUseNewUuiDs() const { return (mOptions & GODM_UseNewUUID) != 0; }

	private:
		UnorderedMap<u64, u64> mIdMapping;
		UnorderedMap<u64, SPtr<GameObjectHandleData>> mUnresolvedHandleData;
		UnorderedMap<u64, GameObjectHandleBase> mDeserializedObjects;
		Vector<UnresolvedHandle> mUnresolvedHandles;
		Vector<std::function<void()>> mEndCallbacks;
		u32 mOptions;
	};

	/** @} */
} // namespace bs
