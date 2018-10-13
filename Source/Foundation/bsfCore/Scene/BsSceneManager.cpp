//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsSceneManager.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsComponent.h"
#include "Renderer/BsRenderable.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsLight.h"
#include "RenderAPI/BsViewport.h"
#include "Scene/BsGameObjectManager.h"
#include "RenderAPI/BsRenderTarget.h"
#include "Renderer/BsLightProbeVolume.h"
#include "Scene/BsSceneActor.h"

namespace bs
{
	enum ListType
	{
		NoList = 0,
		ActiveList = 1,
		InactiveList = 2,
		UninitializedList = 3
	};

	struct ScopeToggle
	{
		ScopeToggle(bool& val) :val(val) { val = true; }
		~ScopeToggle() { val = false;}

	private:
		bool& val;
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
		setRootNode(newRoot);
	}

	void SceneManager::setRootNode(const HSceneObject& root)
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

	void SceneManager::_bindActor(const SPtr<SceneActor>& actor, const HSceneObject& so)
	{
		mBoundActors[actor.get()] = BoundActorData(actor, so);
	}

	void SceneManager::_unbindActor(const SPtr<SceneActor>& actor)
	{
		mBoundActors.erase(actor.get());
	}

	HSceneObject SceneManager::_getActorSO(const SPtr<SceneActor>& actor) const
	{
		auto iterFind = mBoundActors.find(actor.get());
		if (iterFind != mBoundActors.end())
			return iterFind->second.so;

		return HSceneObject();		
	}

	void SceneManager::_registerCamera(const SPtr<Camera>& camera)
	{
		mCameras[camera.get()] = camera;
	}

	void SceneManager::_unregisterCamera(const SPtr<Camera>& camera)
	{
		mCameras.erase(camera.get());

		auto iterFind = std::find_if(mMainCameras.begin(), mMainCameras.end(),
			[&](const SPtr<Camera>& x)
		{
			return x == camera;
		});

		if (iterFind != mMainCameras.end())
			mMainCameras.erase(iterFind);
	}

	void SceneManager::_notifyMainCameraStateChanged(const SPtr<Camera>& camera)
	{
		auto iterFind = std::find_if(mMainCameras.begin(), mMainCameras.end(),
			[&](const SPtr<Camera>& entry)
		{
			return entry == camera;
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
		for (auto& entry : mBoundActors)
			entry.second.actor->_updateState(*entry.second.so);
	}

	SPtr<Camera> SceneManager::getMainCamera() const
	{
		if (mMainCameras.size() > 0)
			return mMainCameras[0];

		return nullptr;
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
			aspect = rtProps.width / (float)rtProps.height;
		}

		for (auto& entry : mMainCameras)
		{
			entry->getViewport()->setTarget(rt);
			entry->setAspectRatio(aspect);
		}
	}

	void SceneManager::setComponentState(ComponentState state)
	{
		if(mDisableStateChange)
		{
			LOGWRN("Component state cannot be changed from the calling locating. Are you calling it from Component \
				callbacks?");
			return;
		}

		if (mComponentState == state)
			return;

		ComponentState oldState = mComponentState;

		// Make sure to change the state before calling any callbacks, so callbacks can query the state
		mComponentState = state;

		// Make sure the per-state lists are up-to-date
		processStateChanges();

		ScopeToggle toggle(mDisableStateChange);

		// Wake up all components with onInitialize/onEnable events if moving to running or paused state
		if(state == ComponentState::Running || state == ComponentState::Paused)
		{
			if(oldState == ComponentState::Stopped)
			{
				// Disable, and then re-enable components that have an AlwaysRun flag
				for(auto& entry : mActiveComponents)
				{
					if (entry->sceneObject()->getActive())
					{
						entry->onDisabled();
						entry->onEnabled();
					}
				}

				// Process any state changes queued by the component callbacks
				processStateChanges();

				// Trigger enable on all components that don't have AlwaysRun flag (at this point those will be all
				// inactive components that have active scene object parents)
				for(auto& entry : mInactiveComponents)
				{
					if (entry->sceneObject()->getActive())
					{
						entry->onEnabled();

						if(state == ComponentState::Running)
							mStateChanges.emplace_back(entry, ComponentStateEventType::Activated);
					}
				}

				// Process any state changes queued by the component callbacks
				processStateChanges();

				// Initialize and enable uninitialized components
				for(auto& entry : mUninitializedComponents)
				{
					entry->onInitialized();

					if (entry->sceneObject()->getActive())
					{
						entry->onEnabled();
						mStateChanges.emplace_back(entry, ComponentStateEventType::Activated);
					}
					else
						mStateChanges.emplace_back(entry, ComponentStateEventType::Deactivated);
				}

				// Process any state changes queued by the component callbacks
				processStateChanges();
			}
		}

		// Stop updates on all active components
		if(state == ComponentState::Paused || state == ComponentState::Stopped)
		{
			// Trigger onDisable events if stopping
			if (state == ComponentState::Stopped)
			{
				for (const auto& component : mActiveComponents)
				{
					const bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);

					component->onDisabled();

					if(alwaysRun)
						component->onEnabled();
				}
			}

			// Move from active to inactive list
			for (INT32 i = 0; i < (INT32)mActiveComponents.size(); i++)
			{
				// Note: Purposely not a reference since the list changes in the add/remove methods below
				const HComponent component = mActiveComponents[i];

				const bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
				if (alwaysRun)
					continue;

				removeFromStateList(component);
				addToStateList(component, InactiveList);

				i--; // Keep the same index next iteration to process the component we just swapped
			}
		}
	}

	void SceneManager::_notifyComponentCreated(const HComponent& component, bool parentActive)
	{
		// Note: This method must remain reentrant (in case the callbacks below trigger component state changes)

		// Queue the change before any callbacks trigger, as the callbacks could trigger their own changes and they should
		// be in order
		mStateChanges.emplace_back(component, ComponentStateEventType::Created);
		ScopeToggle toggle(mDisableStateChange);
		
		component->onCreated();
		
		const bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
		if(alwaysRun || mComponentState != ComponentState::Stopped)
		{
			component->onInitialized();

			if (parentActive)
				component->onEnabled();
		}
	}

	void SceneManager::_notifyComponentActivated(const HComponent& component, bool triggerEvent)
	{
		// Note: This method must remain reentrant (in case the callbacks below trigger component state changes)

		// Queue the change before any callbacks trigger, as the callbacks could trigger their own changes and they should
		// be in order
		mStateChanges.emplace_back(component, ComponentStateEventType::Activated);
		ScopeToggle toggle(mDisableStateChange);

		const bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
		if(alwaysRun || mComponentState != ComponentState::Stopped)
		{
			if (triggerEvent)
				component->onEnabled();
		}
	}

	void SceneManager::_notifyComponentDeactivated(const HComponent& component, bool triggerEvent)
	{
		// Note: This method must remain reentrant (in case the callbacks below trigger component state changes)

		// Queue the change before any callbacks trigger, as the callbacks could trigger their own changes and they should
		// be in order
		mStateChanges.emplace_back(component, ComponentStateEventType::Deactivated);
		ScopeToggle toggle(mDisableStateChange);

		const bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
		if(alwaysRun || mComponentState != ComponentState::Stopped)
		{
			if (triggerEvent)
				component->onDisabled();
		}
	}

	void SceneManager::_notifyComponentDestroyed(const HComponent& component, bool immediate)
	{
		// Note: This method must remain reentrant (in case the callbacks below trigger component state changes)

		// Queue the change before any callbacks trigger, as the callbacks could trigger their own changes and they should
		// be in order
		if(!immediate)
		{
			// If destruction is immediate no point in queuing state change since it will be ignored anyway
			mStateChanges.emplace_back(component, ComponentStateEventType::Destroyed);
		}

		ScopeToggle toggle(mDisableStateChange);

		const bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
		const bool isEnabled = component->sceneObject()->getActive() && (alwaysRun || 
			mComponentState != ComponentState::Stopped);

		if (isEnabled)
			component->onDisabled();
		
		component->onDestroyed();

		if(immediate)
		{
			// Since the state change wasn't queued, remove the component from the list right away. Its expected the caller
			// knows what is he doing.

			UINT32 existingListType;
			UINT32 existingIdx;
			decodeComponentId(component->getSceneManagerId(), existingIdx, existingListType);

			if(existingListType != 0)
				removeFromStateList(component);
		}
	}

	void SceneManager::addToStateList(const HComponent& component, UINT32 listType)
	{
		if(listType == 0)
			return;

		Vector<HComponent>& list = *mComponentsPerState[listType - 1];

		const auto idx = (UINT32)list.size();
		list.push_back(component);

		component->setSceneManagerId(encodeComponentId(idx, listType));
	}

	void SceneManager::removeFromStateList(const HComponent& component)
	{
		UINT32 listType;
		UINT32 idx;
		decodeComponentId(component->getSceneManagerId(), idx, listType);

		if(listType == 0)
			return;

		Vector<HComponent>& list = *mComponentsPerState[listType - 1];

		UINT32 lastIdx;
		decodeComponentId(list.back()->getSceneManagerId(), lastIdx, listType);

		assert(list[idx] == component);

		if (idx != lastIdx)
		{
			std::swap(list[idx], list[lastIdx]);
			list[idx]->setSceneManagerId(encodeComponentId(idx, listType));
		}

		list.erase(list.end() - 1);
	}

	void SceneManager::processStateChanges()
	{
		const bool isStopped = mComponentState == ComponentState::Stopped;

		for(auto& entry : mStateChanges)
		{
			const HComponent& component = entry.obj;
			if(component.isDestroyed(false))
				continue;

			const bool alwaysRun = component->hasFlag(ComponentFlag::AlwaysRun);
			const bool isActive = component->SO()->getActive();

			UINT32 listType = 0;
			switch(entry.type)
			{
			case ComponentStateEventType::Created: 
				if (alwaysRun || !isStopped)
					listType = isActive ? ActiveList : InactiveList;
				else
					listType = UninitializedList;
				break;
			case ComponentStateEventType::Activated:
			case ComponentStateEventType::Deactivated:
				if (alwaysRun || !isStopped)
					listType = isActive ? ActiveList : InactiveList;
				break;
			case ComponentStateEventType::Destroyed: 
				listType = 0;
				break;
			default: break;
			}

			UINT32 existingListType;
			UINT32 existingIdx;
			decodeComponentId(component->getSceneManagerId(), existingIdx, existingListType);

			if(existingListType == listType)
				continue;

			if(existingListType != 0)
				removeFromStateList(component);

			addToStateList(component, listType);
		}

		mStateChanges.clear();
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

	bool SceneManager::isComponentOfType(const HComponent& component, UINT32 rttiId)
	{
		return component->getRTTI()->getRTTIId() == rttiId;
	}

	void SceneManager::_update()
	{
		processStateChanges();

		// Note: Eventually perform updates based on component types and/or on component priority. Right now we just
		// iterate in an undefined order, but it wouldn't be hard to change it.

		ScopeToggle toggle(mDisableStateChange);
		for (auto& entry : mActiveComponents)
			entry->update();

		GameObjectManager::instance().destroyQueuedObjects();
	}

	void SceneManager::_fixedUpdate()
	{
		processStateChanges();

		ScopeToggle toggle(mDisableStateChange);
		for (auto& entry : mActiveComponents)
			entry->fixedUpdate();
	}

	void SceneManager::registerNewSO(const HSceneObject& node)
	{ 
		if(mRootNode)
			node->setParent(mRootNode);
	}

	void SceneManager::onMainRenderTargetResized()
	{
		auto& rtProps = mMainRT->getProperties();
		float aspect = rtProps.width / (float)rtProps.height;

		for (auto& entry : mMainCameras)
			entry->setAspectRatio(aspect);
	}

	SceneManager& gSceneManager()
	{
		return SceneManager::instance();
	}
}
