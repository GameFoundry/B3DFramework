//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsMatrix4.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsRTTIType.h"
#include "BsGameObjectManager.h"
#include "BsGameObject.h"
#include "BsComponent.h"

namespace BansheeEngine
{
	/** @addtogroup Scene
	 *  @{
	 */

	/** Possible modifiers that can be applied to a SceneObject. */
	enum SceneObjectFlags
	{
		SOF_DontInstantiate = 0x01, /**< Object wont be in the main scene and its components won't receive updates. */
		SOF_DontSave = 0x02,		/**< Object will be skipped when saving the scene hierarchy or a prefab. */
		SOF_Persistent = 0x04,		/**< Object will remain in the scene even after scene clear, unless destroyed directly. 
										 This only works with top-level objects. */
		SOF_Internal = 0x08			/**< Provides a hint to external systems that his object is used by engine internals.
									     For example, those systems might not want to display those objects together with the
										 user created ones. */
	};

	/**
	 * An object in the scene graph. It has a world position, place in the hierarchy and optionally a number of attached 
	 * components.
	 */
	class BS_CORE_EXPORT SceneObject : public GameObject
	{
		/**	Flags that signify which part of the SceneObject needs updating. */
		enum DirtyFlags
		{
			LocalTfrmDirty = 0x01,
			WorldTfrmDirty = 0x02
		};

		friend class CoreSceneManager;
		friend class Prefab;
		friend class PrefabDiff;
		friend class PrefabUtility;
	public:
		~SceneObject();

		/**
		 * Creates a new SceneObject with the specified name. Object will be placed in the top of the scene hierarchy.
		 *
		 * @param[in]	name	Name of the scene object.
		 * @param[in]	flags	Optional flags that control object behavior. See SceneObjectFlags.
		 */
		static HSceneObject create(const String& name, UINT32 flags = 0);

		/**
		 * Destroys this object and any of its held components.
		 *
		 * @param[in]	immediate	If true, the object will be deallocated and become unusable right away. Otherwise the
		 *							deallocation will be delayed to the end of frame (preferred method).
		 */
		void destroy(bool immediate = false);

		/**	Returns a handle to this object. */
		HSceneObject getHandle() const { return mThisHandle; }

		/**
		 * Returns the UUID of the prefab this object is linked to, if any. 
		 *
		 * @param[in]	onlyDirect	If true, this method will return prefab link only for the root object of the prefab
		 *							instance. If false the parent objects will be searched for the prefab ID.
		 */
		String getPrefabLink(bool onlyDirect = false) const;

		/** 
		 * Returns the root object of the prefab instance that this object belongs to, if any. Returns null if the object 
		 * is not part of a prefab instance. 
		 */
		HSceneObject getPrefabParent() const;

		/**
		 * Breaks the link between this prefab instance and its prefab. Object will retain all current values but will no
		 * longer be influenced by modifications to its parent prefab.
		 */
		void breakPrefabLink();

		/**	Checks if the scene object has a specific bit flag set. */
		bool hasFlag(UINT32 flag) const;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GameObject::_setInstanceData */
		void _setInstanceData(GameObjectInstanceDataPtr& other) override;

		/** 
		 * Register the scene object with the scene and activate all of its components. 
		 *
		 * @param[in]	prefabOnly	If true, only objects within the current prefab will be instantiated. If false all child
		 *							objects and components will.
		 */
		void _instantiate(bool prefabOnly = false);

		/**
		 * Clears the internally stored prefab diff. If this object is updated from prefab its instance specific changes 
		 * will be lost.
		 */
		void _clearPrefabDiff() { mPrefabDiff = nullptr; }

		/**
		 * Returns the UUID of the prefab this object is linked to, if any. Unlike getPrefabLink() method this will not
		 * search parents, but instead return only the value assigned to this object.
		 */
		const String& _getPrefabLinkUUID() const { return mPrefabLinkUUID; }

		/**
		 * Allows you to change the prefab link UUID of this object. Normally this should be accompanied by reassigning the
		 * link IDs.
		 */
		void _setPrefabLinkUUID(const String& UUID) { mPrefabLinkUUID = UUID; }

		/**
		 * Returns a prefab diff object containing instance specific modifications of this object compared to its prefab
		 * reference, if any.
		 */
		const SPtr<PrefabDiff>& _getPrefabDiff() const { return mPrefabDiff; }

		/** Assigns a new prefab diff object. Caller must ensure the prefab diff was generated for this object. */
		void _setPrefabDiff(const SPtr<PrefabDiff>& diff) { mPrefabDiff = diff; }

		/** @} */

	private:
		SceneObject(const String& name, UINT32 flags);

		/**
		 * Creates a new SceneObject instance, registers it with the game object manager, creates and returns a handle to
		 * the new object.
		 *
		 * @note	
		 * When creating objects with DontInstantiate flag it is the callers responsibility to manually destroy the object,
		 * otherwise it will leak.
		 */
		static HSceneObject createInternal(const String& name, UINT32 flags = 0);

		/**
		 * Creates a new SceneObject instance from an existing pointer, registers it with the game object manager, creates 
		 * and returns a handle to the object.
		 *			
		 * @param[in]	soPtr		Pointer to the scene object register and return a handle to.
		 * @param[in]	originalId	If the provided pointer was deserialized, this is the original object's ID at the time
		 * 							of serialization. Used for resolving handles pointing to the object.
		 */
		static HSceneObject createInternal(const SPtr<SceneObject>& soPtr, UINT64 originalId = 0);

		/**
		 * Destroys this object and any of its held components.
		 *
		 * @param[in]	handle		Game object handle to this object.
		 * @param[in]	immediate	If true, the object will be deallocated and become unusable right away. Otherwise the 
		 *							deallocation will be delayed to the end of frame (preferred method).
		 *
		 * @note	Unlike destroy(), does not remove the object from its parent.
		 */
		void destroyInternal(GameObjectHandleBase& handle, bool immediate = false) override;

		/** Recursively enables the provided set of flags on this object and all children. */
		void setFlags(UINT32 flags);

		/** Recursively disables the provided set of flags on this object and all children. */
		void unsetFlags(UINT32 flags);

		/**	Checks is the scene object instantiated and visible in the scene. */
		bool isInstantiated() const { return (mFlags & SOF_DontInstantiate) == 0; }

	private:
		HSceneObject mThisHandle;
		String mPrefabLinkUUID;
		SPtr<PrefabDiff> mPrefabDiff;
		UINT32 mPrefabHash;
		UINT32 mFlags;

		/************************************************************************/
		/* 								Transform	                     		*/
		/************************************************************************/
	public:
		/**	Sets the local position of the object. */
		void setPosition(const Vector3& position);

		/**	Gets the local position of the object. */
		const Vector3& getPosition() const { return mPosition; }

		/**	Sets the world position of the object. */
		void setWorldPosition(const Vector3& position);

		/**
		 * Gets the world position of the object.
		 *
		 * @note	Performance warning: This might involve updating the transforms if the transform is dirty.
		 */
		const Vector3& getWorldPosition() const;

		/**	Sets the local rotation of the object. */
		void setRotation(const Quaternion& rotation);

		/**	Gets the local rotation of the object. */
		const Quaternion& getRotation() const { return mRotation; }

		/**	Sets the world rotation of the object. */
		void setWorldRotation(const Quaternion& rotation);

		/**
		 * Gets world rotation of the object.
		 *
		 * @note	Performance warning: This might involve updating the transforms if the transform is dirty.
		 */
		const Quaternion& getWorldRotation() const;

		/**	Sets the local scale of the object. */
		void setScale(const Vector3& scale);

		/**	Gets the local scale of the object. */
		const Vector3& getScale() const { return mScale; }

		/**
		 * Sets the world scale of the object.
		 *
		 * @note	This will not work properly if this object or any of its parents have non-affine transform matrices.
		 */
		void setWorldScale(const Vector3& scale);

		/**
		 * Gets world scale of the object.
		 *
		 * @note	Performance warning: This might involve updating the transforms if the transform is dirty.
		 */
		const Vector3& getWorldScale() const;

		/**
		 * Orients the object so it is looking at the provided @p location (world space) where @p up is used for 
		 * determining the location of the object's Y axis.
		 */
		void lookAt(const Vector3& location, const Vector3& up = Vector3::UNIT_Y);

		/**
		 * Gets the objects world transform matrix.
		 *
		 * @note	Performance warning: This might involve updating the transforms if the transform is dirty.
		 */
		const Matrix4& getWorldTfrm() const;

		/**
		 * Gets the objects inverse world transform matrix.
		 *
		 * @note	Performance warning: This might involve updating the transforms if the transform is dirty.
		 */
		Matrix4 getInvWorldTfrm() const;

		/** Gets the objects local transform matrix. */
		const Matrix4& getLocalTfrm() const;

		/**	Moves the object's position by the vector offset provided along world axes. */
        void move(const Vector3& vec);

		/**	Moves the object's position by the vector offset provided along it's own axes (relative to orientation). */
        void moveRelative(const Vector3& vec);

		/**
		 * Gets the Z (forward) axis of the object, in world space.
		 *
		 * @return	Forward axis of the object.
		 */
		Vector3 getForward() const { return getWorldRotation().rotate(-Vector3::UNIT_Z); }

		/**
		 * Gets the Y (up) axis of the object, in world space.
		 *
		 * @return	Up axis of the object.
		 */
		Vector3 getUp() const { return getWorldRotation().rotate(Vector3::UNIT_Y); }

		/**
		 * Gets the X (right) axis of the object, in world space.
		 *
		 * @return	Right axis of the object.
		 */
		Vector3 getRight() const { return getWorldRotation().rotate(Vector3::UNIT_X); }

		/**
		 * Rotates the game object so it's forward axis faces the provided direction.
		 *
		 * @param[in]	forwardDir	The forward direction to face, in world space.
		 *
		 * @note	Local forward axis is considered to be negative Z.
		 */
		void setForward(const Vector3& forwardDir);

		/**	Rotate the object around an arbitrary axis. */
        void rotate(const Vector3& axis, const Radian& angle);

		/**	Rotate the object around an arbitrary axis using a Quaternion. */
        void rotate(const Quaternion& q);

		/**
		 * Rotates around local Z axis.
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void roll(const Radian& angle);

		/**
		 * Rotates around Y axis.
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void yaw(const Radian& angle);

		/**
		 * Rotates around X axis
		 *
		 * @param[in]	angle	Angle to rotate by.
		 */
		void pitch(const Radian& angle);

		/**
		 * Forces any dirty transform matrices on this object to be updated.
		 *
		 * @note	
		 * Normally this is done internally when retrieving a transform, but sometimes it is useful to update transforms
		 * manually.
		 */
		void updateTransformsIfDirty();

		/**
		 * Returns a hash value that changes whenever a scene objects transform gets updated. It allows you to detect 
		 * changes with the local or world transforms without directly comparing their values with some older state.
		 */
		UINT32 getTransformHash() const { return mDirtyHash; }

	private:
		Vector3 mPosition;
		Quaternion mRotation;
		Vector3 mScale;

		mutable Vector3 mWorldPosition;
		mutable Quaternion mWorldRotation;
		mutable Vector3 mWorldScale;

		mutable Matrix4 mCachedLocalTfrm;
		mutable Matrix4 mCachedWorldTfrm;

		mutable UINT32 mDirtyFlags;
		mutable UINT32 mDirtyHash;

		/** 
		 * Notifies components and child scene object that a transform has been changed.  
		 * 
		 * @param	flags	Specifies in what way was the transform changed.
		 */
		void notifyTransformChanged(TransformChangedFlags flags) const;

		/** Updates the local transform. Normally just reconstructs the transform matrix from the position/rotation/scale. */
		void updateLocalTfrm() const;

		/**
		 * Updates the world transform. Reconstructs the local transform matrix and multiplies it with any parent transforms.
		 *
		 * @note	If parent transforms are dirty they will be updated.
		 */
		void updateWorldTfrm() const;

		/**	Checks if cached local transform needs updating. */
		bool isCachedLocalTfrmUpToDate() const { return (mDirtyFlags & DirtyFlags::LocalTfrmDirty) == 0; }

		/**	Checks if cached world transform needs updating. */
		bool isCachedWorldTfrmUpToDate() const { return (mDirtyFlags & DirtyFlags::WorldTfrmDirty) == 0; }

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
		void setParent(const HSceneObject& parent, bool keepWorldTransform = true);

		/**
		 * Gets the parent of this object.
		 *
		 * @return	Parent object, or nullptr if this SceneObject is at root level.
		 */
		HSceneObject getParent() const { return mParent; }

		/**
		 * Gets a child of this item.
		 *
		 * @param[in]	idx	The zero based index of the child.
		 * @return		SceneObject of the child.
		 */
		HSceneObject getChild(UINT32 idx) const;

		/**
		 * Find the index of the specified child. Don't persist this value as it may change whenever you add/remove children.
		 *
		 * @param[in]	child	The child to look for.
		 * @return				The zero-based index of the found child, or -1 if no match was found.
		 */
		int indexOfChild(const HSceneObject& child) const;

		/**	Gets the number of all child GameObjects. */
		UINT32 getNumChildren() const { return (UINT32)mChildren.size(); }

		/**
		 * Searches the child objects for an object matching the specified name.
		 *
		 * @param[in]	name		Name of the object to locate.
		 * @param[in]	recursive	If true all descendants of the scene object will be searched, otherwise only immediate
		 *							children.
		 * @return					First found scene object, or empty handle if none found.
		 */
		HSceneObject findChild(const String& name, bool recursive = true);

		/**
		 * Searches the child objects for objects matching the specified name.
		 *
		 * @param[in]	name		Name of the objects to locate.
		 * @param[in]	recursive	If true all descendants of the scene object will be searched, otherwise only immediate
		 *							children.
		 * @return					All scene objects matching the specified name.
		 */
		Vector<HSceneObject> findChildren(const String& name, bool recursive = true);

		/**
		 * Enables or disables this object. Disabled objects also implicitly disable all their child objects. No components
		 * on the disabled object are updated.
		 */
		void setActive(bool active);

		/**
		 * Returns whether or not an object is active.
		 *
		 * @param[in]	self	If true, the method will only check if this particular object was activated or deactivated
		 *						directly via setActive. If false we we also check if any of the objects parents are inactive.
		 */
		bool getActive(bool self = false);

		/**
		 * Makes a deep copy of this object.
		 * 			
		 * @param[in]	instantiate	If false, the cloned hierarchy will just be a memory copy, but will not be present in the
		 * 							scene or otherwise active until instantiate() is called.
		 */
		HSceneObject clone(bool instantiate = true);

	private:
		HSceneObject mParent;
		Vector<HSceneObject> mChildren;
		bool mActiveSelf;
		bool mActiveHierarchy;

		/**
		 * Internal version of setParent() that allows you to set a null parent.
		 *
		 * @param[in]	parent			New parent.
		 * @param[in]	keepWorldPos	Determines should the current transform be maintained even after the parent is 
		 *								changed (this means the local transform will be modified accordingly).
		 */
		void _setParent(const HSceneObject& parent, bool keepWorldTransform = true);

		/**
		 * Adds a child to the child array. This method doesn't check for null or duplicate values.
		 *
		 * @param[in]	object	New child.
		 */
		void addChild(const HSceneObject& object);
		
		/**
		 * Removes the child from the object. 
		 *
		 * @param[in]	object	Child to remove.
		 */
		void removeChild(const HSceneObject& object);

		/** Changes the object active in hierarchy state, and triggers necessary events. */
		void setActiveHierarchy(bool active, bool triggerEvents = true);

		/************************************************************************/
		/* 								Component	                     		*/
		/************************************************************************/
	public:
		/** Constructs a new component of the specified type and adds it to the internal component list. */
		template<class T, class... Args>
		GameObjectHandle<T> addComponent(Args &&... args)
		{
			static_assert((std::is_base_of<BansheeEngine::Component, T>::value),
				"Specified type is not a valid Component.");

			SPtr<T> gameObject(new (bs_alloc<T>()) T(mThisHandle,
				std::forward<Args>(args)...),
				&bs_delete<T>, StdAlloc<T>());

			GameObjectHandle<T> newComponent =
				GameObjectManager::instance().registerObject(gameObject);

			newComponent->mThisHandle = newComponent;
			mComponents.push_back(newComponent);

			if (isInstantiated())
			{
				newComponent->_instantiate();
				newComponent->onInitialized();

				if (getActive())
					newComponent->onEnabled();
			}

			return newComponent;
		}

		/**
		 * Searches for a component with the specific type and returns the first one it finds. Will also return components
		 * derived from the type.
		 * 			
		 * @tparam	typename T	Type of the component.
		 * @return				Component if found, nullptr otherwise.
		 *
		 * @note	
		 * Don't call this too often as it is relatively slow. It is more efficient to call it once and store the result 
		 * for further use.
		 */
		template <typename T>
		GameObjectHandle<T> getComponent()
		{
			static_assert((std::is_base_of<BansheeEngine::Component, T>::value), 
				"Specified type is not a valid Component.");

			return static_object_cast<T>(getComponent(T::getRTTIStatic()));
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
		Vector<GameObjectHandle<T>> getComponents()
		{
			static_assert((std::is_base_of<BansheeEngine::Component, T>::value), 
				"Specified type is not a valid Component.");

			Vector<GameObjectHandle<T>> output;

			for (auto entry : mComponents)
			{
				if (entry->getRTTI()->isDerivedFrom(T::getRTTIStatic()))
					output.push_back(entry);
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
		bool hasComponent()
		{
			static_assert((std::is_base_of<BansheeEngine::Component, T>::value), 
				"Specified type is not a valid Component.");

			for (auto entry : mComponents)
			{
				if (entry->getRTTI()->isDerivedFrom(T::getRTTIStatic()))
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
		HComponent getComponent(RTTITypeBase* type) const;

		/**
		 * Removes the component from this object, and deallocates it. 
		 *
		 * @param[in]	component	The component to destroy.
		 * @param[in]	immediate	If true, the component will be deallocated and become unusable right away. Otherwise 
		 *							the deallocation will be delayed to the end of frame (preferred method).
		 */
		void destroyComponent(const HComponent component, bool immediate = false);

		/**
		 * Removes the component from this object, and deallocates it.
		 *
		 * @param[in]	component	The component to destroy.
		 * @param[in]	immediate	If true, the component will be deallocated and become unusable right away. Otherwise 
		 *							the deallocation will be delayed to the end of frame (preferred method).
		 */
		void destroyComponent(Component* component, bool immediate = false);

		/**	Returns all components on this object. */
		const Vector<HComponent>& getComponents() const { return mComponents; }

	private:
		/**	Creates an empty component with the default constructor. */
		template <typename T>
		static SPtr<T> createEmptyComponent()
		{
			static_assert((std::is_base_of<BansheeEngine::Component, T>::value), "Specified type is not a valid Component.");

			T* rawPtr = new (bs_alloc<T>()) T();
			SPtr<T> gameObject(rawPtr, &bs_delete<T>, StdAlloc<T>());

			return gameObject;
		}

		/**	Adds the component to the internal component array. */
		void addComponentInternal(const SPtr<Component> component);

		Vector<HComponent> mComponents;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GameObjectRTTI;
		friend class SceneObjectRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}