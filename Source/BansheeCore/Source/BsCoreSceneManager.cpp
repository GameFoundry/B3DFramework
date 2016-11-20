//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCoreSceneManager.h"
#include "BsSceneObject.h"
#include "BsComponent.h"
#include "BsCamera.h"
#include "BsViewport.h"
#include "BsGameObjectManager.h"
#include "BsRenderTarget.h"

namespace bs
{
	std::function<void()> SceneManagerFactory::mFactoryMethod;

	CoreSceneManager::CoreSceneManager()
	{
		mRootNode = SceneObject::createInternal("SceneRoot");
	}

	CoreSceneManager::~CoreSceneManager()
	{
		if (mRootNode != nullptr && !mRootNode.isDestroyed())
			mRootNode->destroy(true);
	}

	void CoreSceneManager::clearScene(bool forceAll)
	{
		UINT32 numChildren = mRootNode->getNumChildren();

		UINT32 curIdx = 0;
		for (UINT32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = mRootNode->getChild(curIdx);

			if (forceAll || !child->hasFlag(SOF_Persistent))
				child->destroy();
			else
				curIdx++;
		}

		GameObjectManager::instance().destroyQueuedObjects();

		HSceneObject newRoot = SceneObject::createInternal("SceneRoot");
		_setRootNode(newRoot);
	}

	void CoreSceneManager::_setRootNode(const HSceneObject& root)
	{
		if (root == nullptr)
			return;

		HSceneObject oldRoot = mRootNode;

		UINT32 numChildren = oldRoot->getNumChildren();
		// Make sure to keep persistent objects

		bs_frame_mark();
		{
			FrameVector<HSceneObject> toRemove;
			for (UINT32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = oldRoot->getChild(i);

				if (child->hasFlag(SOF_Persistent))
					toRemove.push_back(child);
			}

			for (auto& entry : toRemove)
				entry->setParent(root, false);
		}
		bs_frame_clear();

		mRootNode = root;
		mRootNode->_setParent(HSceneObject());

		oldRoot->destroy();
	}

	void CoreSceneManager::_registerCamera(const SPtr<Camera>& camera, const HSceneObject& so)
	{
		mCameras[camera.get()] = SceneCameraData(camera, so);
	}

	void CoreSceneManager::_unregisterCamera(const SPtr<Camera>& camera)
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

	void CoreSceneManager::_notifyMainCameraStateChanged(const SPtr<Camera>& camera)
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

	SceneCameraData CoreSceneManager::getMainCamera() const
	{
		if (mMainCameras.size() > 0)
			return mMainCameras[0];

		return SceneCameraData();
	}

	void CoreSceneManager::setMainRenderTarget(const SPtr<RenderTarget>& rt)
	{
		if (mMainRT == rt)
			return;

		mMainRTResizedConn.disconnect();

		if (rt != nullptr)
			mMainRTResizedConn = rt->onResized.connect(std::bind(&CoreSceneManager::onMainRenderTargetResized, this));

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

	void CoreSceneManager::_update()
	{
		Stack<HSceneObject> todo;
		todo.push(mRootNode);

		while(!todo.empty())
		{
			HSceneObject currentGO = todo.top();
			todo.pop();

			if (!currentGO->getActive(true))
				continue;
			                  
			const Vector<HComponent>& components = currentGO->getComponents();

			for(auto iter = components.begin(); iter != components.end(); ++iter)
			{
				(*iter)->update();
			}

			for(UINT32 i = 0; i < currentGO->getNumChildren(); i++)
				todo.push(currentGO->getChild(i));
		}

		GameObjectManager::instance().destroyQueuedObjects();
	}

	void CoreSceneManager::registerNewSO(const HSceneObject& node) 
	{ 
		if(mRootNode)
			node->setParent(mRootNode);
	}

	void CoreSceneManager::onMainRenderTargetResized()
	{
		auto& rtProps = mMainRT->getProperties();
		float aspect = rtProps.getWidth() / (float)rtProps.getHeight();

		for (auto& entry : mMainCameras)
			entry.camera->setAspectRatio(aspect);
	}

	CoreSceneManager& gCoreSceneManager()
	{
		return CoreSceneManager::instance();
	}

	void SceneManagerFactory::create()
	{
		if (mFactoryMethod != nullptr)
			mFactoryMethod();
		else
			BS_EXCEPT(InvalidStateException, "Failed to initialize scene manager because no valid factory method is set.");
	}
}