//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Scene/BsGameObject.h"

namespace bs
{
	class LightProbeVolume;

	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/** Information about a scene actor and the scene object it has been bound to. */
	struct BoundActorData
	{
		BoundActorData() { }

		BoundActorData(const SPtr<SceneActor>& actor, const HSceneObject& so)
			:actor(actor), so(so)
		{ }

		SPtr<SceneActor> actor;
		HSceneObject so;
	};

	/** Possible states components can be in. Controls which component callbacks are triggered. */
	enum class ComponentState
	{
		Running, /**< All components callbacks are being triggered normally. */
		Paused, /**< All component callbacks except update are being triggered normally. */
		Stopped /**< No component callbacks are being triggered. */
	};

	/** 
	 * Keeps track of all active SceneObject%s and their components. Keeps track of component state and triggers their
	 * events. Updates the transforms of objects as SceneObject%s move.
	 */
	class BS_CORE_EXPORT SceneManager : public Module<SceneManager>
	{
	public:
		SceneManager();
		~SceneManager();

		/**	Returns the root scene object. */
		HSceneObject getRootNode() const { return mRootNode; }

		/**
		 * Destroys all scene objects in the scene.
		 *
		 * @param[in]	forceAll	If true, then even the persistent objects will be unloaded.
		 */
		void clearScene(bool forceAll = false);

		/** Changes the root scene object. Any persistent objects will remain in the scene, now parented to the new root. */
		void setRootNode(const HSceneObject& root);

		/** 
		 * Changes the component state that globally determines which component callbacks are activated. Only affects
		 * components that don't have the ComponentFlag::AlwaysRun flag set. 
		 */
		void setComponentState(ComponentState state);

		/** Checks are the components currently in the Running state. */
		bool isRunning() const { return mComponentState == ComponentState::Running; }

		/** 
		 * Returns a list of all components of the specified type currently in the scene. 
		 *
		 * @tparam		T			Type of the component to search for.
		 * 
		 * @param[in]	activeOnly	If true only active components are returned, otherwise all components are returned.
		 * @return					A list of all matching components in the scene.
		 */
		template<class T>
		Vector<GameObjectHandle<T>> findComponents(bool activeOnly = true);

		/** Returns all cameras in the scene. */
		const UnorderedMap<Camera*, SPtr<Camera>>& getAllCameras() const { return mCameras; }

		/**
		 * Returns the camera in the scene marked as main. Main camera controls the final render surface that is displayed
		 * to the user. If there are multiple main cameras, the first one found returned.
		 */
		SPtr<Camera> getMainCamera() const;

		/**
		 * Sets the render target that the main camera in the scene (if any) will render its view to. This generally means
		 * the main game window when running standalone, or the Game viewport when running in editor.
		 */
		void setMainRenderTarget(const SPtr<RenderTarget>& rt);

		/** 
		 * Binds a scene actor with a scene object. Every frame the scene object's transform will be monitored for
		 * changes and those changes will be automatically transfered to the actor. 
		 */
		void _bindActor(const SPtr<SceneActor>& actor, const HSceneObject& so);

		/** Unbinds an actor that was previously bound using bindActor(). */
		void _unbindActor(const SPtr<SceneActor>& actor);

		/** Returns a scene object bound to the provided actor, if any. */
		HSceneObject _getActorSO(const SPtr<SceneActor>& actor) const;

		/**	Notifies the scene manager that a new camera was created. */
		void _registerCamera(const SPtr<Camera>& camera);

		/**	Notifies the scene manager that a camera was removed. */
		void _unregisterCamera(const SPtr<Camera>& camera);

		/**	Notifies the scene manager that a camera either became the main camera, or has stopped being main camera. */
		void _notifyMainCameraStateChanged(const SPtr<Camera>& camera);

		/** Called every frame. Calls update methods on all scene objects and their components. */
		void _update();

		/** Called at fixed time internals. Calls the fixed update method on all active components. */
		void _fixedUpdate();

		/** Updates dirty transforms on any core objects that may be tied with scene objects. */
		void _updateCoreObjectTransforms();

		/** Notifies the manager that a new component has just been created. The manager triggers necessary callbacks. */
		void _notifyComponentCreated(const HComponent& component, bool parentActive);

		/** 
		 * Notifies the manager that a scene object the component belongs to was activated. The manager triggers necessary
		 * callbacks. 
		 */
		void _notifyComponentActivated(const HComponent& component, bool triggerEvent);

		/** 
		 * Notifies the manager that a scene object the component belongs to was deactivated. The manager triggers necessary
		 * callbacks. 
		 */
		void _notifyComponentDeactivated(const HComponent& component, bool triggerEvent);

		/** Notifies the manager that a component is about to be destroyed. The manager triggers necessary callbacks. */
		void _notifyComponentDestroyed(const HComponent& component, bool immediate);

	protected:
		/** Types of events that represent component state changes relevant to the scene manager. */
		enum class ComponentStateEventType
		{
			Created, Activated, Deactivated, Destroyed
		};

		/** Describes a single component state change. */
		struct ComponentStateChange
		{
			ComponentStateChange(HComponent obj, ComponentStateEventType type)
				:obj(std::move(obj)), type(type)
			{ }

			HComponent obj;
			ComponentStateEventType type;
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
		void registerNewSO(const HSceneObject& node);

		/**	Callback that is triggered when the main render target size is changed. */
		void onMainRenderTargetResized();

		/** 
		 * Adds a component to the specified state list. Caller is expected to first remove the component from any
		 * existing state lists.
		 */
		void addToStateList(const HComponent& component, UINT32 listType);

		/** Removes a component from its current scene manager state list (if any). */
		void removeFromStateList(const HComponent& component);

		/** Iterates over components that had their state modified and moves them to the appropriate state lists. */
		void processStateChanges();

		/** 
		 * Encodes an index and a type into a single 32-bit integer. Top 2 bits represent the type, while the rest represent
		 * the index.
		 */
		static UINT32 encodeComponentId(UINT32 idx, UINT32 type);

		/** Decodes an id encoded with encodeComponentId(). */
		static void decodeComponentId(UINT32 id, UINT32& idx, UINT32& type);

		/** Checks does the specified component type match the provided RTTI id. */
		static bool isComponentOfType(const HComponent& component, UINT32 rttiId);

	protected:
		HSceneObject mRootNode;

		UnorderedMap<SceneActor*, BoundActorData> mBoundActors;
		UnorderedMap<Camera*, SPtr<Camera>> mCameras;
		Vector<SPtr<Camera>> mMainCameras;

		Vector<HComponent> mActiveComponents;
		Vector<HComponent> mInactiveComponents;
		Vector<HComponent> mUninitializedComponents;

		std::array<Vector<HComponent>*, 3> mComponentsPerState = 
			{ { &mActiveComponents, &mInactiveComponents, &mUninitializedComponents } };

		SPtr<RenderTarget> mMainRT;
		HEvent mMainRTResizedConn;

		ComponentState mComponentState = ComponentState::Running;
		bool mDisableStateChange = false;
		Vector<ComponentStateChange> mStateChanges;
	};

	/**	Provides easy access to the SceneManager. */
	BS_CORE_EXPORT SceneManager& gSceneManager();

	template<class T>
	Vector<GameObjectHandle<T>> SceneManager::findComponents(bool activeOnly)
	{
		UINT32 rttiId = T::getRTTIStatic()->getRTTIId();

		Vector<GameObjectHandle<T>> output;
		for(auto& entry : mActiveComponents)
		{
			if (isComponentOfType(entry, rttiId))
				output.push_back(static_object_cast<T>(entry));
		}

		if(!activeOnly)
		{
			for(auto& entry : mInactiveComponents)
			{
				if (isComponentOfType(entry, rttiId))
					output.push_back(static_object_cast<T>(entry));
			}
				
			for(auto& entry : mUninitializedComponents)
			{
				if (isComponentOfType(entry, rttiId))
					output.push_back(static_object_cast<T>(entry));
			}
		}

		return output;
	}

	/** @} */
}
