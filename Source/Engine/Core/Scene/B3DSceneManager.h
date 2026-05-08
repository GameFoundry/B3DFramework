//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Scene/B3DGameObject.h"

namespace b3d
{
	class ParticleScene;
	class RendererScene;
	class LightProbeVolume;
	class PhysicsScene;

	/** @addtogroup Scene
	 *  @{
	 */

	/**
	 * Keeps track of all active SceneObject%s and their components. Keeps track of component state and triggers their
	 * events. Updates the transforms of objects as SceneObject%s move.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT() SceneManager : public Module<SceneManager>
	{
	public:
		SceneManager() = default;
		~SceneManager();

		void OnStartUp() override;

		/**
		 * In a standalone game this represents the scene that is playing. In editor this represents the primary scene being edited.
		 * When creating scene objects and no scene is provided, the object will be created in the main scene. If null main scene is set, new empty main
		 * scene will be created internally, as the main scene must always exist.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(MainScene))
		void SetMainScene(const TShared<SceneInstance>& scene);

		/** @copydoc SetMainScene */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(MainScene))
		const TShared<SceneInstance>& GetMainScene() const { return mMainScene; }

		/** Returns all live scene instances. */
		const UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>>& GetAllScenes() const { return mSceneInstances; }

		/**
		 * Sets the render target that the main camera in the scene (if any) will render its view to. This generally means
		 * the main game window when running standalone, or the Game viewport when running in editor.
		 */
		void SetMainCameraRenderTarget(const TShared<RenderTarget>& renderTarget);

		/** Notifies the manager that a new scene instance was created. */
		void NotifySceneInstanceCreated(const TShared<SceneInstance>& sceneInstance);

		/** Notifies the manager that a scene instance was destroyed. */
		void NotifySceneInstanceDestroyed(SceneInstance* sceneInstance);

	protected:
		friend class SceneObject;

		TShared<SceneInstance> mMainScene;
		UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>> mSceneInstances;
	};

	/**	Provides easy access to the SceneManager. */
	B3D_EXPORT SceneManager& GetSceneManager();

	/** @} */
} // namespace b3d
