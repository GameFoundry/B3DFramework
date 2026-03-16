//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Scene/B3DGameObject.h"
#include "Math/B3DBounds.h"
#include "ECS/B3DEntity.h"
#include "ECS/B3DIECSEntityOwner.h"

namespace b3d::ecs { class Registry; }

namespace b3d
{
	class SceneInstance;
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

	/**	Signals which portion of a component is dirty. */
	enum class ComponentDirtyFlag
	{
		/** Component transform has changed. Sync the transform to the render thread and perform appropriate actions. */
		Transform = 1 << 0,

		/** Component active state has changed. Sync the active state to the render thread and perform appropriate actions. */
		Active = 1 << 1,

		/**
		 * Some undetermined property on the component has changed. Sync all properties to the render thread and perform appropriate actions. This can always be used
		 * as a catch-all, but prefer more specific dirty flags as they are faster.
		 */
		Everything = 1 << 2,

		/** Some component dependency was marked dirty. */
		Dependency = kDirtyDependencyMask
	};

	typedef Flags<ComponentFlag> ComponentFlags;
	B3D_FLAGS_OPERATORS(ComponentFlag)

	/**
	 * Components represent primary logic elements in the scene. They are attached to scene objects.
	 *
	 * You should implement some or all of Update/FixedUpdate/OnCreated/OnBeginPlay/OnEnabled/OnDisabled/
	 * OnTransformChanged/OnDestroyed methods to implement the relevant component logic. Avoid putting logic in constructors
	 * or destructors.
	 *
	 * Components can be in different states. These states control which of the events listed above trigger:
	 *  - Running - Scene manager is sending out events.
	 *  - Paused - Scene manager is sending out all events except per-frame update().
	 *	- Stopped - Scene manager is not sending out events except for OnCreated/OnDestroyed.
	 *
	 * These states can be changed globally though SceneManager and affect all components. Individual components can
	 * override these states in two ways:
	 *  - Set the ComponentFlag::AlwaysRun to true and the component will always stay in Running state, regardless of
	 *    state set in SceneManager. This flag should be set in constructor and not change during component lifetime.
	 *  - If the component's parent SceneObject is inactive (SceneObject::setActive(false)), or any of his parents are
	 *    inactive, then the component is considered to be in Stopped state, regardless whether the ComponentFlag::AlwaysRun
	 *    flag is set or not.
	 **/
	class B3D_EXPORT Component : public GameObject
	{
	public:
		virtual ~Component() = default;

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

		/** Enables or disables this object. Disabled component is not updated. */
		void SetEnabled(bool enabled);

		/**
		 * Returns whether or not an object is enabled.
		 *
		 * @param	self	If true, the method will only check if this particular object was enabled or disabled
		 *					directly via SetEnabled. If false we also check if any of the objects parents are disabled.
		 */
		bool GetEnabled(bool self = false) const;

		// IECSEntityOwner interface
		void CreateECSEntity(ecs::Registry*) override {} // Components don't create entities

		/** @name Internal
		 *  @{
		 */

		/**
		 * Construct any resources the component needs before use. Called when the parent scene object is initialized.
		 * A non-initialized component shouldn't be used in a live scene (i.e. it should not receive any of the
		 * component logic updates or events).
		 */
		virtual void Initialize();

		/** Sets new flags that determine when is onTransformChanged called. */
		void SetNotifyFlags(TransformChangedFlags flags) { mNotifyFlags = flags; }

		/** Sets or unsets the disable state flag depending on the enabled state of the parent and the component itself. */
		void RefreshEnabledState(bool triggerEvents = true);

		/** Gets the currently assigned notify flags. See SetNotifyFlagsInternal(). */
		TransformChangedFlags GetNotifyFlags() const { return mNotifyFlags; }

		/** @} */
	protected:
		friend class SceneManager;
		friend class SceneInstance;
		friend class SceneInstanceComponents;
		friend class SceneObject;
		friend class SceneObjectRTTI;

		Component(HSceneObject parent);

		/** Called once when the component has been created. Called regardless of the state the component is in. */
		virtual void OnCreated() {}

		/**
		 * Called once when the component first leaves the Stopped state. This includes component creation if requirements
		 * for leaving Stopped state are met, in which case it is called after OnCreated. Note this is called even if
		 * the component is in disabled state.
		 */
		virtual void OnBeginPlay() {}

		/**	Called once just before the component is destroyed. Called regardless of the state the component is in. */
		virtual void OnDestroyed() {}

		/**
		 * Called every time a component is placed into the Stopped state. This includes component destruction if component
		 * wasn't already in Stopped state during destruction. When called during destruction it is called before
		 * OnDestroyed.
		 */
		virtual void OnDisabled() {}

		/**
		 * Called every time a component leaves the Stopped state, if the component is enabled. This includes component creation
		 * if requirements for leaving the Stopped state are met. When called during creation it is called after OnBeginPlay.
		 */
		virtual void OnEnabled() {}

		/**
		 * Called when the component's parent scene object has changed. Not called if the component is in Stopped state.
		 * Also only called if necessary notify flags are set via SetNotifyFlagsInternal().
		 */
		virtual void OnTransformChanged(TransformChangedFlags flags) {}

		/**
		 * Called when the parent SceneObject's ECS entity is migrated to a new registry (e.g. when moving between
		 * scenes). The old scene and entity are provided so that components can access sub-systems (renderer, physics,
		 * etc.) and read any data stored only in ECS fragments.
		 */
		virtual void OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity) {}

		/**
		 * Destroys the component without delay. Object will be removed from its game object collection, and reference to the object
		 * in all active handles will become null. If @p removeFromParent is specified, the component will be removed from the parent's
		 * child list, otherwise it's expected the caller to perform the removal.
		 */
		void DestroyImmediate(bool removeFromParent);

		/**
		 * Queues the component to be destroyed at the end of the frame. If @p removeFromParent is specified, the component
		 * will be removed from the parent's child list, otherwise it's expected the caller to perform the removal.
		 */
		void QueueForDestroy(bool removeFromParent);

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
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	protected:
		Component() = default; // Serialization only
	};

	/** @} */
} // namespace b3d
