//************************************ B3D Framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DPrefabUpdateHelper.h"

#include "B3DGameObjectCollection.h"
#include "B3DPrefabUtility.h"
#include "B3DSceneManager.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DPrefab.h"
#include "Scene/B3DSceneObject.h"
#include "Resources/B3DResources.h"

using namespace b3d;

/** Contains a reference to a scene object representing a prefab instance root, and a prefab resource associated with that root. */
struct PrefabInstanceRoot
{
	PrefabInstanceRoot(const HPrefab& parentPrefab, const HSceneObject& sceneObjectInParentPrefab, const HPrefab& prefabToUpdateFrom)
		: ParentPrefab(parentPrefab), SceneObjectInParentPrefab(sceneObjectInParentPrefab), PrefabToUpdateFrom(prefabToUpdateFrom)
	{ }

	HPrefab ParentPrefab;
	HSceneObject SceneObjectInParentPrefab;
	HPrefab PrefabToUpdateFrom;
};

TOptional<PrefabUpdateHelper::ObjectInPrefab> PrefabUpdateHelper::FindInstanceInPrefab(const GameObjectHandle& gameObject, const UUID& prefabResourceId)
{
	const HPrefab prefab = GetResources().Load<Prefab>(prefabResourceId, ResourceLoadOptions(false, false, false));

	if(!prefab.IsLoaded())
	{
		B3D_LOG(Error, LogPrefab, "Unable to find instance in prefab. Prefab {0} cannot be loaded.", prefabResourceId);
		return {};
	}

	const SPtr<GameObjectCollection>& gameObjectCollection = prefab->GetGameObjectCollection();
	if(!B3D_ENSURE(gameObjectCollection))
		return {};

	HGameObject instanceInPrefab;
	if(!gameObjectCollection->TryGetObject(gameObject->GetPrefabObjectId(), instanceInPrefab))
		return {};

	return ObjectInPrefab(prefab, instanceInPrefab);
}

TOptional<PrefabUpdateHelper::ObjectInPrefab> PrefabUpdateHelper::FindInstanceInPrefab(const HSceneObject& sceneObject)
{
	if(!sceneObject->IsPrefabInstance())
		return {};

	return FindInstanceInPrefab(sceneObject, sceneObject->GetPrefabResourceId());
}

TOptional<PrefabUpdateHelper::ObjectInPrefab> PrefabUpdateHelper::FindInstanceInPrefab(const HComponent& component)
{
	HSceneObject sceneObject = component->SceneObject();
	if(!sceneObject->IsPrefabInstance())
		return {};

	return FindInstanceInPrefab(component, sceneObject->GetPrefabResourceId());
}

UnorderedMap<UUID, UUID> PrefabUpdateHelper::FindInstanceIdsThatNeedRemapping(const HSceneObject& root)
{
	// First iterate parents of @p sceneObjectToUpdateWith, and find the root-most prefab instance
	UUID rootInstancePrefabId;
	{
		// Start with the parent, because we don't care if sceneObjectToUpdateWith is an instance itself, as that doesn't require special handling
		HSceneObject currentObject = root->GetParent();
		while(currentObject.IsValid())
		{
			if(!currentObject->IsPrefabInstance())
				break;

			rootInstancePrefabId = currentObject->GetPrefabResourceId();
			currentObject = currentObject->GetParent();
		}
	}

	// For each object in the provided instance hierarchy, determine the bottom-most prefab it has been defined in, relative to the root prefab instance
	UnorderedMap<UUID, UUID> instanceIdToBottomMostPrefab;
	root->IterateHierarchy([&rootInstancePrefabId, &instanceIdToBottomMostPrefab](const HSceneObject& sceneObject) {
		// Map scene object
		{
			HPrefab bottomMostPrefab = nullptr;
			HSceneObject sceneObjectInBottomMostPrefab = sceneObject;

			if(!rootInstancePrefabId.Empty() && sceneObject->GetPrefabResourceId() == rootInstancePrefabId) // If an object is not part of the root instance, we don't need special handling
			{
				while(TOptional<ObjectInPrefab> found = FindInstanceInPrefab(sceneObjectInBottomMostPrefab))
				{
					if(!found.has_value())
						break;

					HSceneObject instanceInPrefab = B3DStaticGameObjectCast<SceneObject>(found->InstanceInPrefab);

					// Reached the bottom-most prefab, as the instance points to itself
					if(found->Prefab->GetId() == instanceInPrefab->GetPrefabResourceId())
						break;

					sceneObjectInBottomMostPrefab = instanceInPrefab;
					bottomMostPrefab = found->Prefab;
				}
			}

			if(bottomMostPrefab != nullptr)
				instanceIdToBottomMostPrefab[sceneObject.GetId()] = sceneObjectInBottomMostPrefab->GetId();
			else
				instanceIdToBottomMostPrefab[sceneObject.GetId()] = sceneObject.GetId();
		}

		// Map components
		for(const auto& component : sceneObject->GetComponents())
		{
			HPrefab bottomMostPrefab = nullptr;
			HComponent componentInBottomMostPrefab = component;

			if(!rootInstancePrefabId.Empty() && sceneObject->GetPrefabResourceId() == rootInstancePrefabId) // If an object is not part of the root instance, we don't need special handling
			{
				while(TOptional<ObjectInPrefab> found = FindInstanceInPrefab(componentInBottomMostPrefab))
				{
					if(!found.has_value())
						break;

					HComponent instanceInPrefab = B3DStaticGameObjectCast<Component>(found->InstanceInPrefab);

					// Reached the bottom-most prefab, as the instance points to itself
					if(found->Prefab->GetId() == instanceInPrefab->SceneObject()->GetPrefabResourceId())
						break;

					componentInBottomMostPrefab = instanceInPrefab;
					bottomMostPrefab = found->Prefab;
				}
			}

			if(bottomMostPrefab != nullptr)
				instanceIdToBottomMostPrefab[component.GetId()] = componentInBottomMostPrefab->GetId();
			else
				instanceIdToBottomMostPrefab[component.GetId()] = component.GetId();
		}

		return true;
	}, nullptr, true);

	return instanceIdToBottomMostPrefab;
}

void PrefabUpdateHelper::UpdatePrefab(const HPrefab& prefabToUpdate, const HSceneObject& sceneObjectToUpdateWith)
{
	if(!B3D_ENSURE(prefabToUpdate.IsLoaded(false)))
		return;

	// Find the object ID in the last (bottom-most, most-nested) prefab that an object is linked to (ignoring links to itself). If the object is not linked
	// to any prefab, or is just linked to itself, this will be its own ID in the provided hierarchy. This ID represents the object whose prefab object ID we will need to update.
	UnorderedMap<UUID, UUID> instanceIdToPrefabThatNeedsRemappingInstanceId = FindInstanceIdsThatNeedRemapping(sceneObjectToUpdateWith);

	FrameAllocatorScope frameScope;

	// Record any parent prefab instances. As described above, we need to update these recursively bottom to top, in order to maintain prefab object ID links.
	FrameVector<PrefabInstanceRoot> prefabInstanceParents;
	if(sceneObjectToUpdateWith->IsPrefabInstance())
	{
		HSceneObject currentSceneObject = sceneObjectToUpdateWith;
		HPrefab parentPrefab = nullptr;
		while(TOptional<ObjectInPrefab> found = FindInstanceInPrefab(currentSceneObject))
		{
			if(!found.has_value())
				break;

			currentSceneObject = B3DStaticGameObjectCast<SceneObject>(found->InstanceInPrefab);
			parentPrefab = found->Prefab;

			// Current prefab becomes the prefab to update from for the parent
			if(!prefabInstanceParents.empty())
				prefabInstanceParents.back().PrefabToUpdateFrom = parentPrefab;

			if(parentPrefab == prefabToUpdate)
				break;

			HSceneObject currentSceneObjectInstanceRoot = currentSceneObject->GetPrefabInstanceRoot();
			if(!B3D_ENSURE(currentSceneObjectInstanceRoot != nullptr))
				continue;

			prefabInstanceParents.push_back(PrefabInstanceRoot(found->Prefab, currentSceneObjectInstanceRoot, nullptr));
		}
	}

	// Update the prefab, and retrieve the map containing the prefab object IDs that we assigned. 
	UnorderedMap<UUID, UUID> instanceIdToPrefabToUpdateInstanceId = prefabToUpdate->ReplaceInternalHierarchy(sceneObjectToUpdateWith);

	// Clear the delta as the prefab was just updated to match the hierarchy exactly
	sceneObjectToUpdateWith->SetPrefabDelta(nullptr);

	// Combine @p instanceIdToFirstRelevantPrefab with @p instanceIdToPrefabToUpdateInstanceId to give us a map that tells us which objects need
	// their prefab object ID remapped (value in @p instanceIdToPrefabThatNeedsRemappingInstanceId), and what is prefab object ID to remap to (value in @p instanceIdToPrefabToUpdateInstanceId).
	UnorderedMap<UUID, PrefabLinkInformation> prefabObjectIdRemapping;
	for(const auto& entry : instanceIdToPrefabThatNeedsRemappingInstanceId)
	{
		if(auto found = instanceIdToPrefabToUpdateInstanceId.find(entry.first); found != instanceIdToPrefabToUpdateInstanceId.end())
			prefabObjectIdRemapping[entry.second] = PrefabLinkInformation(found->second, prefabToUpdate->GetId());
	}

	// In order to keep prefab object IDs valid, we need to update all parent prefab instances of the object we just updated, starting from most nested (bottom-most) to root (top-most).
	for(auto it = prefabInstanceParents.rbegin(); it != prefabInstanceParents.rend(); ++it)
	{
		const PrefabInstanceRoot& prefabInstanceRoot = *it;

		// If this prefab contains any object in the @p prefabObjectIdRemapping, this means this is the last prefab that has a link to this object (excluding
		// owner prefab of the object, if any). We need to update its prefab object ID, as it could have changed during ReplaceInternalHierarchy call above,
		// or during previous iterations of this loop when UpdateInstanceFromPrefab was called.
		// 
		// We need to perform the prefab object ID remapping before the UpdateInstanceFromPrefab call, otherwise game object instance data will not be restored
		// correctly as it relies on matching prefab object IDs.
		PrefabUtility::RemapPrefabInstanceIds(prefabInstanceRoot.SceneObjectInParentPrefab, prefabObjectIdRemapping);

		HSceneObject newHierarchy = PrefabUtility::UpdateInstanceFromPrefab(prefabInstanceRoot.SceneObjectInParentPrefab, *prefabInstanceRoot.PrefabToUpdateFrom);
		if(!B3D_ENSURE(newHierarchy != nullptr))
			continue;

		// UpdateInstanceFromPrefab could have resulted in new prefab instance objects getting created, with their own unique IDs. We need to make sure
		// that parent prefab instances wanting to linking to those objects (as stored in @p prefabObjectIdRemapping) now link to the new prefab instance.
		// This action is performed for each iteration as we go up the prefab instance hierarchy, ensuring the parent prefab instance ends up linking to the
		// last (top-most, least-nested) instance of the object.
		const UnorderedMap<UUID, UUID> prefabObjectToInstanceIdMap = PrefabUtility::GetPrefabToInstanceIdMap(prefabInstanceRoot.SceneObjectInParentPrefab, true);
		for(auto& entry : prefabObjectIdRemapping)
		{
			if(auto found = prefabObjectToInstanceIdMap.find(entry.second.PrefabObjectId); found != prefabObjectToInstanceIdMap.end())
				entry.second = PrefabLinkInformation(found->second, prefabInstanceRoot.ParentPrefab->GetId());
		}

		prefabInstanceRoot.ParentPrefab->TickPrefabVersion();

		// Note: We purposefully don't update deltas here, and do another update that takes care of that below. This is because
		// the prefab could contain multiple instances of our update prefab, and we're only updating one that's related to @p root
	}

	// Same as we do above for prefab instance roots, but this time for the scene hierarchy
	PrefabUtility::RemapPrefabInstanceIds(sceneObjectToUpdateWith, prefabObjectIdRemapping);

	// Ensure all instances of @p prefabToUpdate are up to date in all live scene instances and all live prefabs
	PrefabCache prefabCache;
	prefabCache.AddToCache(prefabToUpdate);

	const UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>>& sceneInstances = GetSceneManager().GetAllScenes();
	for(const auto& pair : sceneInstances)
	{
		SPtr<SceneInstance> scene = pair.second.lock();
		if(B3D_ENSURE(scene != nullptr))
		{
			FrameVector<UUID> parentPrefabChain;
			PrefabUtility::UpdateNestedPrefabInstancesRecursive(scene->GetRoot(), prefabCache);
		}
	}

	UnorderedSet<Prefab*> livePrefabs = PrefabManager::Instance().GetLivePrefabs();
	for(const auto& entry : livePrefabs)
	{
		const UUID& prefabResourceId = entry->GetId();
		if(prefabCache.ExistsInCache(prefabResourceId))
			continue;

		if(PrefabUtility::UpdateNestedPrefabInstancesRecursive(entry->GetRoot(), prefabCache))
		{
			entry->TickPrefabVersion();
			entry->RecordNestedPrefabInstanceDeltas();
		}
	}
}
