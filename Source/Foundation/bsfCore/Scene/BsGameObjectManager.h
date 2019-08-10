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
	 * @note	Sim thread only.
	 */
	class BS_CORE_EXPORT GameObjectManager : public Module<GameObjectManager>
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
		GameObjectHandleBase registerObject(const SPtr<GameObject>& object);

		/**
		 * Unregisters a GameObject. Handles to this object will no longer be valid after this call. This should be called
		 * whenever a GameObject is destroyed.
		 *
		 * @note	Thread safe.
		 */
		void unregisterObject(GameObjectHandleBase& object);

		/**
		 * Attempts to find a GameObject handle based on the GameObject instance ID. Returns empty handle if ID cannot be
		 * found.
		 *
		 * @note	Thread safe.
		 */
		GameObjectHandleBase getObject(UINT64 id) const;

		/**
		 * Attempts to find a GameObject handle based on the GameObject instance ID. Returns true if object with the
		 * specified ID is found, false otherwise.
		 *
		 * @note	Thread safe.
		 */
		bool tryGetObject(UINT64 id, GameObjectHandleBase& object) const;

		/**	
		 * Checks if the GameObject with the specified instance ID exists.
		 *
		 * @note	Thread safe.
		 */
		bool objectExists(UINT64 id) const;

		/**
		 * Changes the instance ID by which an object can be retrieved by.
		 *
		 * @note	Caller is required to update the object itself with the new ID.
		 * @note	Thread safe.
		 */
		void remapId(UINT64 oldId, UINT64 newId);

		/**
		 * Allocates a new unique game object ID.
		 *
		 * @note	Thread safe.
		 */
		UINT64 reserveId();

		/**	Queues the object to be destroyed at the end of a GameObject update cycle. */
		void queueForDestroy(const GameObjectHandleBase& object);

		/**	Destroys any GameObjects that were queued for destruction. */
		void destroyQueuedObjects();

		/**	Triggered when a game object is being destroyed. */
		Event<void(const HGameObject&)> onDestroyed;

	private:
		std::atomic<UINT64> mNextAvailableID = { 1 } ; // 0 is not a valid ID
		Map<UINT64, GameObjectHandleBase> mObjects;
		Map<UINT64, GameObjectHandleBase> mQueuedForDestroy;

		mutable Mutex mMutex;
	};

	/** Resolves game object handles and ID during deserialization of a game object hierarchy. */
	class BS_CORE_EXPORT GameObjectDeserializationState
	{
	private:
		/**	Contains data for an yet unresolved game object handle. */
		struct UnresolvedHandle
		{
			UINT64 originalInstanceId;
			GameObjectHandleBase handle;
		};

	public:
		/**
		 * Starts game object deserialization.
		 *
		 * @param[in]	options		One or a combination of GameObjectDeserializationModeFlags, controlling how
		 *							are game objects deserialized.
		 */
		GameObjectDeserializationState(UINT32 options = GODM_UseNewIds | GODM_BreakExternal);
		~GameObjectDeserializationState();

		/**	Queues the specified handle and resolves it when deserialization ends. */
		void registerUnresolvedHandle(UINT64 originalId, GameObjectHandleBase& object);

		/** Notifies the system about a new deserialized game object and its original ID. */
		void registerObject(UINT64 originalId, GameObjectHandleBase& object);

		/**	Registers a callback that will be triggered when GameObject serialization ends. */
		void registerOnDeserializationEndCallback(std::function<void()> callback);

		/** Resolves all registered handles and objects, and triggers end callbacks. */
		void resolve();

		/** Determines should new UUID's be generated for deserialized objects, or existing ones kept. */
		bool getUseNewUUIDs() const { return (mOptions & GODM_UseNewUUID) != 0; }

	private:
		UnorderedMap<UINT64, UINT64> mIdMapping;
		UnorderedMap<UINT64, SPtr<GameObjectHandleData>> mUnresolvedHandleData;
		UnorderedMap<UINT64, GameObjectHandleBase> mDeserializedObjects;
		Vector<UnresolvedHandle> mUnresolvedHandles;
		Vector<std::function<void()>> mEndCallbacks;
		UINT32 mOptions;
	};

	/** @} */
}
