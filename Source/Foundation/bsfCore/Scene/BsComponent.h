//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Scene/BsGameObject.h"
#include "Math/BsBounds.h"

namespace bs
{
	/** @addtogroup Scene
	 *  @{
	 */

	/** Flags that control behavior of a Component. */
	enum class ComponentFlag
	{
		/**
		 * Ensures that scene manager cannot pause or stop component callbacks from executing. Off by default.
		 * Note that this flag must be specified on component creation, in its constructor and any later changes
		 * to the flag could be ignored.
		 */
		AlwaysRun = 1
	};

	typedef Flags<ComponentFlag> ComponentFlags;
	B3D_FLAGS_OPERATORS(ComponentFlag)

	/**
	 * Components represent primary logic elements in the scene. They are attached to scene objects.
	 *
	 * You should implement some or all of update/fixedUpdate/onCreated/onInitialized/onEnabled/onDisabled/
	 * onTransformChanged/onDestroyed methods to implement the relevant component logic. Avoid putting logic in constructors
	 * or destructors.
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
	class B3D_CORE_EXPORT Component : public GameObject
	{
	public:
		/**	Returns a handle to this object. */
		HComponent GetHandle() const { return B3DStaticGameObjectCast<Component>(mThisHandle); }

		/**	Returns the SceneObject this Component is assigned to. */
		const HSceneObject& SceneObject() const { return mParent; }

		/** @copydoc SceneObject */
		const HSceneObject& SO() const { return SceneObject(); }

		/** Called once per frame. Only called if the component is in Running state. */
		virtual void Update() {}

		/**
		 * Called at fixed time intervals (e.g. 60 times per frame). Generally any physics-related functionality should
		 * go in this method in order to ensure stability of calculations. Only called if the component is in Running
		 * state.
		 */
		virtual void FixedUpdate() {}

		/**
		 * Calculates bounds of the visible contents represented by this component (for example a mesh for Renderable).
		 *
		 * @param[in]	bounds	Bounds of the contents in world space coordinates.
		 * @return				True if the component has bounds with non-zero volume, otherwise false.
		 */
		virtual bool CalculateBounds(Bounds& bounds);

		/**
		 * Checks if this and the provided component represent the same type.
		 *
		 * @note
		 * RTTI type cannot be checked directly since components can be further specialized internally for scripting
		 * purposes.
		 */
		virtual bool TypeEquals(const Component& other);

		/**
		 * Removes the component from parent SceneObject and deletes it. All the references to this component will be
		 * marked as destroyed and you will get an exception if you try to use them.
		 *
		 * @param[in]	immediate	If true the destruction will be performed immediately, otherwise it will be delayed
		 *							until the end of the current frame (preferred option).
		 */
		void Destroy(bool immediate = false);

		/** @name Internal
		 *  @{
		 */

		/**
		 * Construct any resources the component needs before use. Called when the parent scene object is instantiated.
		 * A non-instantiated component shouldn't be used for any other purpose than serialization.
		 */
		virtual void InstantiateInternal() {}

		/** Sets new flags that determine when is onTransformChanged called. */
		void SetNotifyFlags(TransformChangedFlags flags) { mNotifyFlags = flags; }

		/** Gets the currently assigned notify flags. See SetNotifyFlagsInternal(). */
		TransformChangedFlags GetNotifyFlagsInternal() const { return mNotifyFlags; }

		/** @} */
	protected:
		friend class SceneManager;
		friend class SceneObject;
		friend class SceneObjectRTTI;

		Component(HSceneObject parent);
		virtual ~Component() = default;

		/** Called once when the component has been created. Called regardless of the state the component is in. */
		virtual void OnCreated() {}

		/**
		 * Called once when the component first leaves the Stopped state. This includes component creation if requirements
		 * for leaving Stopped state are met, in which case it is called after onCreated.
		 */
		virtual void OnInitialized() {}

		/**	Called once just before the component is destroyed. Called regardless of the state the component is in. */
		virtual void OnDestroyed() {}

		/**
		 * Called every time a component is placed into the Stopped state. This includes component destruction if component
		 * wasn't already in Stopped state during destruction. When called during destruction it is called before
		 * onDestroyed.
		 */
		virtual void OnDisabled() {}

		/**
		 * Called every time a component leaves the Stopped state. This includes component creation if requirements
		 * for leaving the Stopped state are met. When called during creation it is called after onInitialized.
		 */
		virtual void OnEnabled() {}

		/**
		 * Called when the component's parent scene object has changed. Not called if the component is in Stopped state.
		 * Also only called if necessary notify flags are set via SetNotifyFlagsInternal().
		 */
		virtual void OnTransformChanged(TransformChangedFlags flags) {}

		/** Checks whether the component wants to received the specified transform changed message. */
		bool SupportsNotify(TransformChangedFlags flags) const { return (mNotifyFlags & flags) != 0; }

		/** Enables or disabled a flag controlling component's behaviour. */
		void SetFlag(ComponentFlag flag, bool enabled)
		{
			if(enabled)
				mFlags.Set(flag);
			else
				mFlags.Unset(flag);
		}

		/** Checks if the component has a certain flag enabled. */
		bool HasFlag(ComponentFlag flag) const { return mFlags.IsSet(flag); }

		/** Sets an index that uniquely identifies a component with the SceneManager. */
		void SetSceneManagerId(u32 id) { mSceneManagerId = id; }

		/** Returns an index that unique identifies a component with the SceneManager. */
		u32 GetSceneManagerId() const { return mSceneManagerId; }

		/**
		 * Destroys this component.
		 *
		 * @param[in]	handle		Game object handle this this object.
		 * @param[in]	immediate	If true, the object will be deallocated and become unusable right away. Otherwise the
		 *							deallocation will be delayed to the end of frame (preferred method).
		 *
		 * @note	Unlike Destroy(), does not remove the component from its parent.
		 */
		void DestroyInternal(GameObjectHandleBase& handle, bool immediate) override;

	private:
		Component(const Component& other) {}

	protected:
		TransformChangedFlags mNotifyFlags = TCF_None;
		ComponentFlags mFlags;
		u32 mSceneManagerId = 0;

	private:
		HSceneObject mParent;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class ComponentRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		Component() = default; // Serialization only
	};

	/** @} */
} // namespace bs
