//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefabUtility.h"
#include "Scene/BsPrefabDiff.h"
#include "Scene/BsPrefab.h"
#include "Scene/BsSceneObject.h"
#include "Resources/BsResources.h"

using namespace bs;

namespace impl
{
/**	Contains saved Component instance data. */
struct ComponentProxy
{
	GameObjectInstanceDataPtr InstanceData;
	u32 LinkId;
	UUID Uuid;
};

/** Contains saved SceneObject instance data, as well as saved instance data for all its children and components. */
struct SceneObjectProxy
{
	GameObjectInstanceDataPtr InstanceData;
	u32 LinkId;
	UUID Uuid;

	Vector<ComponentProxy> Components;
	Vector<SceneObjectProxy> Children;
};

/** Contains linked game-object instance data and UUID. */
struct LinkedInstanceData
{
	GameObjectInstanceDataPtr InstanceData;
	UUID Uuid;
};

/**
 * Traverses the object hierarchy, finds all child objects and components and records their instance data, as well
 * as their original place in the hierarchy. Instance data essentially holds the object's "identity" and by
 * restoring it we ensure any handles pointing to the object earlier will still point to the new version.
 *
 * @param[in]	so					Object to traverse and record.
 * @param[out]	output				Contains the output hierarchy of instance data.
 * @param[out]	linkedInstanceData	A map of link IDs to instance data. Objects without link IDs will not be
 *									included here.
 *
 * @note	Does not recurse into child prefab instances.
 */
void recordInstanceData(const HSceneObject& so, SceneObjectProxy& output, UnorderedMap<u32, LinkedInstanceData>& linkedInstanceData)
{
	struct StackData
	{
		HSceneObject So;
		SceneObjectProxy* Proxy;
	};

	Stack<StackData> todo;
	todo.push({ so, &output });

	output.InstanceData = so->GetInstanceDataInternal();
	output.Uuid = so->GetUuid();
	output.LinkId = (u32)-1;

	while(!todo.empty())
	{
		StackData curData = todo.top();
		todo.pop();

		const Vector<HComponent>& components = curData.So->GetComponents();
		for(auto& component : components)
		{
			curData.Proxy->Components.push_back(ComponentProxy());

			ComponentProxy& componentProxy = curData.Proxy->Components.back();
			componentProxy.InstanceData = component->GetInstanceDataInternal();
			componentProxy.Uuid = component->GetUuid();
			componentProxy.LinkId = component->GetLinkId();

			linkedInstanceData[componentProxy.LinkId] = { componentProxy.InstanceData, componentProxy.Uuid };
		}

		u32 numChildren = curData.So->GetNumChildren();
		curData.Proxy->Children.resize(numChildren);

		for(u32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = curData.So->GetChild(i);

			SceneObjectProxy& childProxy = curData.Proxy->Children[i];

			childProxy.InstanceData = child->GetInstanceDataInternal();
			childProxy.Uuid = child->GetUuid();
			childProxy.LinkId = child->GetLinkId();

			linkedInstanceData[childProxy.LinkId] = { childProxy.InstanceData, childProxy.Uuid };

			if(child->GetPrefabLinkUUIDInternal().Empty())
				todo.push({ child, &curData.Proxy->Children[i] });
		}
	}
}

/**
 * Restores instance data in the provided hierarchy, using link ids to determine what data maps to which objects.
 *
 * @param[in]	so					Object to traverse and restore the instance data.
 * @param[in]	proxy				Hierarchy containing instance data for all objects and components, returned by
 *									recordInstanceData() method.
 * @param[in]	linkedInstanceData	A map of link IDs to instance data, returned by recordInstanceData() method.
 *
 * @note	Does not recurse into child prefab instances.
 */
void restoreLinkedInstanceData(const HSceneObject& so, SceneObjectProxy& proxy, UnorderedMap<u32, LinkedInstanceData>& linkedInstanceData)
{
	Stack<HSceneObject> todo;
	todo.push(so);

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		Vector<HComponent>& components = current->GetComponentsInternal();
		for(auto& component : components)
		{
			if(component->GetLinkId() != (u32)-1)
			{
				auto iterFind = linkedInstanceData.find(component->GetLinkId());
				if(iterFind != linkedInstanceData.end())
				{
					component->SetInstanceDataInternal(iterFind->second.InstanceData);
					component->SetUUIDInternal(iterFind->second.Uuid);
					component.SetHandleDataInternal(component.GetInternalPtr());
				}
			}
		}

		u32 numChildren = current->GetNumChildren();
		for(u32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = current->GetChild(i);

			if(child->GetLinkId() != (u32)-1)
			{
				auto iterFind = linkedInstanceData.find(child->GetLinkId());
				if(iterFind != linkedInstanceData.end())
				{
					child->SetInstanceDataInternal(iterFind->second.InstanceData);
					child->SetUUIDInternal(iterFind->second.Uuid);
				}
			}

			if(child->GetPrefabLinkUUIDInternal().Empty())
				todo.push(child);
		}
	}
}

/**
 * Restores instance data in the provided hierarchy, but only for objects without a link id. Since the objects do
 * not have a link ID we rely on their sequential order to find out which instance data belongs to which object.
 *
 * @param[in]	so		Object to traverse and restore the instance data.
 * @param[in]	proxy	Hierarchy containing instance data for all objects and components, returned by
 *						recordInstanceData() method.
 *
 * @note	Does not recurse into child prefab instances.
 */
void restoreUnlinkedInstanceData(const HSceneObject& so, SceneObjectProxy& proxy)
{
	struct StackEntry
	{
		HSceneObject So;
		SceneObjectProxy* Proxy;
	};

	Stack<StackEntry> todo;
	todo.push(StackEntry());

	StackEntry& topEntry = todo.top();
	topEntry.So = so;
	topEntry.Proxy = &proxy;

	while(!todo.empty())
	{
		StackEntry current = todo.top();
		todo.pop();

		if(current.Proxy->LinkId == (u32)-1)
		{
			current.So->SetInstanceDataInternal(current.Proxy->InstanceData);
			current.So->SetUUIDInternal(current.Proxy->Uuid);
		}

		Vector<HComponent>& components = current.So->GetComponentsInternal();
		u32 componentProxyIdx = 0;
		u32 numComponentProxies = (u32)current.Proxy->Components.size();
		for(auto& component : components)
		{
			if(component->GetLinkId() == (u32)-1)
			{
				bool foundInstanceData = false;
				(void)foundInstanceData;
				for(; componentProxyIdx < numComponentProxies; componentProxyIdx++)
				{
					if(current.Proxy->Components[componentProxyIdx].LinkId != (u32)-1)
						continue;

					component->SetInstanceDataInternal(current.Proxy->Components[componentProxyIdx].InstanceData);
					component->SetUUIDInternal(current.Proxy->Components[componentProxyIdx].Uuid);
					component.SetHandleDataInternal(component.GetInternalPtr());

					foundInstanceData = true;
					break;
				}

				assert(foundInstanceData);
			}
		}

		u32 numChildren = current.So->GetNumChildren();
		u32 childProxyIdx = 0;
		u32 numChildProxies = (u32)current.Proxy->Children.size();
		for(u32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = current.So->GetChild(i);

			if(child->GetLinkId() == (u32)-1)
			{
				bool foundInstanceData = false;
				(void)foundInstanceData;
				for(; childProxyIdx < numChildProxies; childProxyIdx++)
				{
					if(current.Proxy->Children[childProxyIdx].LinkId != (u32)-1)
						continue;

					assert(current.Proxy->Children[childProxyIdx].LinkId == (u32)-1);
					child->SetInstanceDataInternal(current.Proxy->Children[childProxyIdx].InstanceData);
					child->SetUUIDInternal(current.Proxy->Children[childProxyIdx].Uuid);

					if(child->GetPrefabLinkUUIDInternal().Empty())
					{
						todo.push(StackEntry());

						StackEntry& newEntry = todo.top();
						newEntry.So = child;
						newEntry.Proxy = &current.Proxy->Children[childProxyIdx];
					}

					foundInstanceData = true;
					break;
				}

				assert(foundInstanceData);
			}
			else
			{
				if(!child->GetPrefabLinkUUIDInternal().Empty())
					continue;

				for(u32 j = 0; j < numChildProxies; j++)
				{
					if(child->GetLinkId() == current.Proxy->Children[j].LinkId)
					{
						todo.push(StackEntry());

						StackEntry& newEntry = todo.top();
						newEntry.So = child;
						newEntry.Proxy = &current.Proxy->Children[j];
						break;
					}
				}
			}
		}
	}
}
} // namespace impl

void PrefabUtility::RevertToPrefab(const HSceneObject& so)
{
	UUID prefabLinkUUID = so->GetPrefabLink();
	HPrefab prefabLink = static_resource_cast<Prefab>(gResources().LoadFromUuid(prefabLinkUUID, false, ResourceLoadFlag::None));

	if(!prefabLink.IsLoaded(false))
		return;

	// Save IDs, destroy original, create new, restore IDs
	::impl::SceneObjectProxy soProxy;
	UnorderedMap<u32, ::impl::LinkedInstanceData> linkedInstanceData;
	::impl::recordInstanceData(so, soProxy, linkedInstanceData);

	HSceneObject parent = so->GetParent();

	// This will destroy the object but keep it in the parent's child list
	HSceneObject currentSO = so;
	so->DestroyInternal(currentSO, true);

	HSceneObject newInstance = prefabLink->Instantiate();

	// Remove default parent, and replace with original one
	newInstance->mParent->RemoveChild(newInstance);
	newInstance->mParent = parent;

	::impl::restoreLinkedInstanceData(newInstance, soProxy, linkedInstanceData);
}

void PrefabUtility::UpdateFromPrefab(const HSceneObject& so)
{
	HSceneObject topLevelObject = so;

	while(topLevelObject != nullptr)
	{
		if(!topLevelObject->mPrefabLinkUUID.Empty())
			break;

		if(topLevelObject->mParent != nullptr)
			topLevelObject = topLevelObject->mParent;
		else
			topLevelObject = nullptr;
	}

	if(topLevelObject == nullptr)
		topLevelObject = so;

	Stack<HSceneObject> todo;
	todo.push(topLevelObject);

	// Find any prefab instances
	Vector<HSceneObject> prefabInstanceRoots;

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		if(!current->mPrefabLinkUUID.Empty())
			prefabInstanceRoots.push_back(current);

		u32 childCount = current->GetNumChildren();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = current->GetChild(i);
			todo.push(child);
		}
	}

	// Stores data about the new prefab instance and its original parent and link id
	// (as those aren't stored in the prefab diff)
	struct RestoredPrefabInstance
	{
		HSceneObject NewInstance;
		HSceneObject OriginalParent;
		SPtr<PrefabDiff> Diff;
		u32 OriginalLinkId;
	};

	Vector<RestoredPrefabInstance> newPrefabInstanceData;

	// For each prefab instance load its reference prefab from the disk and check if it changed. If it has changed
	// instantiate the prefab and destroy the current instance. Then apply instance specific changes stored in a
	// prefab diff, if any, as well as restore the original parent and link id (link id of the root prefab instance
	// belongs to the parent prefab if any). Finally fix any handles pointing to the old objects so that they now point
	// to the newly instantiated objects. To the outside world it should be transparent that we just destroyed and then
	// re-created from scratch the entire hierarchy.

	// Need to do this bottom up to ensure I don't destroy the parents before children
	for(auto iter = prefabInstanceRoots.rbegin(); iter != prefabInstanceRoots.rend(); ++iter)
	{
		HSceneObject current = *iter;
		HPrefab prefabLink = static_resource_cast<Prefab>(gResources().LoadFromUuid(current->mPrefabLinkUUID, false, ResourceLoadFlag::None));

		if(prefabLink.IsLoaded(false) && prefabLink->GetHash() != current->mPrefabHash)
		{
			// Save IDs, destroy original, create new, restore IDs
			::impl::SceneObjectProxy soProxy;
			UnorderedMap<u32, ::impl::LinkedInstanceData> linkedInstanceData;
			::impl::recordInstanceData(current, soProxy, linkedInstanceData);

			HSceneObject parent = current->GetParent();
			SPtr<PrefabDiff> prefabDiff = current->mPrefabDiff;

			current->Destroy(true);
			HSceneObject newInstance = prefabLink->CloneInternal();

			// When restoring instance IDs it is important to make all the new handles point to the old GameObjectInstanceData.
			// This is because old handles will have different GameObjectHandleData and we have no easy way of accessing it to
			// change to which GameObjectInstanceData it points. But the GameObjectManager ensures that all handles deserialized
			// at once (i.e. during the ::CloneInternal() call above) will share GameObjectHandleData so we can simply replace
			// to what they point to, affecting all of the handles to that object. (In another words, we can modify the
			// new handles at this point, but old ones must keep referencing what they already were.)
			::impl::restoreLinkedInstanceData(newInstance, soProxy, linkedInstanceData);
			::impl::restoreUnlinkedInstanceData(newInstance, soProxy);

			newPrefabInstanceData.push_back({ newInstance, parent, prefabDiff, newInstance->GetLinkId() });
		}
	}

	// Once everything is cloned, apply diffs, restore old parents & link IDs for root.
	for(auto& entry : newPrefabInstanceData)
	{
		// Diffs must be applied after everything is instantiated and instance data restored since it may contain
		// game object handles within or external to its prefab instance.
		if(entry.Diff != nullptr)
			entry.Diff->Apply(entry.NewInstance);

		entry.NewInstance->mPrefabDiff = entry.Diff;

		entry.NewInstance->SetParent(entry.OriginalParent, false);
		entry.NewInstance->mLinkId = entry.OriginalLinkId;
	}

	// Finally, instantiate everything if the top scene object is live (instantiated)
	if(topLevelObject->IsInstantiated())
	{
		for(auto& entry : newPrefabInstanceData)
			entry.NewInstance->InstantiateInternal(true);
	}

	gResources().UnloadAllUnused();
}

void PrefabUtility::GeneratePrefabIds(const HSceneObject& sceneObject)
{
	u32 startingId = 0;

	Stack<HSceneObject> todo;
	todo.push(sceneObject);

	while(!todo.empty())
	{
		HSceneObject currentSO = todo.top();
		todo.pop();

		for(auto& component : currentSO->mComponents)
		{
			if(component->GetLinkId() != (u32)-1)
				startingId = std::max(component->mLinkId + 1, startingId);
		}

		u32 numChildren = (u32)currentSO->GetNumChildren();
		for(u32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = currentSO->GetChild(i);

			if(!child->HasFlag(SOF_DontSave))
			{
				if(child->GetLinkId() != (u32)-1)
					startingId = std::max(child->mLinkId + 1, startingId);

				if(child->mPrefabLinkUUID.Empty())
					todo.push(currentSO->GetChild(i));
			}
		}
	}

	u32 currentId = startingId;
	todo.push(sceneObject);

	while(!todo.empty())
	{
		HSceneObject currentSO = todo.top();
		todo.pop();

		for(auto& component : currentSO->mComponents)
		{
			if(component->GetLinkId() == (u32)-1)
				component->mLinkId = currentId++;
		}

		u32 numChildren = (u32)currentSO->GetNumChildren();
		for(u32 i = 0; i < numChildren; i++)
		{
			HSceneObject child = currentSO->GetChild(i);

			if(!child->HasFlag(SOF_DontSave))
			{
				if(child->GetLinkId() == (u32)-1)
					child->mLinkId = currentId++;

				if(child->mPrefabLinkUUID.Empty())
					todo.push(currentSO->GetChild(i));
			}
		}
	}

	if(currentId < startingId)
	{
		BS_EXCEPT(InternalErrorException, "Prefab ran out of IDs to assign. "
										  "Consider increasing the size of the prefab ID data type.");
	}
}

void PrefabUtility::ClearPrefabIds(const HSceneObject& sceneObject, bool recursive, bool clearRoot)
{
	Stack<HSceneObject> todo;
	todo.push(sceneObject);

	if(clearRoot)
		sceneObject->mLinkId = (u32)-1;

	while(!todo.empty())
	{
		HSceneObject currentSO = todo.top();
		todo.pop();

		for(auto& component : currentSO->mComponents)
			component->mLinkId = (u32)-1;

		if(recursive)
		{
			u32 numChildren = (u32)currentSO->GetNumChildren();
			for(u32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = currentSO->GetChild(i);
				child->mLinkId = (u32)-1;

				if(child->mPrefabLinkUUID.Empty())
					todo.push(child);
			}
		}
	}
}

void PrefabUtility::RecordPrefabDiff(const HSceneObject& sceneObject)
{
	HSceneObject topLevelObject = sceneObject;

	while(topLevelObject != nullptr)
	{
		if(!topLevelObject->mPrefabLinkUUID.Empty())
			break;

		if(topLevelObject->mParent != nullptr)
			topLevelObject = topLevelObject->mParent;
		else
			topLevelObject = nullptr;
	}

	if(topLevelObject == nullptr)
		topLevelObject = sceneObject;

	Stack<HSceneObject> todo;
	todo.push(topLevelObject);

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		if(!current->mPrefabLinkUUID.Empty())
		{
			current->mPrefabDiff = nullptr;

			HPrefab prefabLink = static_resource_cast<Prefab>(gResources().LoadFromUuid(current->mPrefabLinkUUID, false, ResourceLoadFlag::None));
			if(prefabLink.IsLoaded(false))
				current->mPrefabDiff = PrefabDiff::Create(prefabLink->GetRootInternal(), current->GetHandle());
		}

		u32 childCount = current->GetNumChildren();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = current->GetChild(i);
			todo.push(child);
		}
	}

	gResources().UnloadAllUnused();
}
