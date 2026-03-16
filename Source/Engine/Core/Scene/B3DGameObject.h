//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DIReflectable.h"
#include "Script/B3DIScriptExportable.h"
#include "ECS/B3DEntity.h"
#include "ECS/B3DIECSEntityOwner.h"

namespace b3d::ecs { class Registry; }

namespace b3d
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/** Flags used for notifying child scene object and components when a transform has been changed. */
	enum TransformChangedFlags
	{
		TCF_None = 0, /**< Component will not be notified about any events relating to the transform. */
		TCF_Transform = 1 << 0, /**< Component will be notified when the its position, rotation or scale has changed. */
		TCF_Parent = 1 << 1, /**< Component will be notified when its parent changes. */
		TCF_Mobility = 1 << 2, /**< Component will be notified when mobility state changes. */
		TCF_NotifyStopped = 1 << 3, /**< If set, stopped components will also receive transform & parent changed notifies. */
	};

	/** Flags that specify the state and control behaviour of a GameObject. State of these flags is not serialized. */
	enum class GameObjectTransientFlag
	{
		None = 0,

		/**
		 * Object is initialized, meaning it's registered with the scene manager and sending out events. Objects that are not
		 * initialized are inert and don't notify external systems about their existence (except owner game object collection),
		 * making them perfect for in-memory storage within e.g. resource, undo/redo data, deltas and similar.
		 */
		Initialized = 1 << 0,

		/** Object has been queued for destruction, but hasn't been destroyed just yet. */
		QueuedForDestroy = 1 << 1,

		/**
		 * Object has been destroyed. A shared pointer to the object may be kept by some external system,
		 * but in most cases setting this flag will immediately be followed by object deallocation.
		 */
		Destroyed = 1 << 2,

		/** Game object is disabled, either directly or by one of its parents. */
		Disabled = 1 << 3,
	};

	using GameObjectTransientFlags = Flags<GameObjectTransientFlag>;
	B3D_FLAGS_OPERATORS(GameObjectTransientFlag)

	/** Flags that specify the state and control behaviour of a GameObject. State of these flags is serialized. */
	enum class GameObjectPersistentFlag
	{
		None = 0,

		/** Game object is disabled, directly (i.e. not disabled due to its parent being disabled). */
		DisabledSelf = 1 << 0
	};

	using GameObjectPersistentFlags = Flags<GameObjectPersistentFlag>;
	B3D_FLAGS_OPERATORS(GameObjectPersistentFlag)

	/** @} */
	/** @addtogroup Scene
	 *  @{
	 */

	/**
	 * Type of object that can be referenced by a GameObject handle. Each object has an unique ID and is registered with
	 * the GameObjectManager.
	 */
	class B3D_EXPORT GameObject : public IReflectable, public IScriptExportable, public ecs::IECSEntityOwner
	{
	public:
		GameObject() = default;
		virtual ~GameObject() = default;

		/**	Globally unique identifier of the game object that persists scene save/load. */
		const UUID& GetId() const { return mId; }

		/**	Gets the name of the object. */
		const String& GetName() const { return mName; }

		/**	Sets the name of the object. */
		void SetName(const String& name) { mName = name; }

		/** Checks is the particular object flag set. */
		bool HasGameObjectFlag(GameObjectTransientFlag flag) const { return mTransientGameObjectFlags.IsSet(flag); }

		/** Sets a particular flag on the game object. */
		void SetGameObjectFlag(GameObjectTransientFlag flag) { mTransientGameObjectFlags.Set(flag); }

		/** Removes a particular flag on the game object. */
		void UnsetGameObjectFlag(GameObjectTransientFlag flag) { mTransientGameObjectFlags.Unset(flag); }

		/** Checks is the particular object flag set. */
		bool HasGameObjectFlag(GameObjectPersistentFlag flag) const { return mPersistentGameObjectFlags.IsSet(flag); }

		/** Sets a particular flag on the game object. */
		void SetGameObjectFlag(GameObjectPersistentFlag flag) { mPersistentGameObjectFlags.Set(flag); }

		/** Removes a particular flag on the game object. */
		void UnsetGameObjectFlag(GameObjectPersistentFlag flag) { mPersistentGameObjectFlags.Unset(flag); }

		/** Identifies the equivalent object in the linked prefab. This will be an empty ID if the object is not linked to a prefab. */
		const UUID& GetPrefabObjectId() const { return mPrefabObjectId; }

		/**
		 * Returns true if this object is linked to a prefab. This generally means the object was created as a copy of some prefab, or the prefab
		 * was created from this object.
		 */
		bool IsPrefabInstance() const { return !mPrefabObjectId.Empty(); }

		ecs::Registry* GetECSRegistry() const override { return mECSRegistry; }
		ecs::Entity GetECSEntity() const override { return mECSEntity; }
	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GetId */
		void SetId(const UUID& id) { mId = id; }

		/** @copydoc GetPrefabObjectId */
		void SetPrefabObjectId(const UUID& id) { mPrefabObjectId = id; }

		/**
		 * Replaces the instance data with another object's instance data. This object will basically become the original
		 * owner of the provided instance data as far as all game object handles referencing it are concerned.
		 *
		 * @note
		 * No alive objects should ever be sharing the same instance data. This can be used for restoring dead handles.
		 */
		void SetInstanceData(const SPtr<GameObjectInstanceData>& other);

		/** Returns instance data that identifies this GameObject and is used for referencing by game object handles. */
		virtual const SPtr<GameObjectInstanceData>& GetInstanceData() const { return mInstanceData; }

		/** Returns the collection that this game object is a part of. */
		const WeakSPtr<GameObjectCollection>& GetOwnerCollection() const { return mOwnerCollection; }

		/**
		 * Changes the collection the game object is part of. Game object will be unregistered with the
		 * old collection (if any) and registered with the new collection.
		 */
		virtual void SetOwnerCollection(const SPtr<GameObjectCollection>& collection);

		/**
		 * Destroys the game object without delay. Object will be removed from its game object collection, and reference to the object
		 * in all active handles will become null. If the object contains any child objects or components, those will be destroyed as well.
		 */
		virtual void DestroyImmediate();

		/**
		 * Queues the provided game object to be destroyed at the end of the frame. If the object contains any child objects or components,
		 * those will be queued for destroy as well. Object will not be removed from any parent's child or component list.
		 */
		virtual void QueueForDestroy();

		/** @} */

	protected:
		friend class GameObjectHandle;
		friend class GameObjectManager;
		friend class GameObjectCollection;
		friend class SceneObjectHierarchyDelta;
		friend class PrefabUtility;

		/**	Initializes instance data and assigns the GameObject after construction. */
		void InitializeInstanceData(const SPtr<GameObject>& object);

	protected:
		String mName;
		HGameObject mThisHandle;
		UUID mId; /**< Unique identifier for this object. */
		UUID mPrefabObjectId; /**< Identifier of the object in the prefab that this object is linked to, if any. */
		WeakSPtr<GameObjectCollection> mOwnerCollection; /**< Collection that owns this game object. */
		GameObjectTransientFlags mTransientGameObjectFlags;
		GameObjectPersistentFlags mPersistentGameObjectFlags;

		Any mRTTIData; // RTTI only

		ecs::Registry* mECSRegistry = nullptr;
		ecs::Entity mECSEntity = ecs::kNullEntity;

	private:
		friend class Prefab;
		SPtr<GameObjectInstanceData> mInstanceData;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
		friend class ComponentRTTI;
		friend class SceneObjectRTTI;

	public:
		friend class GameObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
