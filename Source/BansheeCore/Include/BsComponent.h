//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGameObject.h"
#include "BsBounds.h"

namespace bs
{
	/** @addtogroup Scene
	 *  @{
	 */

	/** Flags that control behavior of a Component. */
	enum class ComponentFlag
	{
		AlwaysRun = 1 /**< Ensures that scene manager cannot pause or stop component callbacks from executing. Off by default. */
	};

	typedef Flags<ComponentFlag> ComponentFlags;
	BS_FLAGS_OPERATORS(ComponentFlag)

	/** 
	 * Components represent primary logic elements in the scene. They are attached to scene objects. 
	 *
	 * You should implement some or all of update/onCreated/onInitialized/onEnabled/onDisabled/onTransformChanged/
	 * onDestroyed methods to implement the relevant component logic. Avoid putting logic in constructors or destructors.
	 *
	 * Components can be in different states. These states control which of the events listed above trigger:
	 *  - Running - Scene manager is sending out events.  
	 *  - Paused - Scene manager is sending out all events except per-frame update(). 
	 *	- Stopped - Scene manager is not sending out events except for onCreated/onDestroyed.
	 *	
	 * These states can be changed globally though SceneManager and affect all components. Individual components can
	 * override these states in two ways:
	 *  - Set the ComponentFlag::AlwaysRun to true and the component will always stay in Running state, regardless of
	 *    state set in SceneManager. This flag should be set in constructor and not change during component lifetime.
	 *  - If the component's parent SceneObject is inactive (SceneObject::setActive(false)), or any of his parents are
	 *    inactive, then the component is considered to be in Stopped state, regardless whether the ComponentFlag::AlwaysRun
	 *    flag is set or not.
	 **/
	class BS_CORE_EXPORT Component : public GameObject
	{
	public:
		/**	Returns the SceneObject this Component is assigned to. */
		HSceneObject sceneObject() const { return mParent; }

		/** @copydoc sceneObject */
		HSceneObject SO() const { return sceneObject(); }

		/**	Returns a handle to this object. */
		HComponent getHandle() const { return mThisHandle; }

		/** Called once per frame. Only called if the component is in Running state. */
		virtual void update() { }

		/**
		 * Calculates bounds of the visible contents represented by this component (for example a mesh for Renderable).
		 * 
		 * @param[in]	bounds	Bounds of the contents in world space coordinates.	
		 * @return				True if the component has bounds with non-zero volume, otherwise false.
		 */
		virtual bool calculateBounds(Bounds& bounds);

		/**
		 * Checks if this and the provided component represent the same type.
		 * 			
		 * @note	
		 * RTTI type cannot be checked directly since components can be further specialized internally for scripting 
		 * purposes.
		 */
		virtual bool typeEquals(const Component& other);

		/**
		 * Removes the component from parent SceneObject and deletes it. All the references to this component will be 
		 * marked as destroyed and you will get an exception if you try to use them.
		 *
		 * @param[in]	immediate	If true the destruction will be performed immediately, otherwise it will be delayed 
		 *							until the end of the current frame (preferred option).
		 */
		void destroy(bool immediate = false);

		/** @name Internal 
		 *  @{
		 */

		/**
		 * Construct any resources the component needs before use. Called when the parent scene object is instantiated. 
		 * A non-instantiated component shouldn't be used for any other purpose than serialization.
		 */
		virtual void _instantiate() {}

		/** Sets new flags that determine when is onTransformChanged called. */
		void _setNotifyFlags(TransformChangedFlags flags) { mNotifyFlags = flags; }

		/** Gets the currently assigned notify flags. See _setNotifyFlags(). */
		TransformChangedFlags _getNotifyFlags() const { return mNotifyFlags; }

		/** @} */
	protected:
		friend class SceneManager;
		friend class SceneObject;
		friend class SceneObjectRTTI;

		Component(const HSceneObject& parent);
		virtual ~Component();

		/** Called once when the component has been created. Called regardless of the state the component is in. */
		virtual void onCreated() {}

		/**	
		 * Called once when the component first leaves the Stopped state. This includes component creation if requirements
		 * for leaving Stopped state are met, in which case it is called after onCreated. 
		 */
		virtual void onInitialized() {}

		/**	Called once just before the component is destroyed. Called regardless of the state the component is in. */
		virtual void onDestroyed() {}

		/**	
		 * Called every time a component is placed into the Stopped state. This includes component destruction if component
		 * wasn't already in Stopped state during destruction. When called during destruction it is called before
		 * onDestroyed. 
		 */
		virtual void onDisabled() {}

		/**	
		 * Called every time a component leaves the Stopped state. This includes component creation if requirements
		 * for leaving the Stopped state are met. When called during creation it is called after onInitialized. 
		 */
		virtual void onEnabled() {}

		/** 
		 * Called when the component's parent scene object has changed. Not called if the component is in Stopped state. 
		 * Also only called if necessary notify flags are set via _setNotifyFlags().
		 */
		virtual void onTransformChanged(TransformChangedFlags flags) { }

		/** Checks whether the component wants to received the specified transform changed message. */
		bool supportsNotify(TransformChangedFlags flags) const { return (mNotifyFlags & flags) != 0; }

		/** Enables or disabled a flag controlling component's behaviour. */
		void setFlag(ComponentFlag flag, bool enabled) { enabled ? mFlags.set(flag) : mFlags.unset(flag); }

		/** Checks if the component has a certain flag enabled. */
		bool hasFlag(ComponentFlag flag) const { return mFlags.isSet(flag); }

		/** Sets an index that uniquely identifies a component with the SceneManager. */
		void setSceneManagerIdx(UINT32 idx) { mSceneManagerIdx = idx; }

		/** Returns an index that unique identifies a component with the SceneManager. */
		UINT32 getSceneManagerIdx() const { return mSceneManagerIdx; }

		/**
		 * Destroys this component.
		 *
		 * @param[in]	handle		Game object handle this this object.
		 * @param[in]	immediate	If true, the object will be deallocated and become unusable right away. Otherwise the 
		 *							deallocation will be delayed to the end of frame (preferred method).
		 *
		 * @note	Unlike destroy(), does not remove the component from its parent.
		 */
		void destroyInternal(GameObjectHandleBase& handle, bool immediate = false) override;
	private:
		Component(const Component& other) { }

	protected:
		HComponent mThisHandle;
		TransformChangedFlags mNotifyFlags;
		ComponentFlags mFlags;
		UINT32 mSceneManagerIdx;

	private:
		HSceneObject mParent;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ComponentRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;

	protected:
		Component(); // Serialization only
	};

	/** @} */
}