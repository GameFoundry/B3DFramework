//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Scene/B3DGameObject.h"

namespace b3d
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**
	 * Collection of all game objects associated with a particular scene or prefab. Provides functionality to patch game object handles
	 * as may be needed when deserializing, updating from prefab, applying deltas, and similar.
	 */
	class B3D_EXPORT GameObjectCollection final : public std::enable_shared_from_this<GameObjectCollection>
	{
		struct PrivatelyConstruct { };
	public:
		GameObjectCollection(PrivatelyConstruct);
		~GameObjectCollection();

		/** Unique ID for the collection. */
		UUID GetId() const { return mId; }

		/**
		 * Registers a newly created game object and returns the handle to the object.
		 *
		 * @param[in]	object			Constructed GameObject to wrap in the handle and initialize.
		 * @return						Handle to the GameObject.
		 */
		GameObjectHandle RegisterNewObject(const SPtr<GameObject>& object);

		/** Registers a game object that already has a handle. */
		void RegisterExistingObject(const GameObjectHandle& object);

		/**
		 * Unregisters the game object. Handles to this object will no longer be valid after this call. 
		 *
		 * @param	object					Object handle to unregister.
		 * @param	triggerDestroyEvent		If true, will trigger OnDestroyed event. Should be true if the object was instantiated
		 *									and is being destroyed.
		 */
		void UnregisterObject(GameObjectHandle& object, bool triggerDestroyEvent);

		/** Attempts to find a game object based on the provided ID. Returns empty handle if ID cannot be found. */
		GameObjectHandle GetObject(const UUID& id) const;

		/**
		 * Attempts to find a game object handle based on the provided ID. Returns true if object with the
		 * specified ID is found, false otherwise.
		 */
		bool TryGetObject(const UUID& id, GameObjectHandle& outObject) const;

		/** Checks if the GameObject with the specified ID exists. */
		bool ObjectExists(const UUID& id) const;

		/** Returns the number of game objects registered with this collection. */
		u32 GetObjectCount() const { return (u32)mObjects.size(); }

		/**
		 * Call this method after you clone/copy/deserialize a new instance of an existing game object. It will ensure that
		 * any existing handles are updated so they point to the newly created object. The old object is expected to be
		 * discarded, as they cannot both exist at once.
		 *
		 * @param	newObjectHandle				Newly created handle pointing to the newly created object.
		 * @param	originalObjectInstanceData	Game object instance data of the game object we're replacing.
		 *
		 * @note
		 * The internal mechanism works as follows:
		 *	- @p originalObjectInstanceData is updated so it points to the new object (as referenced by @p handle)
		 *  - @p newObjectHandle is updated so it points to @p originalObjectInstanceData 
		 *	- Newly created game object is updated so it holds @p originalObjectInstanceData 
		 *  - Collection ID mapping is updated in case object ID changed
		 *
		 * This works because any old handles will point to the original GameObjectInstanceData, which we just patched to
		 * point to the new object.
		 *
		 * And any new handles will be patched when we update @p newObjectHandle itself, as our system guarantees that all handles
		 * created during a single clone/deserialization operation share the same handle data, so it's enough to just patch
		 * this handle.
		 */
		void ReplaceGameObjectInstance(GameObjectHandle& newObjectHandle, const SPtr<GameObjectInstanceData>& originalObjectInstanceData);

		/** Changes the instance ID of the provided game object. */
		void ChangeGameObjectId(GameObjectHandle& gameObject, const UUID& newId);

		/**
		 * Notifies the collection that we are about to register game objects whose handle might not be immediately valid. This
		 * is primarily used during deserialization, as game objects and handles will be registered with the collection during
		 * deserialization as they are deserialized, but may be resolved only after deserialization for the entire collection is
		 * complete.
		 *
		 * Must be followed by EndHandleResolve(), which will resolve any unresolved handles. When handle resolve is active you are
		 * allowed to call the following methods:
		 *  - RegisterUnresolvedHandle()
		 *	- RegisterUnresolvedHandleIdRemapping()
		 */
		void BeginHandleResolve();

		/**
		 * Ends the handle resolve process started in BeginHandleResolve(). Resolves any unresolved handles by mapping them to
		 * the correct game object instance data and correct id. */
		void EndHandleResolve();

		/**
		 * Registers a handle to be resolved when EndHandleResolve() is called. Unresolved handles are generated during deserialization,
		 * as the objects they are pointing to might not be deserialized yet.
		 */
		void RegisterUnresolvedHandle(GameObjectHandle& handle);

		/**
		 * Notifies the system that id of the object an resolved handle is pointing to has changed. Useful if you are generating new IDs
		 * during deserialization.
		 *
		 * @param	originalId		Current ID, as registered by RegisterUnresolvedHandle.
		 * @param	newId			New wanted ID of the object.
		 */
		void RegisterUnresolvedHandleIdRemapping(const UUID& originalId, const UUID& newId);

		/**	Queues the object to be destroyed at the end of a GameObject update cycle. */
		void QueueForDestroy(const GameObjectHandle& object);

		/**	Destroys any GameObjects that were queued for destruction. */
		void DestroyQueuedObjects();

		/** Creates a new empty game object collection. */
		static SPtr<GameObjectCollection> Create();

		Event<void(const HGameObject&)> OnDestroyed; /**< Triggered when a game object is being destroyed. */

	private:
		UUID mId;
		bool mHandleResolveActive = false;

		UnorderedMap<UUID, UUID> mUUIDRemapping;
		UnorderedMap<UUID, GameObjectHandle> mObjects;
		UnorderedMap<UUID, GameObjectHandle> mQueuedForDestroy;
		Vector<UUID> mOrderedQueuedForDestroy;
		UnorderedMap<UUID, SPtr<GameObjectHandleData>> mUnresolvedHandleSharedHandleData;
		Vector<GameObjectHandle> mUnresolvedHandles;
	};

	/** @} */
} // namespace b3d
