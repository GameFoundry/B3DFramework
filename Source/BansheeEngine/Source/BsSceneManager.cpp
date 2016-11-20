//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSceneManager.h"
#include "BsSceneObject.h"
#include "BsRenderable.h"
#include "BsCamera.h"
#include "BsLight.h"

namespace bs
{
	volatile SceneManager::InitOnStart SceneManager::DoInitOnStart;

	void SceneManager::_registerRenderable(const SPtr<Renderable>& renderable, const HSceneObject& so)
	{
		mRenderables[renderable.get()] = SceneRenderableData(renderable, so);
	}

	void SceneManager::_unregisterRenderable(const SPtr<Renderable>& renderable)
	{
		mRenderables.erase(renderable.get());
	}

	void SceneManager::_registerLight(const SPtr<Light>& light, const HSceneObject& so)
	{
		mLights[light.get()] = SceneLightData(light, so);
	}

	void SceneManager::_unregisterLight(const SPtr<Light>& light)
	{
		mLights.erase(light.get());
	}

	void SceneManager::_updateCoreObjectTransforms()
	{
		for (auto& renderablePair : mRenderables)
		{
			SPtr<Renderable> renderable = renderablePair.second.renderable;
			HSceneObject so = renderablePair.second.sceneObject;

			renderable->_updateTransform(so);

			if (so->getActive() != renderable->getIsActive())
				renderable->setIsActive(so->getActive());
		}

		for (auto& cameraPair : mCameras)
		{
			SPtr<Camera> handler = cameraPair.second.camera;
			HSceneObject so = cameraPair.second.sceneObject;

			UINT32 curHash = so->getTransformHash();
			if (curHash != handler->_getLastModifiedHash())
			{
				handler->setPosition(so->getWorldPosition());
				handler->setRotation(so->getWorldRotation());

				handler->_setLastModifiedHash(curHash);
			}

			if (so->getActive() != handler->getIsActive())
			{
				handler->setIsActive(so->getActive());
			}
		}

		for (auto& lightPair : mLights)
		{
			SPtr<Light> handler = lightPair.second.light;
			HSceneObject so = lightPair.second.sceneObject;

			UINT32 curHash = so->getTransformHash();
			if (curHash != handler->_getLastModifiedHash())
			{
				handler->setPosition(so->getWorldPosition());
				handler->setRotation(so->getWorldRotation());

				handler->_setLastModifiedHash(curHash);
			}

			if (so->getActive() != handler->getIsActive())
			{
				handler->setIsActive(so->getActive());
			}
		}
	}

	SceneManager& SceneManager::instance()
	{
		return static_cast<SceneManager&>(CoreSceneManager::instance());
	}

	SceneManager* SceneManager::instancePtr()
	{
		return static_cast<SceneManager*>(CoreSceneManager::instancePtr());
	}

	SceneManager& gSceneManager()
	{
		return static_cast<SceneManager&>(gCoreSceneManager());
	}
}
