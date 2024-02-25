//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Scene/BsGameObject.h"

namespace bs
{
	class LightProbeVolume;
	class PhysicsScene;

	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/** Information about a scene actor and the scene object it has been bound to. */
	struct BoundActorData
	{
		BoundActorData() = default;

		BoundActorData(const SPtr<SceneActor>& actor, const HSceneObject& so)
			: Actor(actor), So(so)
		{}

		SPtr<SceneActor> Actor;
		HSceneObject So;
	};

	/** Possible states components can be in. Controls which component callbacks are triggered. */
	enum class ComponentState
	{
		Running, /**< All components callbacks are being triggered normally. */
		Paused, /**< All component callbacks except update are being triggered normally. */
		Stopped /**< No component callbacks are being triggered. */
	};

	/** Contains information about an instantiated scene. */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Scene)) SceneInstance
	{
		struct ConstructPrivately
		{};

	public:
		SceneInstance(ConstructPrivately dummy, const String& name, const HSceneObject& root, const SPtr<PhysicsScene>& physicsScene);

		/** Name of the scene. */
		B3D_SCRIPT_EXPORT(ExportName(Name), Property(Getter))
		const String& GetName() const { return mName; }

		/** Root object of the scene. */
		B3D_SCRIPT_EXPORT(ExportName(Root), Property(Getter))
		const HSceneObject& GetRoot() const { return mRoot; }

		/** Checks is the scene currently active. IF inactive the scene properties aside from the name are undefined. */
		B3D_SCRIPT_EXPORT(ExportName(IsActive), Property(Getter))
		bool IsActive() const { return mIsActive; }

		/**
		 * Physical representation of the scene, as assigned by the physics sub-system. Exact implementation depends on the
		 * physics plugin used.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Physics), Property(Getter))
		const SPtr<PhysicsScene>& GetPhysicsScene() const { return mPhysicsScene; }

		/** Returns the game object collection storing all the scene's game objects. */
		const SPtr<GameObjectCollection>& GetGameObjectCollection() const { return mGameObjectCollection; }

		/** Creates a new empty scene instance. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SceneInstance))
		static SPtr<SceneInstance> Create(const String& name);

	private:
		friend class SceneManager;

		String mName;
		HSceneObject mRoot;
		bool mIsActive = true;
		SPtr<PhysicsScene> mPhysicsScene;
		SPtr<GameObjectCollection> mGameObjectCollection;
	};

	/**
	 * Keeps track of all active SceneObject%s and their components. Keeps track of component state and triggers their
	 * events. Updates the transforms of objects as SceneObject%s move.
	 */
	class B3D_CORE_EXPORT SceneManager : public Module<SceneManager>
	{
	public:
		SceneManager();
		~SceneManager();

		/** Returns the object that represents the main scene. */
		const SPtr<SceneInstance>& GetMainScene() const { return mMainScene; }

		/**
		 * Destroys all scene objects in the scene.
		 *
		 * @param[in]	forceAll	If true, then even the persistent objects will be unloaded.
		 */
		void ClearScene(bool forceAll = false);

		/**
		 * Instantiates a new scene and makes it active. All non-persistent objects that are part of the current scene will
		 * be destroyed.
		 */
		void LoadScene(const HPrefab& scene);

		/**
		 * Saves all the currently active scene objects into a brand new prefab which can then be saved to disk, loaded back
		 * and provided to setScene() for loading.
		 */
		HPrefab SaveScene() const;

		/**
		 * Changes the component state that globally determines which component callbacks are activated. Only affects
		 * components that don't have the ComponentFlag::AlwaysRun flag set.
		 */
		void SetComponentState(ComponentState state);

		/** Checks are the components currently in the Running state. */
		bool IsRunning() const { return mComponentState == ComponentState::Running; }

		/**
		 * Returns a list of all components of the specified type currently in the scene.
		 *
		 * @tparam		T			Type of the component to search for.
		 *
		 * @param[in]	activeOnly	If true only active components are returned, otherwise all components are returned.
		 * @return					A list of all matching components in the scene.
		 */
		template <class T>
		Vector<GameObjectHandle<T>> FindComponents(bool activeOnly = true);

		/** Returns all cameras in the scene. */
		const UnorderedMap<Camera*, SPtr<Camera>>& GetAllCameras() const { return mCameras; }

		/**
		 * Returns the camera in the scene marked as main. Main camera controls the final render surface that is displayed
		 * to the user. If there are multiple main cameras, the first one found returned.
		 */
		SPtr<Camera> GetMainCamera() const;

		/**
		 * Sets the render target that the main camera in the scene (if any) will render its view to. This generally means
		 * the main game window when running standalone, or the Game viewport when running in editor.
		 */
		void SetMainRenderTarget(const SPtr<RenderTarget>& rt);

		/** Changes the root scene object. Any persistent objects will remain in the scene, now parented to the new root. */
		void SetRootNodeInternal(const HSceneObject& root);

		/**
		 * Binds a scene actor with a scene object. Every frame the scene object's transform will be monitored for
		 * changes and those changes will be automatically transfered to the actor.
		 */
		void BindActorInternal(const SPtr<SceneActor>& actor, const HSceneObject& so);

		/** Unbinds an actor that was previously bound using bindActor(). */
		void UnbindActorInternal(const SPtr<SceneActor>& actor);

		/** Returns a scene object bound to the provided actor, if any. */
		HSceneObject GetActorSOInternal(const SPtr<SceneActor>& actor) const;

		/**	Notifies the scene manager that a new camera was created. */
		void RegisterCameraInternal(const SPtr<Camera>& camera);

		/**	Notifies the scene manager that a camera was removed. */
		void UnregisterCameraInternal(const SPtr<Camera>& camera);

		/**	Notifies the scene manager that a camera either became the main camera, or has stopped being main camera. */
		void NotifyMainCameraStateChangedInternal(const SPtr<Camera>& camera);

		/** Called every frame. Calls update methods on all scene objects and their components. */
		void UpdateInternal();

		/** Called at fixed time internals. Calls the fixed update method on all active components. */
		void FixedUpdateInternal();

		/** Updates dirty transforms on any core objects that may be tied with scene objects. */
		void UpdateCoreObjectTransformsInternal();

		/** Notifies the manager that a new component has just been created. The manager triggers necessary callbacks. */
		void NotifyComponentCreatedInternal(const HComponent& component, bool parentActive);

		/**
		 * Notifies the manager that a scene object the component belongs to was activated. The manager triggers necessary
		 * callbacks.
		 */
		void NotifyComponentActivatedInternal(const HComponent& component, bool triggerEvent);

		/**
		 * Notifies the manager that a scene object the component belongs to was deactivated. The manager triggers necessary
		 * callbacks.
		 */
		void NotifyComponentDeactivatedInternal(const HComponent& component, bool triggerEvent);

		/** Notifies the manager that a component is about to be destroyed. The manager triggers necessary callbacks. */
		void NotifyComponentDestroyedInternal(const HComponent& component, bool immediate);

	protected:
		/** Types of events that represent component state changes relevant to the scene manager. */
		enum class ComponentStateEventType
		{
			Created,
			Activated,
			Deactivated,
			Destroyed
		};

		/** Describes a single component state change. */
		struct ComponentStateChange
		{
			ComponentStateChange(HComponent obj, ComponentStateEventType type)
				: Obj(std::move(obj)), Type(type)
			{}

			HComponent Obj;
			ComponentStateEventType Type;
		};

		friend class SceneObject;

		/**
		 * Register a new node in the scene manager, on the top-most level of the hierarchy.
		 *
		 * @param[in]	node	Node you wish to add. It's your responsibility not to add duplicate or null nodes. This
		 *						method won't check.
		 *
		 * @note
		 * After you add a node in the scene manager, it takes ownership of its memory and is responsible for releasing it.
		 * Do NOT add nodes that have already been added (if you just want to change their parent). Normally this
		 * method will only be called by SceneObject.
		 */
		void RegisterNewSo(const HSceneObject& node);

		/**	Callback that is triggered when the main render target size is changed. */
		void OnMainRenderTargetResized();

		/**
		 * Adds a component to the specified state list. Caller is expected to first remove the component from any
		 * existing state lists.
		 */
		void AddToStateList(const HComponent& component, u32 listType);

		/** Removes a component from its current scene manager state list (if any). */
		void RemoveFromStateList(const HComponent& component);

		/** Iterates over components that had their state modified and moves them to the appropriate state lists. */
		void ProcessStateChanges();

		/**
		 * Encodes an index and a type into a single 32-bit integer. Top 2 bits represent the type, while the rest represent
		 * the index.
		 */
		static u32 EncodeComponentId(u32 idx, u32 type);

		/** Decodes an id encoded with encodeComponentId(). */
		static void DecodeComponentId(u32 id, u32& idx, u32& type);

		/** Checks does the specified component type match the provided RTTI id. */
		static bool IsComponentOfType(const HComponent& component, u32 rttiId);

	protected:
		SPtr<SceneInstance> mMainScene;

		UnorderedMap<SceneActor*, BoundActorData> mBoundActors;
		UnorderedMap<Camera*, SPtr<Camera>> mCameras;
		Vector<SPtr<Camera>> mMainCameras;

		Vector<HComponent> mActiveComponents;
		Vector<HComponent> mInactiveComponents;
		Vector<HComponent> mUninitializedComponents;

		std::array<Vector<HComponent>*, 3> mComponentsPerState = { { &mActiveComponents, &mInactiveComponents, &mUninitializedComponents } };

		SPtr<RenderTarget> mMainRT;
		HEvent mMainRTResizedConn;

		ComponentState mComponentState = ComponentState::Running;
		bool mDisableStateChange = false;
		Vector<ComponentStateChange> mStateChanges;
	};

	/**	Provides easy access to the SceneManager. */
	B3D_CORE_EXPORT SceneManager& GetSceneManager();

	template <class T>
	Vector<GameObjectHandle<T>> SceneManager::FindComponents(bool activeOnly)
	{
		u32 rttiId = T::GetRttiStatic()->GetRttiId();

		Vector<GameObjectHandle<T>> output;
		for(auto& entry : mActiveComponents)
		{
			if(IsComponentOfType(entry, rttiId))
				output.push_back(B3DStaticGameObjectCast<T>(entry));
		}

		if(!activeOnly)
		{
			for(auto& entry : mInactiveComponents)
			{
				if(IsComponentOfType(entry, rttiId))
					output.push_back(B3DStaticGameObjectCast<T>(entry));
			}

			for(auto& entry : mUninitializedComponents)
			{
				if(IsComponentOfType(entry, rttiId))
					output.push_back(B3DStaticGameObjectCast<T>(entry));
			}
		}

		return output;
	}

	/** @} */
} // namespace bs
