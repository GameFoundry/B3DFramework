//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"
#include "Reflection/B3DRTTIType.h"
#include "Scene/B3DGameObject.h"
#include "Scene/B3DComponent.h"
#include "Math/B3DTransform.h"
#include "Script/B3DIScriptExportable.h"
#include "ECS/B3DIECSEntityOwner.h"

namespace b3d
{
	class SceneInstance;

	/** @addtogroup Scene
	 *  @{
	 */

	/** Possible modifiers that can be applied to a SceneObject. */
	enum class B3D_SCRIPT_EXPORT() SceneObjectFlag
	{
		DontSave = 1 << 0, /**< Object will be skipped when saving the scene hierarchy or a prefab. */

		/** Object will remain in the scene even after scene clear, unless destroyed directly. This only works with top-level objects. Runtime persistent objects cannot be saved. */
		RuntimePersistent = 1 << 1,

		/** Provides a hint to external systems that his object is used by engine internals. For example, those systems might not want to display those objects together with the user created ones. */
		Internal = 1 << 2
	};

	B3D_FLAGS_OPERATORS(SceneObjectFlag)
	using SceneObjectFlags = Flags<SceneObjectFlag>;

	/**
	 * An object in the scene graph. It has a transform object that allows it to be positioned, scaled and rotated. It can
	 * have other scene objects as children, and will have a scene object as a parent, in which case transform changes
	 * to the parent are reflected to the child scene objects (children are relative to the parent).
	 *
	 * Each scene object can have one or multiple Component%s attached to it, where the components inherit the scene
	 * object's transform, and receive updates about transform and hierarchy changes.
	 */
	class B3D_EXPORT SceneObject : public GameObject
	{
		friend class SceneManager;
		friend class Scene;
		friend class Prefab;
		friend class SceneObjectHierarchyDelta;
		friend class PrefabUtility;

	public:
		~SceneObject();

		/**
		 * Creates a new SceneObject with the specified name. Object will be placed in the top of the scene hierarchy.
		 *
		 * @param	name	Name of the scene object.
		 * @param	flags	Optional flags that control object behavior. See SceneObjectFlags.
		 */
		static HSceneObject Create(const String& name, u32 flags = 0);

		/**
		 * Destroys this object and any of its held components.
		 *
		 * @param	immediate	If true, the object will be deallocated and become unusable right away. Otherwise the
		 *						deallocation will be delayed to the end of frame (preferred method).
		 */
		void Destroy(bool immediate = false);

		/**	Returns a handle to this object. */
		HSceneObject GetHandle() const { return B3DStaticGameObjectCast<SceneObject>(mThisHandle); }

		/** Identifies the prefab resource this object is linked to. Will return an empty ID if the object is not linked to a prefab. */
		const UUID& GetPrefabResourceId() const { return mPrefabResourceId; }

		/** Returns the version of the prefab the prefab instance was created from. Not relevant if the object is not a prefab instance. */
		const UUID& GetPrefabVersion() const { return mPrefabVersion; }

		/**
		 * Returns true if this object is linked to a prefab (See IsPrefabInstance()), and is the root of the prefab instance hierarchy (i.e. its parent is
		 * either not linked to a prefab, or linked to a different prefab). 
		 */
		bool IsPrefabInstanceRoot() const;

		/**
		 * Returns the root object of the prefab instance that this object belongs to, if any. Returns null if the object
		 * is not a prefab instance.
		 */
		HSceneObject GetPrefabInstanceRoot() const;

		/**
		 * Breaks the link between this prefab instance and its prefab. Object will retain all current values but will no
		 * longer be influenced by modifications to its parent prefab, nor will you be able to apply changes from this object
		 * to the prefab resource.
		 */
		void BreakPrefabLink();

		/**	Checks if the scene object has a specific bit flag set. */
		bool HasFlag(SceneObjectFlag flag) const;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		void SetOwnerCollection(const SPtr<GameObjectCollection>& collection) override;

		/** Register the scene object and its children with the scene manager, and initialize all of their components. */
		void Initialize();

		/** @copydoc GetPrefabResourceId */
		void SetPrefabResourceId(const UUID& id) { mPrefabResourceId = id; }

		/** Clears the internally stored prefab delta. If this object is updated from prefab its instance specific changes will be lost. */
		void ClearPrefabDelta() { mPrefabDelta = nullptr; }

		/** Returns a prefab delta object containing instance specific modifications of this object compared to its prefab reference, if any. */
		const SPtr<SceneObjectHierarchyDelta>& GetPrefabDelta() const { return mPrefabDelta; }

		/** Assigns a new prefab delta. Caller must ensure the prefab delta was generated for this object. */
		void SetPrefabDelta(const SPtr<SceneObjectHierarchyDelta>& delta) { mPrefabDelta = delta; }

		/** @copydoc GetPrefabVersion */
		void SetPrefabVersion(const UUID& version) { mPrefabVersion = version; }

		/** Recursively enables the provided set of flags on this object and all children. */
		void SetFlags(SceneObjectFlags flags);

		/** Recursively disables the provided set of flags on this object and all children. */
		void UnsetFlags(SceneObjectFlags flags);

		/** Sets the current scene parent to null. Only useful if an object is replacing an existing scene instance root. */
		void ClearParent();

		/** @} */

	private:
		SceneObject(const String& name, u32 flags);

		/**
		 * Creates a new SceneObject instance and registers it with the game object collection, and returns a handle to
		 * the new object.
		 *
		 * @param	ownerCollection			Collection to register the scene object with.
		 * @param	name					Name of the scene object.
		 * @param	flags					Optional flags that control scene object behaviour.
		 *
		 * @note
		 * When creating objects with DontInstantiate flag it is the callers responsibility to manually destroy the object,
		 * otherwise it will leak.
		 */
		static HSceneObject CreateInternal(const SPtr<GameObjectCollection>& ownerCollection, const String& name, u32 flags = 0);

		/**
		 * Registers an existing SceneObject instance with the game object collection, and returns a handle to the object.
		 *
		 * @param	ownerCollection			Collection to register the scene object with.
		 * @param	sceneObject				Scene object to register.
		 */
		static HSceneObject CreateInternal(const SPtr<GameObjectCollection>& ownerCollection, const SPtr<SceneObject>& sceneObject);

		void DestroyImmediate() override;
		void QueueForDestroy() override;

	private:
		friend class Component;

		UUID mPrefabResourceId; /**< Identifier of the prefab resource that this object is linked to, if any. */
		UUID mPrefabVersion = UUID::kEmpty;
		SPtr<SceneObjectHierarchyDelta> mPrefabDelta;
		SceneObjectFlags mFlags;

		/************************************************************************/
		/* 								Transform	                     		*/
		/************************************************************************/
	public:
		/** Gets the transform object representing object's position/rotation/scale in world space. */
		const Transform& GetTransform() const;

		/** Updates the world transform if it is dirty. */
		void UpdateWorldTransformIfDirty() const;

		/** Gets the transform object representing object's position/rotation/scale relative to its parent. */
		const Transform& GetLocalTransform() const;

		/** Sets a new transform for the object, relative to the parent. */
		void SetLocalTransform(const Transform& transform);

		/**	Sets the local position of the object. */
		void SetPosition(const Vector3& position);

		/**	Sets the world position of the object. */
		void SetWorldPosition(const Vector3& position);

		/**	Sets the local rotation of the object. */
		void SetRotation(const Quaternion& rotation);

		/**	Sets the world rotation of the object. */
		void SetWorldRotation(const Quaternion& rotation);

		/**	Sets the local scale of the object. */
		void SetScale(const Vector3& scale);

		/**
		 * Sets the world scale of the object.
		 *
		 * @note	This will not work properly if this object or any of its parents have non-affine transform matrices.
		 */
		void SetWorldScale(const Vector3& scale);

		/**
		 * Orients the object so it is looking at the provided @p location (world space) where @p up is used for
		 * determining the location of the object's Y axis.
		 */
		void LookAt(const Vector3& location, const Vector3& up = Vector3::kUnitY);

		/**
		 * Gets the objects world transform matrix.
		 *
		 * @note	Performance warning: This might involve updating the transforms if the transform is dirty.
		 */
		Matrix4 GetWorldMatrix() const;

		/**
		 * Gets the objects inverse world transform matrix.
		 *
		 * @note	Performance warning: This might involve updating the transforms if the transform is dirty.
		 */
		Matrix4 GetInvWorldMatrix() const;

		/** Gets the objects local transform matrix. */
		Matrix4 GetLocalMatrix() const;

		/**	Moves the object's position by the vector offset provided along world axes. */
		void Move(const Vector3& vec);

		/**	Moves the object's position by the vector offset provided along it's own axes (relative to orientation). */
		void MoveRelative(const Vector3& vec);

		/**
		 * Rotates the game object so it's forward axis faces the provided direction.
		 *
		 * @param[in]	forwardDir	The forward direction to face, in world space.
		 *
		 * @note	Local forward axis is considered to be negative Z.
		 */
		void SetForward(const Vector3& forwardDir);

		/**	Rotate the object around an arbitrary axis. */
		void Rotate(const Vector3& axis, const Radian& angle);

		/**	Rotate the object around an arbitrary axis using a Quaternion. */
		void Rotate(const Quaternion& q);

		/**
		 * Rotates around local Z axis.
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Roll(const Radian& angle);

		/**
		 * Rotates around Y axis.
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Yaw(const Radian& angle);

		/**
		 * Rotates around X axis
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void Pitch(const Radian& angle);

		/**
		 * Returns a hash value that changes whenever a scene objects transform gets updated. It allows you to detect
		 * changes with the local or world transforms without directly comparing their values with some older state.
		 */
		u32 GetTransformHash() const { return mDirtyHash; }

	public:
		// IECSEntityOwner interface
		void CreateECSEntity(ecs::Registry* registry) override;

	private:
		mutable u32 mDirtyHash = 0;

		/** Adds the ECS mobility tag for the given mobility. For fresh entities with no existing tags. */
		void AddMobilityTag(ObjectMobility mobility);

		/** Returns true if the scene object can be moved. This is true if the object has ObjectMobility::Movable mobility. */
		bool IsMovable() const;

		/** Returns a mutable reference to the local transform stored in the ECS registry. */
		Transform& GetMutableLocalTransform();

		/** Returns a mutable reference to the world transform stored in the ECS registry. */
		Transform& GetMutableWorldTransform();

		/** Recomputes hierarchy depth based on current parent and propagates to all descendants. */
		void UpdateHierarchyDepthFromParent();

		/** Assigns hierarchy depth to this object and descendants, increasing by one per hierarchy level. */
		void UpdateHierarchyDepthRecursive(u16 hierarchyDepth);

		/**
		 * Notifies components and child scene object that a transform has been changed.
		 *
		 * @param	flags		Specifies in what way was the transform changed.
		 */
		void NotifyTransformChanged(TransformChangedFlags flags) const;

		/**
		 * Updates the world transform. Reconstructs the local transform matrix and multiplies it with any parent transforms.
		 *
		 * @note	If parent transforms are dirty they will be updated.
		 */
		void UpdateWorldTransform() const;

		/************************************************************************/
		/* 								Hierarchy	                     		*/
		/************************************************************************/
	public:
		/**
		 * Changes the parent of this object. Also removes the object from the current parent, and assigns it to the new
		 * parent.
		 *
		 * @param[in]	parent				New parent.
		 * @param[in]	keepWorldTransform	Determines should the current transform be maintained even after the parent is
		 *									changed (this means the local transform will be modified accordingly).
		 */
		void SetParent(const HSceneObject& parent, bool keepWorldTransform = true);

		/**
		 * Gets the parent of this object.
		 *
		 * @return	Parent object, or nullptr if this SceneObject is at root level.
		 */
		HSceneObject GetParent() const { return mParent; }

		/**
		 * Gets a child of this item.
		 *
		 * @param[in]	idx	The zero based index of the child.
		 * @return		SceneObject of the child.
		 */
		HSceneObject GetChild(u32 idx) const;

		/**
		 * Find the index of the specified child. Don't persist this value as it may change whenever you add/remove children.
		 *
		 * @param[in]	child	The child to look for.
		 * @return				The zero-based index of the found child, or -1 if no match was found.
		 */
		int IndexOfChild(const HSceneObject& child) const;

		/**	Gets the number of all child scene objects. */
		u32 GetChildCount() const { return (u32)mChildren.size(); }

		/**
		 * Iterates over all the components on this object, and then does the same on all child scene objects recursively. Calls a callback
		 * for each component and scene object.
		 *
		 * @param	onSceneObjectFound		Called for every child object. If the callback returns false, iteration will stop.
		 *									If @p visitSelf is true, it's also called on the root object itself. May be null.
		 * @param	onComponentFound		Called for every component. May be null.
		 * @param	visitSelf				If true, @p onSceneObjectFound will be called for the root object.
		 */
		void IterateHierarchy(const Function<bool(const HSceneObject&)>& onSceneObjectFound, const Function<void(const HComponent&)>& onComponentFound, bool visitSelf = true) const;

		/** Returns the scene this object is part of. Can be null if scene object hasn't been instantiated. */
		SPtr<SceneInstance> GetScene() const { return mParentScene.lock(); }

		/**
		 * Searches the scene object hierarchy to find a child scene object using the provided path.
		 *
		 * @param[in]	path	Path to the property, where each element of the path is separated with "/" Path elements signify
		 *						names of child scene objects (first one relative to this object).
		 */
		HSceneObject FindPath(const String& path) const;

		/**
		 * Searches the child objects for an object matching the specified name.
		 *
		 * @param[in]	name		Name of the object to locate.
		 * @param[in]	recursive	If true all descendants of the scene object will be searched, otherwise only immediate
		 *							children.
		 * @return					First found scene object, or empty handle if none found.
		 */
		HSceneObject FindChild(const String& name, bool recursive = true);

		/**
		 * Searches the child objects for objects matching the specified name.
		 *
		 * @param[in]	name		Name of the objects to locate.
		 * @param[in]	recursive	If true all descendants of the scene object will be searched, otherwise only immediate
		 *							children.
		 * @return					All scene objects matching the specified name.
		 */
		Vector<HSceneObject> FindChildren(const String& name, bool recursive = true);

		/**
		 * Enables or disables this object. Disabled objects also implicitly disable all their child objects. No components
		 * on the disabled object are updated.
		 */
		void SetActive(bool active); // TODO - Active -> Enabled, to match Component and OnEnabled/OnDisabled

		/**
		 * Returns whether or not an object is active.
		 *
		 * @param[in]	self	If true, the method will only check if this particular object was activated or deactivated
		 *						directly via SetActive. If false we we also check if any of the objects parents are inactive.
		 */
		bool GetActive(bool self = false) const;

		/**
		 * Determines the mobility of a scene object. This is used primarily as a performance hint to engine systems. Objects
		 * with more restricted mobility will result in higher performance. Some mobility constraints will be enforced by
		 * the engine itself, while for others the caller must be sure not to break the promise he made when mobility was
		 * set. By default scene object's mobility is unrestricted.
		 */
		void SetMobility(ObjectMobility mobility);

		/** @copydoc SetMobility */
		ObjectMobility GetMobility() const;

		/** Makes a deep copy of this object (including all its children). Cloned object will be parented to the same parent as this object. */
		HSceneObject Clone();

		/** @name Internal
		 *  @{
		 */

		/**
		 * Makes a deep copy of this object (including all its children). The cloned objects will not be parented to any scene object,
		 * not be associated with any scene instance and not initialized.
		 *
		 * @param		cloneOwnerCollection	Collection into which to place the cloned scene objects. If @p preserveIds is true
		 *										this must be a different collection that the current scene object, otherwise IDs would
		 *										conflict. The collection's ECS registry will be used for the cloned hierarchy's entities and components.
		 * @param		preserveIds				If false, each cloned game object will be assigned a brand new ID. Otherwise
		 *										the ID of the original game objects will be preserved.
		 * @return								Cloned scene object hierarchy.
		 */
		HSceneObject Clone(const SPtr<GameObjectCollection>& cloneOwnerCollection, bool preserveIds = false);

		/**
		 * Makes a deep copy of this object (including all its children). Cloned object will be parented to the root of the
		 * provided scene instance.
		 *
		 * @param		cloneSceneInstance		Scene instance into which to place the cloned scene objects. If @p preserveIds is true
		 *										this must be a different scene instance that the current scene object, otherwise IDs would
		 *										conflict.
		 * @param		initialize				If false, the cloned hierarchy will just be a memory copy, but will not be present
		 *										in the scene or otherwise active until Initialize() is called.
		 * @param		preserveIds				If false, each cloned game object will be assigned a brand new ID. Otherwise
		 *										the ID of the original game objects will be preserved. 
		 * @return								Cloned scene object hierarchy.
		 */
		HSceneObject Clone(const SPtr<SceneInstance>& cloneSceneInstance, bool initialize = true, bool preserveIds = false);

		/** @} */

	private:
		friend class SceneInstance;

		WeakSPtr<SceneInstance> mParentScene;
		HSceneObject mParent;
		Vector<HSceneObject> mChildren;

		/**
		 * Internal version of setParent() that allows you to set a null parent.
		 *
		 * @param[in]	parent				New parent.
		 * @param[in]	keepWorldTransform	Determines should the current transform be maintained even after the parent is
		 *									changed (this means the local transform will be modified accordingly).
		 */
		void SetParentInternal(const HSceneObject& parent, bool keepWorldTransform = true);

		/** Changes the owning scene of the scene object and all children. */
		void SetScene(const SPtr<SceneInstance>& scene);

		/**
		 * Adds a child to the child array. This method doesn't check for null or duplicate values.
		 *
		 * @param[in]	object	New child.
		 */
		void AddChild(const HSceneObject& object);

		/**
		 * Removes the child from the object.
		 *
		 * @param[in]	object	Child to remove.
		 */
		void RemoveChild(const HSceneObject& object);

		/** Changes the object active in hierarchy state, and triggers necessary events. */
		void SetActiveHierarchy(bool active, bool triggerEvents = true);

		/************************************************************************/
		/* 								Component	                     		*/
		/************************************************************************/
	public:
		/** Constructs a new component of the specified type and adds it to the internal component list. */
		template <class T, class... Args>
		TGameObjectHandle<T> AddComponent(Args&&... args)
		{
			static_assert((std::is_base_of<b3d::Component, T>::value), "Specified type is not a valid Component.");

			SPtr<T> component(new(B3DAllocate<T>()) T(GetHandle(), std::forward<Args>(args)...), &B3DDelete<T>, StdAlloc<T>());
			component->SetId(UUIDGenerator::GenerateRandom());

			const HComponent componentHandle = RegisterComponentWithOwnerCollection(component);
			InternalAddComponent(componentHandle, true);
			return B3DStaticGameObjectCast<T>(componentHandle);
		}

		/**
		 * Constructs a new component of the specified type id and adds it to the internal component list. Component must
		 * have a parameterless constructor.
		 */
		HComponent AddComponent(u32 typeId);

		/**
		 * Searches for a component with the specific type and returns the first one it finds. Will also return components
		 * derived from the type.
		 *
		 * @tparam	T	Type of the component.
		 * @return		Component if found, nullptr otherwise.
		 *
		 * @note
		 * Don't call this too often as it is relatively slow. It is more efficient to call it once and store the result
		 * for further use.
		 */
		template <typename T>
		TGameObjectHandle<T> GetComponent()
		{
			static_assert((std::is_base_of<b3d::Component, T>::value), "Specified type is not a valid Component.");

			return B3DStaticGameObjectCast<T>(GetComponent(T::GetRttiStatic()));
		}

		/**
		 * Returns all components with the specific type. Will also return components derived from the type.
		 *
		 * @tparam	typename T	Type of the component.
		 * @return				Array of found components.
		 *
		 * @note
		 * Don't call this too often as it is relatively slow. It is more efficient to call it once and store the result
		 * for further use.
		 */
		template <typename T>
		Vector<TGameObjectHandle<T>> GetComponents()
		{
			static_assert((std::is_base_of<b3d::Component, T>::value), "Specified type is not a valid Component.");

			Vector<TGameObjectHandle<T>> output;

			for(auto entry : mComponents)
			{
				if(entry->GetRtti()->IsDerivedFrom(T::GetRttiStatic()))
					output.push_back(B3DStaticGameObjectCast<T>(entry));
			}

			return output;
		}

		/**
		 * Checks if the current object contains the specified component or components derived from the provided type.
		 *
		 * @tparam	typename T	Type of the component.
		 * @return				True if component exists on the object.
		 *
		 * @note	Don't call this too often as it is relatively slow.
		 */
		template <typename T>
		bool HasComponent()
		{
			static_assert((std::is_base_of<b3d::Component, T>::value), "Specified type is not a valid Component.");

			for(auto entry : mComponents)
			{
				if(entry->GetRtti()->IsDerivedFrom(T::GetRttiStatic()))
					return true;
			}

			return false;
		}

		/**
		 * Searches for a component with the specified type and returns the first one it finds. Will also return components
		 * derived from the type.
		 *
		 * @param[in]	type	RTTI information for the type.
		 * @return				Component if found, nullptr otherwise.
		 *
		 * @note
		 * Don't call this too often as it is relatively slow. It is more efficient to call it once and store the result
		 * for further use.
		 */
		HComponent GetComponent(RTTIType* type) const;

		/**	Returns all components on this object. */
		const Vector<HComponent>& GetComponents() const { return mComponents; }

		/**	Creates an empty component with the default constructor. Primarily used for RTTI purposes. */
		template <typename T>
		static SPtr<T> CreateEmptyComponent()
		{
			static_assert((std::is_base_of<b3d::Component, T>::value), "Specified type is not a valid Component.");

			T* rawPtr = new(B3DAllocate<T>()) T();
			SPtr<T> gameObject(rawPtr, &B3DDelete<T>, StdAlloc<T>());
			gameObject->mRTTIData = gameObject;

			return gameObject;
		}

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**	Returns a modifyable list of all components on this object. */
		Vector<HComponent>& GetMutableComponents() { return mComponents; }

		/** Removes the component from the internal component list. This shouldn't be called externally, use Component::Destroy() instead. */
		void RemoveComponent(const HComponent& component);

		/** @} */
	private:
		/** Registers the provided component with the owner game object collection and returns the component handle. */
		HComponent RegisterComponentWithOwnerCollection(const SPtr<Component>& component);

		/**
		 *	Adds the component to the internal component array, and optionally initialized it. Note the component will only
		 *	be initialized if this scene object is initialized and @p initialize flag is true.
		 */
		void InternalAddComponent(const HComponent& component, bool initialize);

		/** Equivalent to AddComponent(const HComponent&, bool), but internally looks up the component handle from the game object collection. */
		void InternalAddComponent(const SPtr<Component>& component, bool initialize);

		Vector<HComponent> mComponents;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GameObjectRTTI;
		friend class SceneObjectRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
