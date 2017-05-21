//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSceneManager.h"
#include "BsSceneObject.h"
#include "BsComponent.h"
#include "BsRenderable.h"
#include "BsCamera.h"
#include "BsLight.h"
#include "BsReflectionProbe.h"
#include "BsViewport.h"
#include "BsGameObjectManager.h"
#include "BsRenderTarget.h"

namespace bs
{
	enum ListType
	{
		ActiveList = 0,
		InactiveList = 1,
		UninitializedList = 2
	};

	SceneManager::SceneManager()
	{
		mRootNode = SceneObject::createInternal("SceneRoot");
	}

	SceneManager::~SceneManager()
	{
		if (mRootNode != nullptr && !mRootNode.isDestroyed())
			mRootNode->destroy(true);
	}

	void SceneManager::clearScene(bool forceAll)
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

	void SceneManager::_setRootNode(const HSceneObject& root)
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

	void SceneManager::_registerReflectionProbe(const SPtr<ReflectionProbe>& probe, const HSceneObject& so)
	{
		mReflectionProbes[probe.get()] = SceneReflectionProbeData(probe, so);
	}

	void SceneManager::_unregisterReflectionProbe(const SPtr<ReflectionProbe>& probe)
	{
		mReflectionProbes.erase(probe.get());
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

	void SceneManager::_updateCoreObjectTransforms()
	{
		for (auto& renderablePair : mRenderables)
		{
			SPtr<Renderable> renderable = renderablePair.second.renderable;
			HSceneObject so = renderablePair.second.sceneObject;

			if (so->getMobility() != renderable->getMobility())
				renderable->setMobility(so->getMobility());

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

			if (so->getMobility() != handler->getMobility())
				handler->setMobility(so->getMobility());

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

		for (auto& probePair : mReflectionProbes)
		{
			SPtr<ReflectionProbe> probe = probePair.second.probe;
			HSceneObject so = probePair.second.sceneObject;

			UINT32 curHash = so->getTransformHash();
			if (curHash != probe->_getLastModifiedHash())
			{
				probe->setPosition(so->getWorldPosition());
				probe->setRotation(so->getWorldRotation());

				probe->_setLastModifiedHash(curHash);
			}

			if (so->getActive() != probe->getIsActive())
			{
				probe->setIsActive(so->getActive());
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

	void SceneManager::setComponentState(ComponentState state)
	{
		if (mComponentState == state)
			return;

		// Wake up all components with onInitialize/onEnable events if moving to running or paused state
		if(state == ComponentState::Running || state == ComponentState::Paused)
		{
			if(mComponentState == ComponentState::Stopped)
			{
				// Trigger enable on all components that don't have AlwaysRun flag (at this point those will be all
				// inactive components that have active scene object parents)
				for(auto& entry : mInactiveComponents)
				{
					if (entry->sceneObject()->getActive())
						entry->onEnabled();
				}

				// Initialize and enable uninitialized components
				for(auto& entry : mUnintializedComponents)
				{
					entry->onInitialized();

					if (entry->sceneObject()->getActive())
					{
						entry->onEnabled();

						UINT32 idx = (UINT32)mActiveComponents.size();
						mActiveComponents.push_back(entry);

						entry->setSceneManagerId(encodeComponentId(idx, ActiveList));
					}
					else
					{
						UINT32 idx = (UINT32)mInactiveComponents.size();
						mInactiveComponents.push_back(entry);

						entry->setSceneManagerId(encodeComponentId(idx, InactiveList));
					}
				}

				mUnintializedComponents.clear();
			}
		}

		// Start updates on all active components
		if (state == ComponentState::Running)
		{
			// Move from inactive to active list
			for(INT32 i = 0; i < (INT32)mInactiveComponents.size(); i++)
			{
				HComponent component = mInactiveComponents[i];
				if (!component->sceneObject()->getActive())
					continue;
				
				removeFromInactiveList(component);
				i--; // Keep the same index next iteration to process the component we just swapped

				UINT32 activeIdx = (UINT32)mActiveComponents.size();
				mActiveComponents.push_back(component);

				component->setSceneManagerId(encodeComponentId(activeIdx, ActiveList));
			}
		}
		// Stop updates on all active components
		else if(state == ComponentState::Paused || state == ComponentState::Stopped)
		{
			// Trigger onDisable events if stopping
			if (state == ComponentState::Stopped)
			{
				for (INT32 i = 0; i < (INT32)mActiveComponents.size(); i++)
				{
					HComponent component = mActiveComponents[i];

					bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
					if (alwaysRun)
						continue;

					component->onDisabled();
				}
			}

			// Move from active to inactive list
			for (INT32 i = 0; i < (INT32)mActiveComponents.size(); i++)
			{
				HComponent component = mActiveComponents[i];

				bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
				if (alwaysRun)
					continue;

				removeFromActiveList(component);
				i--; // Keep the same index next iteration to process the component we just swapped

				UINT32 inactiveIdx = (UINT32)mInactiveComponents.size();
				mInactiveComponents.push_back(component);

				component->setSceneManagerId(encodeComponentId(inactiveIdx, InactiveList));
			}
		}

		mComponentState = state;
	}

	void SceneManager::_notifyComponentCreated(const HComponent& component, bool parentActive)
	{
		component->onCreated();
		
		bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
		if(alwaysRun || mComponentState != ComponentState::Stopped)
		{
			component->onInitialized();

			if (parentActive)
			{
				component->onEnabled();

				UINT32 idx = (UINT32)mActiveComponents.size();
				mActiveComponents.push_back(component);

				component->setSceneManagerId(encodeComponentId(idx, ActiveList));
			}
			else
			{
				UINT32 idx = (UINT32)mInactiveComponents.size();
				mInactiveComponents.push_back(component);

				component->setSceneManagerId(encodeComponentId(idx, InactiveList));
			}
		}
		else // Stopped
		{
			UINT32 idx = (UINT32)mUnintializedComponents.size();
			mUnintializedComponents.push_back(component);

			component->setSceneManagerId(encodeComponentId(idx, UninitializedList));
		}
	}

	void SceneManager::_notifyComponentActivated(const HComponent& component, bool triggerEvent)
	{
		bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);

		if (alwaysRun || mComponentState == ComponentState::Running || mComponentState == ComponentState::Paused)
		{
			if (triggerEvent)
				component->onEnabled();

			removeFromInactiveList(component);

			UINT32 activeIdx = (UINT32)mActiveComponents.size();
			mActiveComponents.push_back(component);

			component->setSceneManagerId(encodeComponentId(activeIdx, ActiveList));
		}
	}

	void SceneManager::_notifyComponentDeactivated(const HComponent& component, bool triggerEvent)
	{
		bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);

		if (alwaysRun || mComponentState == ComponentState::Running || mComponentState == ComponentState::Paused)
		{
			if (triggerEvent)
				component->onDisabled();

			removeFromActiveList(component);

			UINT32 inactiveIdx = (UINT32)mInactiveComponents.size();
			mInactiveComponents.push_back(component);

			component->setSceneManagerId(encodeComponentId(inactiveIdx, InactiveList));
		}
	}

	void SceneManager::_notifyComponentDestroyed(const HComponent& component)
	{
		UINT32 listType;
		UINT32 idx;
		decodeComponentId(component->getSceneManagerId(), idx, listType);

		switch(listType)
		{
		case ActiveList:
			removeFromActiveList(component);
			break;
		case InactiveList:
			removeFromInactiveList(component);
			break;
		case UninitializedList:
			removeFromUninitializedList(component);
			break;
		default:
			assert(false);
			break;
		}

		bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
		bool isEnabled = component->sceneObject()->getActive() && (alwaysRun || mComponentState != ComponentState::Stopped);

		if (isEnabled)
			component->onDisabled();
		
		component->onDestroyed();
	}

	void SceneManager::removeFromActiveList(const HComponent& component)
	{
		UINT32 listType;
		UINT32 idx;
		decodeComponentId(component->getSceneManagerId(), idx, listType);

		UINT32 lastIdx;
		decodeComponentId(mActiveComponents.back()->getSceneManagerId(), lastIdx, listType);

		assert(mActiveComponents[idx] == component);

		if (idx != lastIdx)
		{
			std::swap(mActiveComponents[idx], mActiveComponents[lastIdx]);
			mActiveComponents[idx]->setSceneManagerId(encodeComponentId(idx, ActiveList));
		}

		mActiveComponents.erase(mActiveComponents.end() - 1);
	}

	void SceneManager::removeFromInactiveList(const HComponent& component)
	{
		UINT32 listType;
		UINT32 idx;
		decodeComponentId(component->getSceneManagerId(), idx, listType);

		UINT32 lastIdx;
		decodeComponentId(mInactiveComponents.back()->getSceneManagerId(), lastIdx, listType);

		assert(mInactiveComponents[idx] == component);

		if (idx != lastIdx)
		{
			std::swap(mInactiveComponents[idx], mInactiveComponents[lastIdx]);
			mInactiveComponents[idx]->setSceneManagerId(encodeComponentId(idx, InactiveList));
		}

		mInactiveComponents.erase(mInactiveComponents.end() - 1);
	}

	void SceneManager::removeFromUninitializedList(const HComponent& component)
	{
		UINT32 listType;
		UINT32 idx;
		decodeComponentId(component->getSceneManagerId(), idx, listType);

		UINT32 lastIdx;
		decodeComponentId(mUnintializedComponents.back()->getSceneManagerId(), lastIdx, listType);

		assert(mUnintializedComponents[idx] == component);

		if (idx != lastIdx)
		{
			std::swap(mUnintializedComponents[idx], mUnintializedComponents[lastIdx]);
			mUnintializedComponents[idx]->setSceneManagerId(encodeComponentId(idx, UninitializedList));
		}

		mUnintializedComponents.erase(mUnintializedComponents.end() - 1);
	}

	UINT32 SceneManager::encodeComponentId(UINT32 idx, UINT32 type)
	{
		assert(idx <= (0x3FFFFFFF));

		return (type << 30) | idx;
	}

	void SceneManager::decodeComponentId(UINT32 id, UINT32& idx, UINT32& type)
	{
		idx = id & 0x3FFFFFFF;
		type = id >> 30;
	}

	void SceneManager::_update()
	{
		// Note: Eventually perform updates based on component types and/or on component priority. Right now we just
		// iterate in an undefined order, but it wouldn't be hard to change it.

		for (auto& entry : mActiveComponents)
			entry->update();

		GameObjectManager::instance().destroyQueuedObjects();
	}

	void SceneManager::registerNewSO(const HSceneObject& node)
	{ 
		if(mRootNode)
			node->setParent(mRootNode);
	}

	void SceneManager::onMainRenderTargetResized()
	{
		auto& rtProps = mMainRT->getProperties();
		float aspect = rtProps.getWidth() / (float)rtProps.getHeight();

		for (auto& entry : mMainCameras)
			entry.camera->setAspectRatio(aspect);
	}

	SceneManager& gSceneManager()
	{
		return SceneManager::instance();
	}
}