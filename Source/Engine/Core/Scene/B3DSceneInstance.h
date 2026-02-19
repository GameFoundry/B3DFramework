//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Animation/B3DAnimationScene.h"
#include "CoreObject/B3DCoreObject.h"
#include "Utility/B3DModule.h"
#include "Scene/B3DGameObject.h"
#include "Utility/B3DTime.h"
#include "Scene/B3DGameObjectCollection.h"

namespace b3d
{
	class ParticleScene;
	class RendererScene;
	class LightProbeVolume;
	class PhysicsScene;

	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/** Possible states components can be in. Controls which component callbacks are triggered. */
	enum class ComponentState
	{
		Running, /**< All components callbacks are being triggered normally. */
		Paused, /**< All component callbacks except update are being triggered normally. */
		Stopped /**< No component callbacks are being triggered. */
	};

	/** Maintains a list of all components associated with a SceneInstance, and their current state (running, inactive, uninitialized). */
	class B3D_EXPORT SceneInstanceComponents
	{
	public:
		/**
		 * Changes the component state that globally determines which component callbacks are activated. Only affects
		 * components that don't have the ComponentFlag::AlwaysRun flag set.
		 */
		void SetComponentState(ComponentState state);

		/** Checks are the components currently in the Running state. */
		B3D_SCRIPT_EXPORT(ExportName(IsRunning), Property(Getter))
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
		Vector<TGameObjectHandle<T>> FindComponents(bool activeOnly = true);

		/** Called every frame. Calls update methods on all active components. */
		void Update();

		/** Called at fixed time internals. Calls the fixed update method on all active components. */
		void FixedUpdate();

		/** Notifies the manager that a new component has just been created. The manager triggers necessary callbacks. */
		void NotifyComponentCreated(const HComponent& component, bool parentActive);

		/**
		 * Notifies the manager that a scene object the component belongs to was activated. The manager triggers necessary
		 * callbacks.
		 */
		void NotifyComponentActivated(const HComponent& component, bool triggerEvent); // TODO - Activated -> Enabled, to match OnEnabled

		/**
		 * Notifies the manager that a scene object the component belongs to was deactivated. The manager triggers necessary
		 * callbacks.
		 */
		void NotifyComponentDeactivated(const HComponent& component, bool triggerEvent); // TODO - Deactivated -> Disabled, to match OnDisabled

		/** Notifies the manager that a component is about to be destroyed. The manager triggers necessary callbacks. */
		void NotifyComponentDestroyed(const HComponent& component, bool immediate);

	protected:
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

		Vector<HComponent> mActiveComponents;
		Vector<HComponent> mInactiveComponents;
		Vector<HComponent> mUninitializedComponents;

		std::array<Vector<HComponent>*, 3> mComponentsPerState = { { &mActiveComponents, &mInactiveComponents, &mUninitializedComponents } };

		ComponentState mComponentState = ComponentState::Running;
		bool mDisableStateChange = false;
		Vector<ComponentStateChange> mStateChanges;
	};

	template <class T>
	Vector<TGameObjectHandle<T>> SceneInstanceComponents::FindComponents(bool activeOnly)
	{
		u32 rttiId = T::GetRttiStatic()->GetRttiId();

		Vector<TGameObjectHandle<T>> output;
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

#if B3D_WITH_EDITOR 
	/** Interface that provides editor-specific information about a scene instance. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Scene)) IEditorSceneInstance : public IScriptExportable, public IReflectable
	{
	public:
		IEditorSceneInstance() = default;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class IEditorSceneInstanceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};
#endif

	/** Contains information about an instantiated scene. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Scene)) SceneInstance : public CoreObject, public IScriptExportable, public SceneInstanceComponents
	{
		struct ConstructPrivately
		{};

	public:
		SceneInstance(ConstructPrivately dummy, const String& name, const HSceneObject& root, const UUID& associatedResourceId);
		~SceneInstance();

		void Initialize() override;
		void Destroy() override;

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
		 * Representation of the scene used by the physics sub-system. Contains all the objects that can be physically interacted with.
		 * Exact implementation depends on the physics plugin used.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Physics), Property(Getter))
		const SPtr<PhysicsScene>& GetPhysicsScene() const { return mPhysicsScene; }

		/**
		 * Representation of the scene used by the renderer. Contains all the objects that need to be rendered.
		 * Exact implementation depends on the renderer plugin used.
		 */
		const SPtr<RendererScene>& GetRendererScene() const { return mRendererScene; }

		/** Returns the object responsible for updating animations in this scene. */
		const SPtr<AnimationScene>& GetAnimationScene() const { return mAnimationScene; }

		/** Returns the object responsible for updating particles in this scene. */
		const SPtr<ParticleScene>& GetParticleScene() const { return mParticleScene; }

		/** Returns an object that manages time associated with this scene. */
		const SceneTime& GetTime() const { return mTime; }

		/** @copydoc GetTime */
		SceneTime& GetTime() { return mTime; }

		/** Returns the ID of the resource that the scene instance is associated with (e.g. resource the scene was loaded from.). */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(AssociatedResourceId))
		const UUID& GetAssociatedResourceId() const { return mAssociatedResourceId; }

		/** Returns all cameras in the scene. */
		const UnorderedMap<UUID, HCamera>& GetAllCameras() const { return mCameras; }

		/**
		 * Returns the camera in the scene marked as main. Main camera controls the final render surface that is displayed
		 * to the user. If there are multiple main cameras, the first one found returned.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(MainCamera))
		HCamera GetMainCamera() const;

#if B3D_WITH_EDITOR 
		/** Editor scene instance, if running from within the editor. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(EditorSceneInstance))
		SPtr<IEditorSceneInstance> GetEditorSceneInstance() const { return mEditorSceneInstance.lock(); }
#endif

		/** Removes all scene objects from the scene, except for persistent objects. If @p forceAll is true, removes even the persistent objects. */
		B3D_SCRIPT_EXPORT()
		void Clear(bool forceAll = false);

		/**
		 * Creates a new scene object in the scene instance.
		 * 
		 * @param	name	Name of the scene object.
		 * @param	flags	Optional flags that control object behavior. See SceneObjectFlags.
		 */
		B3D_SCRIPT_EXPORT()
		HSceneObject CreateSceneObject(const String& name, u32 flags = 0);

		/** Creates a new empty scene instance. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SceneInstance))
		static SPtr<SceneInstance> Create(const String& name);

		/** Creates a new scene instance with an existing hierarchy. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(SceneInstance))
		static SPtr<SceneInstance> Create(const String& name, const HSceneObject& root);

		/** Creates a new scene instance with an existing hierarchy and associated resource ID. */
		static SPtr<SceneInstance> Create(const String& name, const HSceneObject& root, const UUID& associatedResourceId);

		/**
		 * @name Internal
		 * @{
		 */

		/** Returns the ECS registry for this scene. */
		ecs::Registry& GetECSRegistry() { return mGameObjectCollection->GetECSRegistry(); }

		/** @copydoc GetECSRegistry() */
		const ecs::Registry& GetECSRegistry() const { return mGameObjectCollection->GetECSRegistry(); }

		/** Returns the game object collection storing all the scene's game objects. */
		const SPtr<GameObjectCollection>& GetGameObjectCollection() const { return mGameObjectCollection; }

		/** Sets the ID of the resource that the scene instance is associated with (e.g. resource the scene was loaded from.). */
		void SetAssociatedResourceId(const UUID& id) { mAssociatedResourceId = id; }

		/**
		 * Changes the root scene object. Any persistent objects will remain in the scene, now parented to the new root. All non-persistent objects
		 * in the old root are destroyed.
		 */
		void SetRoot(const HSceneObject& newRoot);

		/** Called every frame before Update(). Calls FixedUpdate() methods on all active components and advances physics. */
		void FixedUpdate();

		/** Called every frame. Calls Update() methods on all active components. */
		void Update();

		/**	Notifies the scene instance that a new camera was created. */
		void RegisterCamera(const HCamera& camera);

		/**	Notifies the scene instance that a camera was removed. */
		void UnregisterCamera(const HCamera& camera);

		/**	Notifies the scene instance that a camera either became the main camera, or has stopped being main camera. */
		void NotifyMainCameraStateChanged(const HCamera& camera);

		/**
		 * Sets the render target that the main camera in the scene (if any) will render its view to. This generally means
		 * the main game window when running standalone, or the Game viewport when running in editor.
		 */
		void SetMainCameraRenderTarget(const SPtr<RenderTarget>& renderTarget);

#if B3D_WITH_EDITOR
		/** @copydoc GetEditorSceneInstance */
		void SetEditorSceneInstance(const SPtr<IEditorSceneInstance>& scene) { mEditorSceneInstance = scene; }
#endif

		/** @} */

	private:
		friend class SceneManager;

		SPtr<render::RenderProxy> CreateRenderProxy() const override;

		/**	Callback that is triggered when the main render target size is changed. */
		void OnMainRenderTargetResized();

		String mName;
		HSceneObject mRoot;
		UUID mAssociatedResourceId; /**< ID of the resource the scene was loaded from, if any. */
		bool mIsActive = true;
		SPtr<PhysicsScene> mPhysicsScene;
		SPtr<RendererScene> mRendererScene;
		SPtr<AnimationScene> mAnimationScene;
		SPtr<ParticleScene> mParticleScene;
		SPtr<GameObjectCollection> mGameObjectCollection;
		SceneTime mTime;

		UnorderedMap<UUID, HCamera> mCameras;
		Vector<HCamera> mMainCameras;

		SPtr<RenderTarget> mPrimaryRenderTarget;
		HEvent mMainRenderTargetResizedHandle;

#if B3D_WITH_EDITOR
		WeakSPtr<IEditorSceneInstance> mEditorSceneInstance;
#endif
	};

	namespace render
	{
		class RendererScene;

		/** @copydoc SceneInstance */
		class B3D_EXPORT SceneInstance : public RenderProxy
		{
		public:
			/**
			 * Representation of the scene used by the renderer. Contains all the objects that need to be rendered.
			 * Exact implementation depends on the renderer plugin used.
			 */
			const SPtr<RendererScene>& GetRendererScene() const { return mRendererScene; }

		protected:
			friend class b3d::SceneInstance;

			SceneInstance(u64 id, const SPtr<RendererScene>& rendererScene)
				: mId(id), mRendererScene(rendererScene)
			{ }

			u64 mId = ~0u;
			SPtr<RendererScene> mRendererScene;
		};
	} // namespace render

	/** @} */
} // namespace b3d
