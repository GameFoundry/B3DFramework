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

HSceneObject PrefabUtility::UpdateInstanceFromPrefab(const HSceneObject& instance, const Prefab& prefab)
{
	if(!B3D_ENSURE(instance.IsValid()))
		return HSceneObject();

	if(!instance->IsPrefabInstanceRoot())
	{
		B3D_LOG(Warning, Scene, "Cannot update scene object from prefab. Provided scene object '{0}' ({1}) is not a prefab instance root.", instance->GetName(), instance.GetId());
		return HSceneObject();
	}

	if(instance->GetPrefabResourceId() != prefab.GetId())
	{
		B3D_LOG(Warning, Scene, "Cannot update scene object from prefab. Provided scene object '{0}' ({1}) is referencing prefab '{2}', but the provided prefab is '{3}'.", instance->GetName(), instance.GetId(), instance->GetPrefabResourceId(), prefab.GetId());
		return HSceneObject();
	}

	if(instance->GetPrefabVersion() == prefab.GetPrefabVersion())
		return HSceneObject();

	// Save IDs, destroy original, create new, restore IDs
	UnorderedMap<UUID, PrefabInstanceData> instanceData;
	RecordInstanceData(instance, instanceData);

	HSceneObject parent = instance->GetParent();
	SPtr<SceneObjectHierarchyDelta> prefabDelta = instance->GetPrefabDelta();
	Transform transform = instance->GetLocalTransform();

	const SPtr<GameObjectCollection> gameObjectCollection = instance->GetOwnerCollection().lock();

	instance->Destroy(true);
	HSceneObject newInstance = prefab.Clone(gameObjectCollection);
	AssignPrefabInstanceIds(newInstance, prefab.GetRoot(), prefab.GetId());

	// When restoring instance IDs it is important to make all the new handles point to the old GameObjectInstanceData.
	// This is because old handles will have different GameObjectHandleData and we have no easy way of accessing it to
	// change to which GameObjectInstanceData it points. But the GameObjectCollection ensures that all handles deserialized
	// at once (i.e. during the Clone() call above) will share GameObjectHandleData so we can simply replace
	// to what they point to, affecting all of the handles to that object. (In another words, we can modify the
	// new handles at this point, but old ones must keep referencing what they already were.)
	RestoreInstanceData(newInstance, instanceData);

	newInstance->SetParent(parent, false);

	if(prefabDelta != nullptr)
		prefabDelta->Apply(newInstance, SceneObjectHierarchyDeltaFlag::PrefabDelta);

	newInstance->SetLocalTransform(transform);
	newInstance->mPrefabDelta = prefabDelta;

	return newInstance;
}

void PrefabUtility::UpdateInstanceFromPrefab(const HSceneObject& sceneObject)
{
	if(!B3D_ENSURE(sceneObject.IsValid()))
		return;

	HSceneObject prefabInstanceRoot = sceneObject->GetPrefabInstanceRoot();
	if(!prefabInstanceRoot.IsValid())
		return;

	const bool isInstantiated = prefabInstanceRoot->IsInstantiated();
	const UUID prefabResourceId = prefabInstanceRoot->GetPrefabResourceId();
	HPrefab prefab = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(prefabResourceId, false, ResourceLoadFlag::None));

	if(!prefab.IsLoaded(false))
	{
		B3D_LOG(Warning, Scene, "Cannot update scene object from prefab. Prefab resource with ID: '{0}' cannot be found.", prefabResourceId);
		return;
	}

	HSceneObject newInstance = UpdateInstanceFromPrefab(sceneObject, *prefab);
	if(newInstance == nullptr)
		return;

	// Load everything referenced by the new prefab objects
	GetResources().LoadFromUuid(prefabResourceId, true, ResourceLoadFlag::LoadDependencies);

	if(isInstantiated)
		newInstance->InstantiateInternal();

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

UnorderedMap<UUID, UUID> PrefabUtility::GetPrefabToInstanceIdMap(const HSceneObject& sceneObject, bool visitChildPrefabs)
{
	UnorderedMap<UUID, UUID> output;
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
		output[prefabObjectId] = sceneObject.GetId();

		return true;
	},
	[&output](const HComponent& component) {
		if(!component->IsPrefabInstance())
			return;

		const UUID& prefabObjectId = component->GetPrefabObjectId();
		output[prefabObjectId] = component.GetId();

	});

	return output;
}

