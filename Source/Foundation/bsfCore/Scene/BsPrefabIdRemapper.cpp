//************************************ bs::framework - Copyright 2024 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefabIdRemapper.h"
#include "BsPrefab.h"
#include "BsSceneObject.h"
#include "Resources/BsResources.h"

using namespace bs;

PrefabIdRemapper::PrefabIdRemapper(const HSceneObject& originalPrefabHierarchy, const UUID& rootPrefabId)
	:mPrefabId(rootPrefabId)
{
	PrefabInformation rootPrefabInformation;
	rootPrefabInformation.PrefabId = rootPrefabId;

	if(originalPrefabHierarchy.IsValid())
		rootPrefabInformation.PrefabHierarchyIds = PrefabUtility::GetInstanceToPrefabLinkInformationMap(originalPrefabHierarchy, true);

	mPrefabCache[rootPrefabId] = std::move(rootPrefabInformation);
}

UnorderedMap<UUID, UUID> PrefabIdRemapper::RestoreOriginalPrefabIds(const HSceneObject& hierarchyRoot)
{
	// This is used as the starting point when looking up instance IDs.
	const UUID instanceRootPrefabId = hierarchyRoot->GetPrefabResourceId();

	FrameScope frameScope;
	FrameStack<SceneObjectInformation> todo;
	todo.emplace(hierarchyRoot, UUID::kEmpty, -1);

	UnorderedMap<UUID, UUID> remappedGameObjectIDs;
	while(!todo.empty())
	{
		const SceneObjectInformation currentObjectToProcess = todo.top();
		todo.pop();

		HSceneObject sceneObject = currentObjectToProcess.SceneObject;

		// Deduce original IDs for the scene object
		const UUID originalSceneObjectId = currentObjectToProcess.SceneObject.GetId();
		PrefabObjectIdAndLinkInformation sceneObjectIds = DeduceInternalPrefabIds(currentObjectToProcess.SceneObject->GetPrefabObjectId(), instanceRootPrefabId, currentObjectToProcess);

		// If we've reached a new prefab instance, increment nesting level.
		i32 nestingLevel = currentObjectToProcess.ParentNestingLevel;
		if(nestingLevel == -1) // -1 means parent resource ID has not been assigned yet
			nestingLevel = 0;
		else if(sceneObjectIds.LinkInformation.PrefabResourceId != currentObjectToProcess.ParentPrefabResourceId)
			nestingLevel++;

		// Deduce and assign component IDs
		// Note: Important to process components before the scene object, as we depend on the owning scene object's IDs to be unchanged
		for(auto& component : sceneObject->GetComponents())
		{
			const UUID originalComponentId = component.GetId();
			PrefabObjectIdAndLinkInformation componentIds = DeduceInternalPrefabIds(component->GetPrefabObjectId(), instanceRootPrefabId, currentObjectToProcess);

			AssignInternalPrefabIds(component, componentIds.GameObjectId, componentIds.LinkInformation, mPrefabId, nestingLevel);

			remappedGameObjectIDs[originalComponentId] = componentIds.GameObjectId; 
		}

		// Assign scene object IDs
		{
			AssignInternalPrefabIds(sceneObject, sceneObjectIds.GameObjectId, sceneObjectIds.LinkInformation, mPrefabId, nestingLevel);

			remappedGameObjectIDs[originalSceneObjectId] = sceneObjectIds.GameObjectId;
		}

		const u32 childCount = currentObjectToProcess.SceneObject->GetChildCount();
		for(u32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			const HSceneObject childSceneObject = currentObjectToProcess.SceneObject->GetChild(childIndex);
			todo.emplace(childSceneObject, sceneObjectIds.LinkInformation.PrefabResourceId, nestingLevel);
		}
	}

	return remappedGameObjectIDs;
}

const PrefabIdRemapper::PrefabInformation& PrefabIdRemapper::GetOrPopulatePrefabInformation(const UUID& prefabResourceId)
{
	static const PrefabInformation kEmpty;

	if(prefabResourceId.Empty())
		return kEmpty;

	if(auto found = mPrefabCache.find(prefabResourceId); found != mPrefabCache.end())
		return found->second;

	PrefabInformation prefabInformation;
	prefabInformation.Prefab = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(prefabResourceId, false, ResourceLoadFlag::None));

	if(prefabInformation.Prefab.IsLoaded(false))
		prefabInformation.PrefabHierarchyIds = PrefabUtility::GetInstanceToPrefabLinkInformationMap(prefabInformation.Prefab->GetRoot(), true);

	auto result = mPrefabCache.insert(std::make_pair(prefabResourceId, std::move(prefabInformation)));
	return result.first->second;
}

PrefabIdRemapper::PrefabObjectIdAndLinkInformation PrefabIdRemapper::DeduceInternalPrefabIds(const UUID& prefabObjectId, const UUID& prefabResourceId, const SceneObjectInformation& associatedSceneObjectInformation, i32 nestingLevel)
{
	// Process to convert prefab object ID -> instance ID is as follows. How we do this depends on the current instance of the object we're resolving for:
	// 1. If the object has no prefab object ID, it is not part of an instance, so we generate a new instance ID.
	// 2. It is an instance of our active prefab, in which case prefabResourceId == mPrefabId. In such case we perform lookup in the prefab hierarchy
	//    based on prefab object ID. If the object is found, prefab object ID is a valid instance ID we assigned previously. Otherwise new ID is generated.
	// 3. It is an instance of some other prefab, in which case prefabResourceId != mPrefabId. In such case we perform lookup in that other prefab's
	//    hierarchy based on prefab object ID. This may yield us a link to another prefab in which case:
	//     - We recurse into that prefab, and if that prefab is our active prefab, we handle it like case #2, checking the current prefab object ID.
	//     - If that prefab is not our prefab, we repeat the process in case #3 using the newly found prefab object ID and prefab resource ID. We repeat the
	//       the process until we reach our active prefab, or find no more prefab links.
	//     - If we don't find our active prefab, we generate a new instance ID.

	// TODO - Describe process for deducing prefab links

	PrefabObjectIdAndLinkInformation output;

	// Try to find or generate a new scene object ID and prefab object ID
	if(prefabObjectId.Empty()) // This is a new object that was never part of a prefab (or its link was broken, in which case we treat it as new)
	{
		output.GameObjectId = UUIDGenerator::GenerateRandom();
		output.LinkInformation.PrefabObjectId = output.GameObjectId;
		output.LinkInformation.PrefabResourceId = associatedSceneObjectInformation.ParentPrefabResourceId;
	}
	else
	{
		// TODO - Improved process for prefab links?
		//  - prefabResourceId initially provided should be an exact link depending on the current object (rather than always the root)
		//  - 

		// Find prefab link IDs in the original hierarchy
		const UnorderedMap<UUID, PrefabLinkInformation>& prefabHierarchyIds = GetOrPopulatePrefabInformation(prefabResourceId).PrefabHierarchyIds;
		auto found = prefabHierarchyIds.find(prefabObjectId);
		if(found != prefabHierarchyIds.end()) // This is an object that was previously part of the prefab, retrieve its original ID
		{
			if(prefabResourceId != mPrefabId) // If it's not this prefab, try looking deeper
				return DeduceInternalPrefabIds(found->second.PrefabObjectId, found->second.PrefabResourceId, associatedSceneObjectInformation, nestingLevel + 1);

			// Found the prefab object ID in this prefab
			output.GameObjectId = prefabObjectId;
			output.LinkInformation = found->second;
		}
		else
		{
			output.GameObjectId = UUIDGenerator::GenerateRandom();

			// This is an object with a prefab link, but its not part of the top-most instance, which means its a newly added nested prefab and needs a new ID
			// and link IDs will be taken from the instance itself.
			if(nestingLevel == 0)
			{
				output.LinkInformation.PrefabObjectId = prefabObjectId;
				output.LinkInformation.PrefabResourceId = associatedSceneObjectInformation.SceneObject->GetPrefabResourceId();

				if(!B3D_ENSURE(!output.LinkInformation.PrefabResourceId.Empty()))
					output.LinkInformation.PrefabResourceId = associatedSceneObjectInformation.ParentPrefabResourceId; // Fallback, shouldn't happen
			}
			// This is an object with a prefab link, but it's already part of some other prefab's instance, which means it needs a new ID, and link IDs
			// need to be taken from the prefab it is part of (as the current link IDs point to that other prefab)
			else
			{
				output.LinkInformation.PrefabObjectId = prefabObjectId;
				output.LinkInformation.PrefabResourceId = prefabResourceId;
			}
		}
	}

	return output;
}

void PrefabIdRemapper::AssignInternalPrefabIds(const GameObjectHandleBase& gameObject, const UUID& gameObjectIdInPrefab, const PrefabLinkInformation& linkInformationInPrefab, const UUID& rootPrefabResourceId, i32 nestingLevel)
{
	gameObject->SetId(gameObjectIdInPrefab);
	gameObject.GetSharedHandleData()->Id = gameObjectIdInPrefab;

	if(nestingLevel == 0) // Root objects just point to self
		gameObject->SetPrefabObjectId(gameObjectIdInPrefab);
	else
		gameObject->SetPrefabObjectId(linkInformationInPrefab.PrefabObjectId);

	// If scene object, set prefab resource IDs as well
	if(auto sceneObject = B3DRTTICast<SceneObject>(gameObject.Get()))
	{
		if(nestingLevel == 0) // Root objects point to root prefab
			sceneObject->SetPrefabResourceId(rootPrefabResourceId);
		else
			sceneObject->SetPrefabResourceId(linkInformationInPrefab.PrefabResourceId);
	}
}
