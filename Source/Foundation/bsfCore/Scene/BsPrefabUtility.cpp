//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefabUtility.h"

#include "BsGameObjectCollection.h"
#include "BsPrefabIdRemapper.h"
#include "BsSceneManager.h"
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
	sceneObject->IterateHierarchy(
		[&outInstanceData](const HSceneObject& sceneObject)
		{
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
	SPtr<GameObjectCollection> gameObjectCollection = sceneObject->GetOwnerCollection().lock();
	if(!B3D_ENSURE(gameObjectCollection))
		return;

	sceneObject->IterateHierarchy(
		[&instanceData, &gameObjectCollection](const HSceneObject& sceneObject)
		{
			const UUID& prefabObjectId = sceneObject->GetPrefabObjectId();
			if(!prefabObjectId.Empty())
			{
				if(auto found = instanceData.find(prefabObjectId); found != instanceData.end())
				{
					sceneObject->SetId(found->second.Id); // ID must be set before calling ReplaceGameObjectInstance 

					HSceneObject sceneObjectMutableHandle = sceneObject;
					gameObjectCollection->ReplaceGameObjectInstance(sceneObjectMutableHandle, found->second.InstanceData);
				}
			}

			return true;
		},
		[&instanceData, &gameObjectCollection](const HComponent& component)
		{
			const UUID& prefabObjectId = component->GetPrefabObjectId();
			if(!prefabObjectId.Empty())
			{
				if(auto found = instanceData.find(prefabObjectId); found != instanceData.end())
				{
					component->SetId(found->second.Id); // ID must be set before calling ReplaceGameObjectInstance 

					HComponent componentMutableHandle = component;
					gameObjectCollection->ReplaceGameObjectInstance(componentMutableHandle, found->second.InstanceData);
				}
			}
		});
}

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

HPrefab PrefabCache::FindOrLoadPrefab(const UUID& prefabId)
{
	if(auto found = mPrefabs.find(prefabId); found != mPrefabs.end())
		return found->second;

	HPrefab prefab = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(prefabId, false, ResourceLoadFlag::None));

	if(!prefab.IsLoaded(false))
	{
		B3D_LOG(Error, Scene, "Prefab with ID: {0} cannot be loaded.", prefabId);
		return nullptr;
	}

	mPrefabs.insert(std::make_pair(prefab->GetId(), prefab));
	return prefab;
}

void PrefabCache::AddToCache(const HPrefab& prefab)
{
	mPrefabs.insert(std::make_pair(prefab.GetId(), prefab));
}

bool PrefabCache::ExistsInCache(const UUID& prefabId)
{
	return mPrefabs.find(prefabId) != mPrefabs.end();
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

	//if(prefabDelta != nullptr)
	//	prefabDelta->Apply(newInstance, SceneObjectHierarchyDeltaFlag::PrefabDelta);

	newInstance->SetLocalTransform(transform);
	newInstance->mPrefabDelta = prefabDelta;

	return newInstance;
}

void PrefabUtility::UpdatePrefab(const HPrefab& prefabToUpdate, const HSceneObject& sceneObjectToUpdateWith)
{
	if(!B3D_ENSURE(prefabToUpdate.IsLoaded(false)))
		return;

	FrameScope frameScope;

	// TODO - Move outside
	auto fnFindInstanceInPrefab = [](const HSceneObject& sceneObject) -> Optional<PrefabInstanceRoot> {
		if(!sceneObject->IsPrefabInstance())
			return {};

		const UUID prefabId = sceneObject->GetPrefabResourceId();
		const HPrefab prefab = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(prefabId, false, ResourceLoadFlag::None));

		if(!prefab.IsLoaded()) // TODO - Log error?
			return {};

		const SPtr<GameObjectCollection>& gameObjectCollection = prefab->GetGameObjectCollection();
		if(!B3D_ENSURE(gameObjectCollection))
			return {};

		HGameObject instanceInPrefab;
		if(!gameObjectCollection->TryGetObject(sceneObject->GetPrefabObjectId(), instanceInPrefab))
			return {};

		return PrefabInstanceRoot(prefab, B3DStaticGameObjectCast<SceneObject>(instanceInPrefab), nullptr);
	};

	// Is the provided hierarchy already an instance of a prefab? If so, record a list of prefab instances that are parents of the instance we're updating. We'll need this below.
	FrameVector<PrefabInstanceRoot> prefabInstanceParents;
	if(sceneObjectToUpdateWith->IsPrefabInstance())
	{
		HSceneObject currentSceneObject = sceneObjectToUpdateWith;
		HPrefab parentPrefab = nullptr;
		while(Optional<PrefabInstanceRoot> maybePrefabInstanceRoot = fnFindInstanceInPrefab(currentSceneObject))
		{
			currentSceneObject = maybePrefabInstanceRoot->SceneObjectInParentPrefab;
			parentPrefab = maybePrefabInstanceRoot->ParentPrefab;

			// Current prefab becomes the prefab to update from for the parent
			if(!prefabInstanceParents.empty())
				prefabInstanceParents.back().PrefabToUpdateFrom = parentPrefab;

			if(parentPrefab == prefabToUpdate)
				break;

			prefabInstanceParents.push_back(std::move(maybePrefabInstanceRoot.value()));
		}
	}

	// TODO - Consider moving much of the logic from ReplaceInternalHierarchy to here
	// TODO - Can the above remapping be utilized to handle assignment of all instance IDs after creating/updating prefab?
	// - Probably. If UpdateFromPrefab is called on the instance itself, it should remap
	// - But that doesn't work for brand new instances unless we also perform some kind of ID remapping
	UnorderedMap<UUID, UUID> instanceIdToUpdatePrefabId = prefabToUpdate->ReplaceInternalHierarchy(sceneObjectToUpdateWith);

	// If root prefab contains an object that was an instance modification, and is now part of the update prefab, it's prefab object ID will change
	// (as original object IDs doesn't link to anything). So we need to know to remap that prefab object ID to the new prefab object ID.
	UnorderedMap<UUID, UUID> rootPrefabIdToUpdatePrefabId;
	sceneObjectToUpdateWith->IterateHierarchy([&instanceIdToUpdatePrefabId, &rootPrefabIdToUpdatePrefabId](const HSceneObject& child) mutable -> bool
	{
		if(auto found = instanceIdToUpdatePrefabId.find(child.GetId()); found != instanceIdToUpdatePrefabId.end())
			rootPrefabIdToUpdatePrefabId.insert(std::make_pair(child->GetPrefabObjectId(), found->second));

		return true;
	},
	   [&instanceIdToUpdatePrefabId, &rootPrefabIdToUpdatePrefabId](const HComponent& component) mutable -> void
	   {
		   if(auto found = instanceIdToUpdatePrefabId.find(component.GetId()); found != instanceIdToUpdatePrefabId.end())
			   rootPrefabIdToUpdatePrefabId.insert(std::make_pair(component->GetPrefabObjectId(), found->second));
	   },
	   true);

	// Update all live prefab instances
	PrefabCache prefabCache;
	prefabCache.AddToCache(prefabToUpdate);

	// If our instance is part of another prefab's instance, we need to make sure the instance points to the correct prefab object IDs.
	// This is needed because if an object is an instance modification of some parent prefab instance,
	// yet this operation makes it a part of the nested prefab instance, it will be assigned a unique ID, in which case we need
	// to update the original prefab instance's prefab object ID, so it continues linking to the object.
	//
	// e.g. If we have a scene such as this:
	// Prefab #1 Instance
	//  - Prefab #2 Instance
	//   - My scene object
	//
	// After adding "My scene object" if we update Prefab #1, "My scene object" will become an instance modification
	// of Prefab #1, and within Prefab #1 will have a prefab object ID that links to itself.
	// 
	// If we then later update Prefab #2, that same object will now become part of Prefab #2. This means we need to
	// update Prefab #1, so instance of "My scene object" links to the object in Prefab #2.
	for(auto it = prefabInstanceParents.rbegin(); it != prefabInstanceParents.rend(); ++it) // Start from most nested
	{
		const PrefabInstanceRoot& prefabInstanceRoot = *it;

		// Ensure the prefab IDs match to what was output when we updated the prefab. This will ensure the scene object instance hierarchy links to the
		// prefab, and correctly set up IDs in the case when an object was an instance modification of part of a parent prefab, and we just added it to
		// a child's prefab.
		prefabInstanceRoot.SceneObjectInParentPrefab->IterateHierarchy(
			[&rootPrefabIdToUpdatePrefabId, prefabId = prefabInstanceRoot.PrefabToUpdateFrom.GetId()](const HSceneObject& sceneObject) {
			if(sceneObject->HasFlag(SOF_DontSave))
				return false;

			if(auto found = rootPrefabIdToUpdatePrefabId.find(sceneObject.GetId()); B3D_ENSURE(found != rootPrefabIdToUpdatePrefabId.end()))
			{
				sceneObject->SetPrefabObjectId(found->second);
				sceneObject->SetPrefabResourceId(prefabId);
			}

			return true; },
		  [&rootPrefabIdToUpdatePrefabId](const HComponent& component) {
			  if(auto found = rootPrefabIdToUpdatePrefabId.find(component.GetId()); B3D_ENSURE(found != rootPrefabIdToUpdatePrefabId.end()))
				  component->SetPrefabObjectId(found->second);
		});

		HSceneObject newHierarchy = UpdateInstanceFromPrefab(prefabInstanceRoot.SceneObjectInParentPrefab, *prefabInstanceRoot.PrefabToUpdateFrom);
		if(!B3D_ENSURE(newHierarchy != nullptr))
			continue;

		// Make sure rootPrefabIdToUpdatePrefabId is kept up to date. If any of its prefab object IDs were just updated, remap them to the updated values,
		// so they always point to the top-most nested prefab.
		// This way when we reach the root prefab (which is the only prefab that contains the game objects that will map to these IDs), we can correctly map to its nested prefab object ID.
		const UnorderedMap<UUID, UUID> prefabObjectToInstanceIdMap = GetPrefabToInstanceIdMap(prefabInstanceRoot.SceneObjectInParentPrefab, true);
		for(auto& entry : rootPrefabIdToUpdatePrefabId)
		{
			if(auto found = prefabObjectToInstanceIdMap.find(entry.second); found != prefabObjectToInstanceIdMap.end())
				entry.second = found->second;
		}

		// Note: We purposefully don't tick the prefab version here, and do another update that takes care of that below. This is because
		// the prefab could contain multiple instances of our update prefab, and we're only updating one that's related to @p root
	}

	const UnorderedMap<SceneInstance*, WeakSPtr<SceneInstance>>& sceneInstances = GetSceneManager().GetAllSceneInstances();
	for(const auto& pair : sceneInstances)
	{
		SPtr<SceneInstance> scene = pair.second.lock();
		if(B3D_ENSURE(scene != nullptr))
		{
			FrameVector<UUID> parentPrefabChain;
			UpdateNestedPrefabInstancesRecursive(scene->GetRoot(), prefabCache);
		}
	}

	UnorderedSet<Prefab*> livePrefabs = PrefabManager::Instance().GetLivePrefabs();
	for(const auto& entry : livePrefabs)
	{
		const UUID& prefabResourceId = entry->GetId();
		if(prefabCache.ExistsInCache(prefabResourceId))
			continue;

		if(UpdateNestedPrefabInstancesRecursive(entry->GetRoot(), prefabCache))
		{
			entry->TickPrefabVersion();
			entry->RecordNestedPrefabInstanceDeltas();
		}
	}
}

bool PrefabUtility::UpdateNestedPrefabInstances(const HSceneObject& sceneObject)
{
	FrameScope frameScope;

	PrefabCache prefabCache;
	return UpdateNestedPrefabInstancesRecursive(sceneObject, prefabCache);
}

bool PrefabUtility::UpdateNestedPrefabInstancesRecursive(const HSceneObject& root, PrefabCache& inOutPrefabCache)
{
	if(!root.IsValid())
		return false;

	// Find all the prefab instances we need to update. This includes the hierarchy of @p root, but also of any prefab that's referenced by root,
	// or nested prefabs of those prefabs, recursively
	FrameVector<PrefabInstanceRoot> prefabInstanceRootsToUpdate;
	bool foundCircularDependency = false;
	auto fnVisitPrefabInstances = [&inOutPrefabCache, &prefabInstanceRootsToUpdate, &foundCircularDependency](const HSceneObject& instanceRoot, const HPrefab& parentPrefab, FrameVector<UUID>& inOutParentPrefabChain, auto& fnVisitPrefabInstances) -> void {

		instanceRoot->IterateHierarchy([&inOutPrefabCache, &prefabInstanceRootsToUpdate, &foundCircularDependency, &parentPrefab, &inOutParentPrefabChain, &fnVisitPrefabInstances](const HSceneObject& child) mutable -> bool
		{
			if(!child->IsPrefabInstanceRoot())
				return true;

			const UUID& nestedPrefabId = child->GetPrefabResourceId();
			if(auto found = std::find(inOutParentPrefabChain.begin(), inOutParentPrefabChain.end(), nestedPrefabId); found != inOutParentPrefabChain.end())
			{
				B3D_LOG(Error, Scene, "Failed to update instance from prefab. Detected circular dependency for prefab with ID:{0}.)", nestedPrefabId);
				foundCircularDependency = true;
				return false;
			}

			const HPrefab nestedPrefab = inOutPrefabCache.FindOrLoadPrefab(nestedPrefabId);
			if(nestedPrefab.IsLoaded(false))
			{
				FrameVector<UUID> parentPrefabChainCopy = inOutParentPrefabChain;
				parentPrefabChainCopy.push_back(nestedPrefabId);

				HSceneObject nestedPrefabInternalsRoot = nestedPrefab->GetRoot();
				if(B3D_ENSURE(nestedPrefabInternalsRoot.IsValid()))
					fnVisitPrefabInstances(nestedPrefabInternalsRoot, nestedPrefab, parentPrefabChainCopy, fnVisitPrefabInstances);
			}

			prefabInstanceRootsToUpdate.emplace_back(parentPrefab, child, nestedPrefab);
			return true;

		},
		nullptr, false);
	};

	FrameVector<UUID> parentPrefabChain;
	fnVisitPrefabInstances(root, nullptr, parentPrefabChain, fnVisitPrefabInstances);

	if(foundCircularDependency)
		return false;

	if(prefabInstanceRootsToUpdate.empty())
		return false;

	bool isAnythingModified = false;
	FrameUnorderedMap<UUID, HPrefab> updatedPrefabs;
	for(const auto& entry : prefabInstanceRootsToUpdate)
	{
		HSceneObject objectToUpdate = entry.SceneObjectInParentPrefab;
		if(!B3D_ENSURE(objectToUpdate.IsValid()))
			continue;

		if(!entry.PrefabToUpdateFrom.IsLoaded(false))
			continue;

		if(UpdateInstanceFromPrefab(objectToUpdate, *entry.PrefabToUpdateFrom) != nullptr)
		{
			isAnythingModified = true;

			if(entry.ParentPrefab.IsLoaded(false))
			{
				entry.ParentPrefab->TickPrefabVersion(); // Technically we only need to tick this once, but this is easier
				updatedPrefabs.insert(std::make_pair(entry.ParentPrefab.GetId(), entry.ParentPrefab));
			}
		}
	}

	for(const auto& entry : updatedPrefabs)
		entry.second->RecordNestedPrefabInstanceDeltas();

	return isAnythingModified;
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
