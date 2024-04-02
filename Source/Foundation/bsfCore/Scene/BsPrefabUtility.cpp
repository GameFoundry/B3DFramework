//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefabUtility.h"
#include "Scene/BsSceneObjectHierarchyDelta.h"
#include "Scene/BsPrefab.h"
#include "Scene/BsSceneObject.h"
#include "Resources/BsResources.h"

using namespace bs;

/** Contains game-object instance data and UUID. */
struct PrefabInstanceData
{
	PrefabInstanceData(const SPtr<GameObjectInstanceData>& instanceData = nullptr, const UUID& id = UUID::kEmpty)
		: InstanceData(instanceData), Id(id)
	{ }

	SPtr<GameObjectInstanceData> InstanceData;
	UUID Id;
};

/**
 * Traverses the object hierarchy, finds all child objects and components and records their instance data. Instance data essentially holds the object's "identity"
 * and by restoring it we ensure any handles pointing to the object earlier will still point to the new version.
 *
 * @param	sceneObject			Root object to traverse and record.
 * @param	outInstanceData		A map of object IDs to instance data. 
 *
 * @note	Does not recurse into child prefab instances.
 */
void RecordInstanceData(const HSceneObject& sceneObject, UnorderedMap<UUID, PrefabInstanceData>& outInstanceData)
{
	const UUID originalResourceId = sceneObject->GetPrefabResourceId();

	sceneObject->IterateHierarchy(
		[&originalResourceId, &outInstanceData](const HSceneObject& sceneObject)
		{
			if(sceneObject->GetPrefabResourceId() != originalResourceId)
				return false;

			const UUID& prefabObjectId = sceneObject->GetPrefabObjectId();
			if(!prefabObjectId.Empty())
				outInstanceData[prefabObjectId] = PrefabInstanceData(sceneObject->GetInstanceData(), sceneObject->GetId());

			return true;
		},
		[&outInstanceData](const HComponent& component)
		{
			const UUID& prefabObjectId = component->GetPrefabObjectId();
			if(!prefabObjectId.Empty())
				outInstanceData[prefabObjectId] = PrefabInstanceData(component->GetInstanceData(), component->GetId());
		});
}

/**
 * Restores instance data in the provided hierarchy, using prefab ids to determine what data maps to which objects.
 *
 * @param	sceneObject		Root object to traverse and restore.
 * @param	instanceData	A map of prefab IDs to instance data, as output by RecordInstanceData() method.
 *
 * @note	Does not recurse into child prefab instances.
 */
void RestoreInstanceData(const HSceneObject& sceneObject, const UnorderedMap<UUID, PrefabInstanceData>& instanceData)
{
	const UUID originalResourceId = sceneObject->GetPrefabResourceId();

	sceneObject->IterateHierarchy(
		[&originalResourceId, &instanceData](const HSceneObject& sceneObject)
		{
			const UUID& prefabObjectId = sceneObject->GetPrefabObjectId();
			if(!prefabObjectId.Empty())
			{
				if(auto found = instanceData.find(prefabObjectId); found != instanceData.end())
				{
					sceneObject->SetInstanceData(found->second.InstanceData);
					sceneObject->SetId(found->second.Id);
				}
			}

			if(sceneObject->GetPrefabResourceId() != originalResourceId)
				return false;

			return true;
		},
		[&instanceData](const HComponent& component)
		{
			const UUID& prefabObjectId = component->GetPrefabObjectId();
			if(!prefabObjectId.Empty())
			{
				if(auto found = instanceData.find(prefabObjectId); found != instanceData.end())
				{
					component->SetInstanceData(found->second.InstanceData);
					component->SetId(found->second.Id);

					HComponent mutableComponentHandle = component; // TODO - Unify this so it's the same as the SceneObject case above
					mutableComponentHandle.SetObject(component.GetShared());
				}
			}
		});
}

void PrefabUtility::RevertToPrefab(const HSceneObject& sceneObject)
{
	if(!B3D_ENSURE(sceneObject.IsValid()))
		return;

	HSceneObject prefabInstanceRoot = sceneObject->GetPrefabInstanceRoot();
	if(!prefabInstanceRoot.IsValid())
		return;

	const UUID& prefabResourceId = sceneObject->GetPrefabResourceId();
	if(!B3D_ENSURE(!prefabResourceId.Empty()))
		return;

	HPrefab linkedPrefab = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(prefabResourceId, false, ResourceLoadFlag::None));
	if(!linkedPrefab.IsLoaded(false))
	{
		B3D_LOG(Warning, Prefab, "Cannot revert scene object '{0}' to prefab. Failed to load prefab with ID: '{1}'.", sceneObject.GetId(), prefabResourceId);
		return;
	}

	// Save IDs, destroy original, create new, restore IDs
	UnorderedMap<UUID, PrefabInstanceData> instanceData;
	RecordInstanceData(sceneObject, instanceData);

	HSceneObject parent = sceneObject->GetParent();

	// This will destroy the object but keep it in the parent's child list
	HSceneObject currentSceneObject = sceneObject;
	sceneObject->DestroyInternal(currentSceneObject, true);

	HSceneObject newInstance = linkedPrefab->Instantiate(nullptr, false);

	// Remove default parent, and replace with original one
	newInstance->mParent->RemoveChild(newInstance);
	newInstance->mParent = parent;

	RestoreInstanceData(newInstance, instanceData);
}

void PrefabUtility::UpdateFromPrefab(const HSceneObject& sceneObject)
{
	if(!B3D_ENSURE(sceneObject.IsValid()))
		return;

	HSceneObject prefabInstanceRoot = sceneObject->GetPrefabInstanceRoot();
	if(!prefabInstanceRoot.IsValid())
		return;

	Stack<HSceneObject> todo;
	todo.push(prefabInstanceRoot);

	// Find any prefab instances
	Vector<HSceneObject> prefabInstanceRoots; // TODO - This should probably just perform an update from a single prefab, and we handle the complexity of nested prefabs elsewhere (e.g. in a prefab editor) - or just allow this method to work on prefab hierarchy itself, then call it recursively

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		if(!current->IsPrefabInstanceRoot())
			prefabInstanceRoots.push_back(current);

		u32 childCount = current->GetChildCount();
		for(u32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			HSceneObject child = current->GetChild(childIndex);
			todo.push(child);
		}
	}

	// Stores data about the new prefab instance and its original parent and link id
	// (as those aren't stored in the prefab diff)
	struct RestoredPrefabInstance
	{
		HSceneObject NewInstance;
		HSceneObject OriginalParent;
		SPtr<SceneObjectHierarchyDelta> Delta;
	};

	Vector<RestoredPrefabInstance> newPrefabInstanceData;

	// For each prefab instance load its reference prefab from the disk and check if it changed. If it has changed
	// instantiate the prefab and destroy the current instance. Then apply instance specific changes stored in a
	// prefab diff, if any, as well as restore the original parent and link id (link id of the root prefab instance
	// belongs to the parent prefab if any). Finally fix any handles pointing to the old objects so that they now point
	// to the newly instantiated objects. To the outside world it should be transparent that we just destroyed and then
	// re-created the entire hierarchy from scratch.

	// Need to do this bottom up to ensure I don't destroy the parents before children
	for(auto iter = prefabInstanceRoots.rbegin(); iter != prefabInstanceRoots.rend(); ++iter)
	{
		HSceneObject current = *iter;
		HPrefab prefabLink = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(current->GetPrefabResourceId(), false, ResourceLoadFlag::None));

		if(prefabLink.IsLoaded(false) && prefabLink->GetHash() != current->mPrefabHash)
		{
			// Save IDs, destroy original, create new, restore IDs
			UnorderedMap<UUID, PrefabInstanceData> instanceData;
			RecordInstanceData(current, instanceData);

			HSceneObject parent = current->GetParent();
			SPtr<SceneObjectHierarchyDelta> prefabDelta = current->GetPrefabDelta();

			const SPtr<GameObjectCollection> gameObjectCollection = current->GetOwnerCollection().lock();

			current->Destroy(true);
			HSceneObject newInstance = prefabLink->Clone(gameObjectCollection);

			// When restoring instance IDs it is important to make all the new handles point to the old GameObjectInstanceData.
			// This is because old handles will have different GameObjectHandleData and we have no easy way of accessing it to
			// change to which GameObjectInstanceData it points. But the GameObjectManager ensures that all handles deserialized
			// at once (i.e. during the ::CloneInternal() call above) will share GameObjectHandleData so we can simply replace
			// to what they point to, affecting all of the handles to that object. (In another words, we can modify the
			// new handles at this point, but old ones must keep referencing what they already were.)
			RestoreInstanceData(newInstance, instanceData);

			newPrefabInstanceData.push_back({ newInstance, parent, prefabDelta });
		}
	}

	// Once everything is cloned, apply diffs, restore old parents & link IDs for root.
	for(auto& entry : newPrefabInstanceData)
	{
		// Diffs must be applied after everything is instantiated and instance data restored since it may contain
		// game object handles within or external to its prefab instance.
		if(entry.Delta != nullptr)
			entry.Delta->Apply(entry.NewInstance);

		entry.NewInstance->mPrefabDelta = entry.Delta;

		entry.NewInstance->SetParent(entry.OriginalParent, false);
	}

	// Finally, instantiate everything if the top scene object is live (instantiated)
	if(prefabInstanceRoot->IsInstantiated())
	{
		for(auto& entry : newPrefabInstanceData)
			entry.NewInstance->InstantiateInternal(true);
	}

	GetResources().UnloadAllUnused();
}

void PrefabUtility::AssignPrefabResourceId(const HSceneObject& sceneObject, const UUID& newPrefabResourceId)
{
	const UUID originalResourceId = sceneObject->GetPrefabResourceId();

	sceneObject->IterateHierarchy(
		[&originalResourceId, &newPrefabResourceId](const HSceneObject& sceneObject)
		{
			if(sceneObject->HasFlag(SOF_DontSave))
				return false;

			const UUID& currentResourceId = sceneObject->GetPrefabResourceId();

			// Assign IDs while the resource ID matches, or if the object is not associated with a prefab (i.e. has an empty ID)
			if(!currentResourceId.Empty() && (currentResourceId != originalResourceId))
				return false;

			sceneObject->SetPrefabResourceId(newPrefabResourceId);
			sceneObject->SetPrefabObjectId(sceneObject->GetId());

			return true;
		},
		[](const HComponent& component)
		{
			component->SetPrefabObjectId(component->GetId());
		});
}


void PrefabUtility::AssignPrefabInstanceIds(const HSceneObject& instanceRoot, const HSceneObject& prefabRoot, const UUID& prefabResourceId)
{
	struct MatchingSceneObjects
	{
		MatchingSceneObjects(HSceneObject instance, HSceneObject prefab)
			: InstanceSceneObject(std::move(instance)), PrefabSceneObject(std::move(prefab))
		{ }

		HSceneObject InstanceSceneObject;
		HSceneObject PrefabSceneObject;
	};

	FrameScope frameScope;
	FrameStack<MatchingSceneObjects> todo;
	todo.emplace(instanceRoot, prefabRoot);

	while(!todo.empty())
	{
		MatchingSceneObjects currentMatchingSceneObjects = todo.top();
		todo.pop();

		currentMatchingSceneObjects.InstanceSceneObject->SetPrefabResourceId(prefabResourceId);
		currentMatchingSceneObjects.InstanceSceneObject->SetPrefabObjectId(currentMatchingSceneObjects.PrefabSceneObject.GetId());

		const Vector<HComponent>& instanceComponents = currentMatchingSceneObjects.InstanceSceneObject->GetComponents();
		const Vector<HComponent>& prefabComponents = currentMatchingSceneObjects.PrefabSceneObject->GetComponents();

		const u32 componentCount = (u32)instanceComponents.size();
		if(!B3D_ENSURE(componentCount == prefabComponents.size()))
			return;

		for(u32 componentIndex = 0; componentIndex < componentCount; ++componentIndex)
		{
			const HComponent& instanceComponent = instanceComponents[componentIndex];
			const HComponent& prefabComponent = prefabComponents[componentIndex];

			instanceComponent->SetPrefabObjectId(prefabComponent.GetId());
		}

		const u32 childCount = currentMatchingSceneObjects.InstanceSceneObject->GetChildCount();
		if(!B3D_ENSURE(childCount == currentMatchingSceneObjects.PrefabSceneObject->GetChildCount()))
			return;

		for(u32 childIndex = 0; childIndex < childCount; ++childIndex)
		{
			const HSceneObject& instanceChild = currentMatchingSceneObjects.InstanceSceneObject->GetChild(childIndex);
			const HSceneObject& prefabChild = currentMatchingSceneObjects.PrefabSceneObject->GetChild(childIndex);

			todo.emplace(instanceChild, prefabChild);
		}
	}
}

void PrefabUtility::ClearPrefabIds(const HSceneObject& sceneObject)
{
	const UUID originalResourceId = sceneObject->GetPrefabResourceId();

	sceneObject->SetPrefabObjectId(UUID::kEmpty);
	sceneObject->SetPrefabResourceId(UUID::kEmpty);

	sceneObject->IterateHierarchy(
		[&originalResourceId](const HSceneObject& sceneObject)
		{
			if(sceneObject->GetPrefabResourceId() != originalResourceId)
				return false;

			sceneObject->SetPrefabObjectId(UUID::kEmpty);
			sceneObject->SetPrefabResourceId(UUID::kEmpty);
			return true;
		},
		[](const HComponent& component)
		{
			component->SetPrefabObjectId(UUID::kEmpty);
		});
}

void PrefabUtility::RecordPrefabDelta(const HSceneObject& sceneObject)
{
	if(!B3D_ENSURE(sceneObject.IsValid()))
		return;

	HSceneObject prefabInstanceRoot = sceneObject->GetPrefabInstanceRoot();
	if(!prefabInstanceRoot.IsValid())
		return;

	prefabInstanceRoot->IterateHierarchy(
		[](const HSceneObject& sceneObject)
		{
			sceneObject->SetPrefabDelta(nullptr);

			const UUID& prefabResourceId = sceneObject->GetPrefabResourceId();
			if(!prefabResourceId.Empty())
			{
				HPrefab linkedPrefab = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(prefabResourceId, false, ResourceLoadFlag::None));
				if(linkedPrefab.IsLoaded(false))
					sceneObject->SetPrefabDelta(SceneObjectHierarchyDelta::Create(linkedPrefab->GetRoot(), sceneObject, SceneObjectHierarchyDeltaFlag::PrefabDelta));
				else
				{
					B3D_LOG(Warning, Prefab, "Cannot record prefab delta for scene object '{0}'. Failed to load prefab with ID: '{1}'.", sceneObject.GetId(), prefabResourceId);
				}
			}

			return true;
		},
		nullptr);

	GetResources().UnloadAllUnused();
}

UnorderedMap<UUID, PrefabLinkInformation> PrefabUtility::GetInstanceToPrefabLinkInformationMap(const HSceneObject& sceneObject, bool visitChildPrefabs)
{
	UnorderedMap<UUID, PrefabLinkInformation> output;
	if(!B3D_ENSURE(sceneObject.IsValid()))
		return output;

	const UUID rootPrefabResourceId = sceneObject->GetPrefabResourceId();

	sceneObject->IterateHierarchy([&output, &rootPrefabResourceId, visitChildPrefabs](const HSceneObject& sceneObject)
	{
		if(sceneObject->GetPrefabResourceId() != rootPrefabResourceId && !visitChildPrefabs)
			return false;

		if(!sceneObject->IsPrefabInstance())
			return true;

		const UUID& prefabObjectId = sceneObject->GetPrefabObjectId();
		const UUID& prefabResourceId = sceneObject->GetPrefabResourceId();

		output[sceneObject.GetId()] = PrefabLinkInformation(prefabObjectId, prefabResourceId);
		return true;
	},
	[&output](const HComponent& component) {
		if(!component->IsPrefabInstance())
			return;

		const UUID& prefabObjectId = component->GetPrefabObjectId();
		const UUID& prefabResourceId = component->SceneObject()->GetPrefabResourceId();

		output[component.GetId()] = PrefabLinkInformation(prefabObjectId, prefabResourceId);

	});

	return output;
}

