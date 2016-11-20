//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"
#include "BsGameObject.h"

namespace bs
{
	/** @addtogroup Scene-Internal
	 *  @{
	 */

	/**	Contains information about a camera managed by the scene manager. */
	struct SceneCameraData
	{
		SceneCameraData() { }

		SceneCameraData(const SPtr<Camera>& camera, const HSceneObject& sceneObject)
			:camera(camera), sceneObject(sceneObject)
		{ }

		SPtr<Camera> camera;
		HSceneObject sceneObject;
	};

	/**
	 * Manages all objects in the scene and provides various query methods for finding objects. This is just the base class
	 * with basic query functionality. You should override it with your own version.
	 */
	class BS_CORE_EXPORT CoreSceneManager : public Module<CoreSceneManager>
	{
	public:
		CoreSceneManager();
		virtual ~CoreSceneManager();

		/**	Returns the root scene object. */
		HSceneObject getRootNode() const { return mRootNode; }

		/**
		 * Destroys all scene objects in the scene.
		 *
		 * @param[in]	forceAll	If true, then even the persistent objects will be unloaded.
		 */
		void clearScene(bool forceAll = false);

		/** Returns all cameras in the scene. */
		const Map<Camera*, SceneCameraData>& getAllCameras() const { return mCameras; }

		/**
		 * Returns the camera in the scene marked as main. Main camera controls the final render surface that is displayed
		 * to the user. If there are multiple main cameras, the first one found returned.
		 */
		SceneCameraData getMainCamera() const;

		/**
		 * Sets the render target that the main camera in the scene (if any) will render its view to. This generally means
		 * the main game window when running standalone, or the Game viewport when running in editor.
		 */
		void setMainRenderTarget(const SPtr<RenderTarget>& rt);

		/**	Notifies the scene manager that a new camera was created. */
		void _registerCamera(const SPtr<Camera>& camera, const HSceneObject& so);

		/**	Notifies the scene manager that a camera was removed. */
		void _unregisterCamera(const SPtr<Camera>& camera);

		/**	Notifies the scene manager that a camera either became the main camera, or has stopped being main camera. */
		void _notifyMainCameraStateChanged(const SPtr<Camera>& camera);

		/** Changes the root scene object. Any persistent objects will remain in the scene, now parented to the new root. */
		void _setRootNode(const HSceneObject& root);

		/** Called every frame. Calls update methods on all scene objects and their components. */
		virtual void _update();

		/** Updates dirty transforms on any core objects that may be tied with scene objects. */
		virtual void _updateCoreObjectTransforms() { }

	protected:
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

	protected:
		HSceneObject mRootNode;

		Map<Camera*, SceneCameraData> mCameras;
		Vector<SceneCameraData> mMainCameras;

		SPtr<RenderTarget> mMainRT;
		HEvent mMainRTResizedConn;
	};

	/**
	 * Handles creation of a scene manager.
	 *
	 * @note	
	 * Since scene manager implementations may vary it is expected that a concrete implementation of a scene manager will 
	 * register its creation method using setFactoryMethod() which will then later be used for creating the scene manager
	 * during application start up.
	 */
	class BS_CORE_EXPORT SceneManagerFactory
	{
	public:
		/**	Creates a concrete scene manager, depending on the currently set factory method. */
		static void create();

		/**	Sets method that will be used for creating the scene manager when create() gets called. */
		static void setFactoryMethod(const std::function<void()>& method)
		{
			mFactoryMethod = method;
		}

	private:
		static std::function<void()> mFactoryMethod;
	};

	/**	Provides easy access to the CoreSceneManager. */
	BS_CORE_EXPORT CoreSceneManager& gCoreSceneManager();

	/** @} */
}