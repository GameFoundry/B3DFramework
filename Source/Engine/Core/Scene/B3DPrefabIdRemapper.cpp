//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/B3DPrefabIdRemapper.h"

#include "B3DGameObjectCollection.h"
#include "B3DPrefab.h"
#include "B3DSceneObject.h"
#include "Resources/B3DResources.h"

using namespace b3d;

PrefabIdRemapper::PrefabIdRemapper(const HSceneObject& originalPrefabHierarchy, const UUID& rootPrefabId, const TShared<GameObjectCollection>& newGameObjectCollection)
	:mPrefabId(rootPrefabId), mNewGameObjectCollection(newGameObjectCollection)
{
	PrefabInformation rootPrefabInformation;
	rootPrefabInformation.PrefabId = rootPrefabId;

	if(originalPrefabHierarchy.IsValid())
		rootPrefabInformation.PrefabHierarchyIds = PrefabUtility::GetInstanceToPrefabLinkInformationMap(originalPrefabHierarchy, true);

	mPrefabCache[rootPrefabId] = std::move(rootPrefabInformation);
}

UnorderedMap<UUID, UUID> PrefabIdRemapper::RestoreOriginalPrefabIds(const HSceneObject& hierarchyRoot)
{
	FrameAllocatorScope frameScope;
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
		PrefabObjectIdAndLinkInformation sceneObjectIds = DeduceInternalPrefabIds(currentObjectToProcess.SceneObject->GetPrefabObjectId(), currentObjectToProcess.SceneObject->GetPrefabResourceId(), currentObjectToProcess);

		// If we've reached a new prefab instance, increment nesting level.
		i32 nestingLevel = currentObjectToProcess.ParentNestingLevel;
		if(nestingLevel == -1) // -1 means parent resource ID has not been assigned yet
			nestingLevel = 0;
		else if(sceneObjectIds.LinkInformation.PrefabResourceId != currentObjectToProcess.ParentPrefabResourceId)
			nestingLevel++;

		// Deduce and assign component IDs
		// Note: Important to process components before the scene object, as we depend on the owning scene object's IDs to be unchanged
		for(auto component : sceneObject->GetComponents())
		{
			const UUID originalComponentId = component.GetId();
			PrefabObjectIdAndLinkInformation componentIds = DeduceInternalPrefabIds(component->GetPrefabObjectId(), currentObjectToProcess.SceneObject->GetPrefabResourceId(), currentObjectToProcess);

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
	prefabInformation.Prefab = GetResources().Load<Prefab>(prefabResourceId, ResourceLoadOptions(false, false, false));

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

	// Process to convert prefab resource ID -> internal prefab resource ID is similar as above, with a few differences:
	// 1. If the object has no prefab object ID, we assign the prefab ID of the parent scene object. If no parent object, the active prefab ID is used.
	// 2. If unable to find a matching object in the active prefab (using process described during prefab object ID lookup), resource ID is assigned
	//    depending on the current nesting level:
	//    - If root (nestingLevel == 0), we take the resource ID directly from the instance scene object, as this is a newly added nested prefab.
	//      This is because such object must be a part of some other prefab (it has a prefab object ID, but not our prefab resource ID), and it
	//      hasn't been registered with our prefab just yet (its prefab object ID was not found in the active prefab).
	//    - If not root (nestingLevel > 0), we take the prefab ID from the parent scene object. This is a newly added nested prefab that is
	//      treated as an instance modification. It must be an instance modification because of its nesting level, because if it was part of
	//      its parent prefab, it would have already been linked to that prefab and nesting level would be 0. We assign such an object
	//		resource ID of the parent, as we expect the instance modification might become part of the parent.

	PrefabObjectIdAndLinkInformation output;

	// Try to find or generate a new scene object ID and prefab object ID
	if(prefabObjectId.Empty()) // This is a new object that was never part of a prefab (or its link was broken, in which case we treat it as new)
	{
		output.GameObjectId = UUIDGenerator::GenerateRandom();
		output.LinkInformation.PrefabObjectId = output.GameObjectId;
		output.LinkInformation.PrefabResourceId = associatedSceneObjectInformation.ParentPrefabResourceId;

		return output;
	}

	// Find prefab link IDs in the original hierarchy
	const UnorderedMap<UUID, PrefabLinkInformation>& prefabHierarchyIds = GetOrPopulatePrefabInformation(prefabResourceId).PrefabHierarchyIds;
	auto found = prefabHierarchyIds.find(prefabObjectId);
	if(found != prefabHierarchyIds.end()) // This is an object that was previously part of the prefab, retrieve its original ID
	{
		if(prefabResourceId == mPrefabId)
		{
			// Found the prefab object ID in this prefab
			output.GameObjectId = prefabObjectId;
			output.LinkInformation = found->second;
			
			return output;
		}

		// If it's not this prefab, try looking deeper
		if(prefabResourceId != found->second.PrefabResourceId) // If resource ID points to itself, we're done as that means we've reached the most-nested prefab
			return DeduceInternalPrefabIds(found->second.PrefabObjectId, found->second.PrefabResourceId, associatedSceneObjectInformation, nestingLevel + 1);
	}

	output.GameObjectId = UUIDGenerator::GenerateRandom();
	output.LinkInformation.PrefabObjectId = prefabObjectId;

	// This is an object with a prefab link, but its not part of the top-most instance, which means its a newly added nested prefab and needs a new ID
	// and link IDs will be taken from the instance itself.
	if(nestingLevel == 0)
	{
		output.LinkInformation.PrefabResourceId = associatedSceneObjectInformation.SceneObject->GetPrefabResourceId();

		if(!B3D_ENSURE(!output.LinkInformation.PrefabResourceId.Empty()))
			output.LinkInformation.PrefabResourceId = associatedSceneObjectInformation.ParentPrefabResourceId; // Fallback, shouldn't happen
	}
	// This is an object with a prefab link, but it's already part of some other prefab's instance, which means it needs a new ID, and link IDs
	// need to be taken from the prefab it is part of (as the current link IDs point to that other prefab)
	else
	{
		output.LinkInformation.PrefabResourceId = prefabResourceId;
	}

	return output;
}

void PrefabIdRemapper::AssignInternalPrefabIds(GameObjectHandle& gameObject, const UUID& gameObjectIdInPrefab, const PrefabLinkInformation& linkInformationInPrefab, const UUID& rootPrefabResourceId, i32 nestingLevel)
{
	mNewGameObjectCollection->ChangeGameObjectId(gameObject, gameObjectIdInPrefab);

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
