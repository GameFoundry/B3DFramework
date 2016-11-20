//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsCoreSceneManager.h"

namespace bs
{
	/** @addtogroup Renderer-Engine-Internal
	 *  @{
	 */

	/**	Contains information about a renderable managed by the scene manager. */
	struct SceneRenderableData
	{
		SceneRenderableData() { }

		SceneRenderableData(const SPtr<Renderable>& renderable, const HSceneObject& sceneObject)
			:renderable(renderable), sceneObject(sceneObject)
		{ }

		SPtr<Renderable> renderable;
		HSceneObject sceneObject;
	};

	/**	Contains information about a light managed by the scene manager. */
	struct SceneLightData
	{
		SceneLightData() { }

		SceneLightData(const SPtr<Light>& light, const HSceneObject& sceneObject)
			:light(light), sceneObject(sceneObject)
		{ }

		SPtr<Light> light;
		HSceneObject sceneObject;
	};

	/** Manages active SceneObjects and provides ways for querying and updating them or their components. */
	class BS_EXPORT SceneManager : public CoreSceneManager
	{
	public:
		struct InitOnStart
		{
		public:
			InitOnStart()
			{
				SceneManagerFactory::setFactoryMethod(&startUp);
			}

			static void startUp()
			{
				CoreSceneManager::startUp<SceneManager>();
			}
		};

		SceneManager() { }
		virtual ~SceneManager() {}

		/**	Returns all renderables in the scene. */
		const Map<Renderable*, SceneRenderableData>& getAllRenderables() const { return mRenderables; }

		/** Notifies the scene manager that a new renderable was created. */
		void _registerRenderable(const SPtr<Renderable>& renderable, const HSceneObject& so);

		/**	Notifies the scene manager that a renderable was removed. */
		void _unregisterRenderable(const SPtr<Renderable>& renderable);

		/**	Notifies the scene manager that a new light was created. */
		void _registerLight(const SPtr<Light>& light, const HSceneObject& so);

		/**	Notifies the scene manager that a light was removed. */
		void _unregisterLight(const SPtr<Light>& light);

		/** @copydoc CoreSceneManager::_updateCoreObjectTransforms */
		void _updateCoreObjectTransforms() override;

		/** @copydoc CoreSceneManager::instance */
		static SceneManager& instance();

		/** @copydoc CoreSceneManager::instancePtr */
		static SceneManager* instancePtr();

	private:
		Map<Renderable*, SceneRenderableData> mRenderables;
		Map<Light*, SceneLightData> mLights;

		volatile static InitOnStart DoInitOnStart;
	};

	/** Provides easier access to SceneManager. */
	BS_EXPORT SceneManager& gSceneManager();

	/** @} */
}