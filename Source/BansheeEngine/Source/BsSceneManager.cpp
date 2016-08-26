//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSceneManager.h"
#include "BsSceneObject.h"
#include "BsRenderable.h"
#include "BsCamera.h"
#include "BsLight.h"
#include "BsRenderTarget.h"

namespace BansheeEngine
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

	void SceneManager::_registerCamera(const SPtr<Camera>& camera, const HSceneObject& so)
	{
		mCameras[camera.get()] = SceneCameraData(camera, so);
	}

	void SceneManager::_unregisterCamera(const SPtr<Camera>& camera)
	{
		mCameras.erase(camera.get());

		auto iterFind = std::find_if(mMainCameras.begin(), mMainCameras.end(), 
			[&](const SceneCameraData& x)
		{
			return x.camera == camera;
		});

		if (iterFind != mMainCameras.end())
			mMainCameras.erase(iterFind);
	}

	void SceneManager::_notifyMainCameraStateChanged(const SPtr<Camera>& camera)
	{
		auto iterFind = std::find_if(mMainCameras.begin(), mMainCameras.end(),
			[&](const SceneCameraData& entry)
		{
			return entry.camera == camera;
		});

		SPtr<Viewport> viewport = camera->getViewport();
		if (camera->isMain())
		{
			if (iterFind == mMainCameras.end())
				mMainCameras.push_back(mCameras[camera.get()]);

			viewport->setTarget(mMainRT);
		}
		else
		{
			if (iterFind != mMainCameras.end())
				mMainCameras.erase(iterFind);

			if (viewport->getTarget() == mMainRT)
				viewport->setTarget(nullptr);
		}
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

	SceneCameraData SceneManager::getMainCamera() const
	{
		if (mMainCameras.size() > 0)
			return mMainCameras[0];

		return SceneCameraData();
	}

	void SceneManager::setMainRenderTarget(const SPtr<RenderTarget>& rt)
	{
		if (mMainRT == rt)
			return;

		mMainRTResizedConn.disconnect();

		if (rt != nullptr)
			mMainRTResizedConn = rt->onResized.connect(std::bind(&SceneManager::onMainRenderTargetResized, this));
		
		mMainRT = rt;

		float aspect = 1.0f;
		if (rt != nullptr)
		{
			auto& rtProps = rt->getProperties();
			aspect = rtProps.getWidth() / (float)rtProps.getHeight();
		}

		for (auto& entry : mMainCameras)
		{
			entry.camera->getViewport()->setTarget(rt);
			entry.camera->setAspectRatio(aspect);
		}
	}

	void SceneManager::onMainRenderTargetResized()
	{
		auto& rtProps = mMainRT->getProperties();
		float aspect = rtProps.getWidth() / (float)rtProps.getHeight();

		for (auto& entry : mMainCameras)
			entry.camera->setAspectRatio(aspect);
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
