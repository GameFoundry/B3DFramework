//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefab.h"
#include "Private/RTTI/BsPrefabRTTI.h"
#include "Resources/BsResources.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsPrefabUtility.h"
#include "BsCoreApplication.h"
#include "BsGameObjectCollection.h"
#include "BsSceneManager.h"

using namespace bs;

namespace bs
{
	B3D_LOG_CATEGORY(Prefab)
}

/**
 * Helper class to allow prefab game objects preserve their IDs when they are being updated from an instance of that prefab.
 *
 * This is needed due to the fact that instanced prefabs will have unique game object IDs, and their prefab object IDs will point directly to their parent prefab (ignoring nested prefabs).
 * Game objects in the prefab resource itself must have game object IDs that matches prefab object IDs of instances of that prefab.
 * Additionally these objects will also contain prefab object ID and prefab resource ID links to the first nested prefab, while IDs to root will be empty.
 *
 * To restore the IDs we need to find out which instance game object ID maps to which object in the prefab resource (if any), and set its game object ID, prefab object ID and
 * prefab resource ID accordingly.
 */
class PrefabIdUtility
{
public:
	struct SceneObjectInformation
	{
		SceneObjectInformation(const HSceneObject& sceneObject, const UUID& parentPrefabResourceId, i32 prefabNestingLevel)
			: SceneObject(sceneObject), ParentPrefabResourceId(parentPrefabResourceId), ParentNestingLevel(prefabNestingLevel)
		{}

		HSceneObject SceneObject;
		UUID ParentPrefabResourceId;
		i32 ParentNestingLevel = 0;
	};

	struct PrefabInformation
	{
		HPrefab Prefab;
		UnorderedMap<UUID, PrefabLinkInformation> PrefabHierarchyIds; /**< Game object ID -> { Prefab object ID, prefab resource ID } map for all objects in the prefab hierarchy. */
	};

	/**
	 * Deduces game object ID to used when referencing the object in the internal prefab hierarchy. This will be the matching prefab object ID, if one is provided and still
	 * exists in the current prefab hierarchy, or a brand new ID.
	 *
	 * @param gameObject							Game object instance for which to deduce the ID.
	 * @param rootPrefabOriginalHierarchyIds		Game object ID -> { prefab object ID, prefab resource ID } mapping for all objects in the original internal prefab hierarchy.
	 *												This is the prefab hierarchy whose internals will contain the provided game object.
	 * @return										Game object ID to assign to the game object.
	 */
	static UUID DeduceInternalPrefabGameObjectId(const HGameObject& gameObject, const UnorderedMap<UUID, PrefabLinkInformation>& rootPrefabOriginalHierarchyIds);

	/**
	 * Deduces original prefab object ID and prefab resource ID, as they would be stored in the internal prefab resource, based on a prefab instance and original prefab hierarchy. 
	 *
	 * @param gameObject							Game object instance for which to deduce the IDs.
	 * @param gameObjectIdInPrefab					Game object ID that will be used for @p gameObject in the internal prefab hierarchy.
	 * @param associatedSceneObjectInformation		Information about the parent scene object that owns @p gameObject. If @p gameObject is a SceneObject, this should point to the same object.
	 * @param associatedPrefabOriginalHierarchyIds	Game object ID -> { prefab object ID, prefab resource ID } mapping for all objects in the original internal prefab hierarchy.
     *												This is the prefab hierarchy of the prefab that is currently linked to the associated scene object.
	 * @return										Prefab object ID and prefab resource ID, as they are stored in the associated prefab. Or empty IDs if not found.
	 */
	static PrefabLinkInformation DeduceInternalPrefabLinkInformation(const HGameObject& gameObject, const UUID& gameObjectIdInPrefab, const SceneObjectInformation& associatedSceneObjectInformation, const UnorderedMap<UUID, PrefabLinkInformation>& associatedPrefabOriginalHierarchyIds);

	/**
	 * Assigns original prefab game object ID, prefab object ID and prefab resource ID to the provided game object. The original prefab IDs refers to the IDs stored when a game object
	 * is part of the internal prefab hierarchy.
	 *
	 * @param gameObject					Game object to which to assign the IDs to.
	 * @param gameObjectIdInPrefab			ID of the game object within the prefab hierarchy, as output by DeduceOriginalPrefabIds().
	 * @param linkInformationInPrefab		Link information to nested prefabs, as output by DeduceOriginalPrefabIds().
	 * @param rootPrefabResourceId			ID of the prefab resource that the resulting hierarchy will be stored in.
	 * @param nestingLevel					Current depth of nested prefab we are at. 0 for root, 1 for first nested prefab, etc.
	 */
	static void AssignInternalPrefabIds(const GameObjectHandleBase& gameObject, const UUID& gameObjectIdInPrefab, const PrefabLinkInformation& linkInformationInPrefab, const UUID& rootPrefabResourceId, i32 nestingLevel);

	/**
	 * Updates all objects in @p instanceHierarchyRoot with IDs so they match previously stored prefab hierarchy. This should be called on a clone of the
	 * scene object hierarchy that is being passed when creating or updating a prefab. The hierarchy with patched IDs can then be used for internal
	 * prefab hierarchy (i.e. replacement for @p rootPrefabOriginalHierarchyRoot).
	 *
	 * Returns a map containing a remapping from instance game object IDs to prefab game object IDs, which may be used for updating the original
	 * scene object hierarchy, so it links to the created/updated prefab hierarchy.
	 */
	static UnorderedMap<UUID, UUID> RestoreOriginalPrefabIds(const UUID& rootPrefabResourceId, const HSceneObject& rootPrefabOriginalHierarchyRoot, const HSceneObject& instanceHierarchyRoot);
};

UUID PrefabIdUtility::DeduceInternalPrefabGameObjectId(const HGameObject& gameObject, const UnorderedMap<UUID, PrefabLinkInformation>& rootPrefabOriginalHierarchyIds)
{
	// Try to find or generate a new scene object ID and prefab object ID
	const UUID& instancePrefabObjectId = gameObject->GetPrefabObjectId();
	if(gameObject->GetPrefabObjectId().Empty()) // This is a new object that was never part of a prefab (or its link was broken, in which case we treat it as new)
		return UUIDGenerator::GenerateRandom();
	else
	{
		auto found = rootPrefabOriginalHierarchyIds.find(instancePrefabObjectId);
		if(found != rootPrefabOriginalHierarchyIds.end()) // This is an object that was previously part of the prefab, retrieve its original ID
			return instancePrefabObjectId;
		else // This is an object with a prefab link, but not previously part of this prefab, which means its a newly added nested prefab and needs a new ID
			return UUIDGenerator::GenerateRandom();
	}
}

PrefabLinkInformation PrefabIdUtility::DeduceInternalPrefabLinkInformation(const HGameObject& gameObject, const UUID& gameObjectIdInPrefab, const SceneObjectInformation& associatedSceneObjectInformation, const UnorderedMap<UUID, PrefabLinkInformation>& associatedPrefabOriginalHierarchyIds)
{
	PrefabLinkInformation prefabLinkInformation;

	const UUID& instancePrefabObjectId = gameObject->GetPrefabObjectId();
	if(gameObject->GetPrefabObjectId().Empty()) // Object with no prefab link inherits the parent prefab resource ID
	{
		prefabLinkInformation.PrefabObjectId = gameObjectIdInPrefab;
		prefabLinkInformation.PrefabResourceId = associatedSceneObjectInformation.ParentPrefabResourceId;
	}
	else
	{
		auto found = associatedPrefabOriginalHierarchyIds.find(instancePrefabObjectId);
		if(found != associatedPrefabOriginalHierarchyIds.end()) // This is an object that was previously part of the prefab, retrieve its original IDs
		{
			prefabLinkInformation.PrefabObjectId = found->second.PrefabObjectId;
			prefabLinkInformation.PrefabResourceId = found->second.PrefabResourceId;
		}
		else // This is an object with a prefab link, but not previously part of this prefab, which means its a newly added nested prefab. Those keep pointing to their original prefabs.
		{
			prefabLinkInformation.PrefabObjectId = gameObject->GetPrefabObjectId();
			prefabLinkInformation.PrefabResourceId = associatedSceneObjectInformation.SceneObject->GetPrefabResourceId();

			if(!B3D_ENSURE(!prefabLinkInformation.PrefabResourceId.Empty()))
				prefabLinkInformation.PrefabResourceId = associatedSceneObjectInformation.ParentPrefabResourceId; // Fallback, shouldn't happen
		}
	}

	return prefabLinkInformation;
}

void PrefabIdUtility::AssignInternalPrefabIds(const GameObjectHandleBase& gameObject, const UUID& gameObjectIdInPrefab, const PrefabLinkInformation& linkInformationInPrefab, const UUID& rootPrefabResourceId, i32 nestingLevel)
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

UnorderedMap<UUID, UUID> PrefabIdUtility::RestoreOriginalPrefabIds(const UUID& rootPrefabResourceId, const HSceneObject& rootPrefabOriginalHierarchyRoot, const HSceneObject& instanceHierarchyRoot)
{
	UnorderedMap<UUID, PrefabLinkInformation> rootPrefabOriginalHierarchyIds;
	if(rootPrefabOriginalHierarchyRoot.IsValid())
		rootPrefabOriginalHierarchyIds = PrefabUtility::GetInstanceToPrefabLinkInformationMap(rootPrefabOriginalHierarchyRoot, true);

	UnorderedMap<UUID, PrefabInformation> prefabCache;

	// Finds the prefab associated with the scene object instance, and populates a lookup from instance ID to { prefab object id, prefab resource id }.
	// If a prefab has already been processed, cached data will be returned.
	auto fnGetOrPopulatePrefabHierarchyIds = [&prefabCache](const HSceneObject& sceneObject) -> const UnorderedMap<UUID, PrefabLinkInformation>& {
		static const UnorderedMap<UUID, PrefabLinkInformation> kEmptyMap;

		const UUID prefabResourceId = sceneObject->GetPrefabResourceId();
		if(prefabResourceId.Empty())
			return kEmptyMap;

		if(auto found = prefabCache.find(prefabResourceId); found != prefabCache.end())
			return found->second.PrefabHierarchyIds;

		PrefabInformation prefabInformation;
		prefabInformation.Prefab = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(prefabResourceId, false, ResourceLoadFlag::None));

		if(prefabInformation.Prefab.IsLoaded(false))
			prefabInformation.PrefabHierarchyIds = PrefabUtility::GetInstanceToPrefabLinkInformationMap(prefabInformation.Prefab->GetRoot(), true);

		auto result = prefabCache.insert(std::make_pair(prefabResourceId, std::move(prefabInformation)));
		return result.first->second.PrefabHierarchyIds;
	};

	FrameScope frameScope;
	FrameStack<SceneObjectInformation> todo;
	todo.emplace(instanceHierarchyRoot, UUID::kEmpty, -1);

	UnorderedMap<UUID, UUID> remappedGameObjectIDs;
	while(!todo.empty())
	{
		const SceneObjectInformation currentObjectToProcess = todo.top();
		todo.pop();

		HSceneObject sceneObject = currentObjectToProcess.SceneObject;

		// Deduce original IDs for the scene object
		const UUID originalSceneObjectId = currentObjectToProcess.SceneObject.GetId();
		const UUID sceneObjectIdInPrefab = DeduceInternalPrefabGameObjectId(currentObjectToProcess.SceneObject, rootPrefabOriginalHierarchyIds);

		const UnorderedMap<UUID, PrefabLinkInformation>& associatedPrefabOriginalHierarchyIds = fnGetOrPopulatePrefabHierarchyIds(currentObjectToProcess.SceneObject);
		const PrefabLinkInformation sceneObjectLinkInformation = DeduceInternalPrefabLinkInformation(currentObjectToProcess.SceneObject, sceneObjectIdInPrefab, currentObjectToProcess, associatedPrefabOriginalHierarchyIds);

		// If we've reached a new prefab instance, increment nesting level.
		i32 nestingLevel = currentObjectToProcess.ParentNestingLevel;
		if(nestingLevel == -1) // -1 means parent resource ID has not been assigned yet
			nestingLevel = 0;
		else if(sceneObjectLinkInformation.PrefabResourceId != currentObjectToProcess.ParentPrefabResourceId)
			nestingLevel++;

		// Deduce and assign component IDs
		// Note: Important to process components before the scene object, as we depend on the owning scene object's IDs to be unchanged
		for(auto& component : sceneObject->GetComponents())
		{
			const UUID originalComponentId = component.GetId();
			const UUID componentIdInPrefab = DeduceInternalPrefabGameObjectId(component, rootPrefabOriginalHierarchyIds);
			const PrefabLinkInformation componentLinkInformation = DeduceInternalPrefabLinkInformation(component, componentIdInPrefab, currentObjectToProcess, associatedPrefabOriginalHierarchyIds);

			AssignInternalPrefabIds(component, componentIdInPrefab, componentLinkInformation, rootPrefabResourceId, nestingLevel);

			remappedGameObjectIDs[originalComponentId] = componentIdInPrefab;
		}

		// Assign scene object IDs
		{
			AssignInternalPrefabIds(sceneObject, sceneObjectIdInPrefab, sceneObjectLinkInformation, rootPrefabResourceId, nestingLevel);

			remappedGameObjectIDs[originalSceneObjectId] = sceneObjectIdInPrefab;
		}

		const u32 childCount = currentObjectToProcess.SceneObject->GetChildCount();
		for(u32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			const HSceneObject childSceneObject = currentObjectToProcess.SceneObject->GetChild(childIndex);
			todo.emplace(childSceneObject, sceneObjectLinkInformation.PrefabResourceId, nestingLevel);
		}
	}

	return remappedGameObjectIDs;
}

Prefab::Prefab()
	: Resource(false), mGameObjectCollection(GameObjectCollection::Create())
{
}

Prefab::~Prefab()
{
	if(mRoot != nullptr)
		mRoot->Destroy(true);
}

HPrefab Prefab::Create(const HSceneObject& sceneObject, bool isScene)
{
	SPtr<Prefab> newPrefab = CreateEmpty();
	newPrefab->mIsScene = isScene;
	newPrefab->mUUID = UUIDGenerator::GenerateRandom(); // TODO - This should be done automatically on resource creation

	newPrefab->Initialize(sceneObject);

	return B3DStaticResourceCast<Prefab>(GetResources().CreateResourceHandle(newPrefab, newPrefab->mUUID));
}

SPtr<Prefab> Prefab::CreateEmpty()
{
	SPtr<Prefab> newPrefab = B3DMakeSharedFromExisting<Prefab>(new(B3DAllocate<Prefab>()) Prefab());
	newPrefab->SetShared(newPrefab);

	return newPrefab;
}

void Prefab::Initialize(const HSceneObject& sceneObject)
{
	sceneObject->mPrefabDelta = nullptr;

	const SPtr<GameObjectCollection> newGameObjectCollection = GameObjectCollection::Create();
	HSceneObject newRoot = sceneObject->Clone(newGameObjectCollection, false, true);
	newRoot->mParent = nullptr;

	// Remove objects with "dont save" flag
	FrameScope frameScope;
	FrameVector<HSceneObject> sceneObjectsToDestroy;
	newRoot->IterateHierarchy([&sceneObjectsToDestroy](const HSceneObject& sceneObject) {
		if(sceneObject->HasFlag(SOF_DontSave))
		{
			sceneObjectsToDestroy.push_back(sceneObject);
			return false;
		}

		return true;
	}, nullptr);

	for(const auto& entry : sceneObjectsToDestroy)
		entry->Destroy();

	// Ensure the prefab hierarchy keeps the original ids
	UnorderedMap<UUID, UUID> remappedGameObjectIDs = PrefabIdUtility::RestoreOriginalPrefabIds(mUUID, mRoot, newRoot);

	// Ensure the instance hierarchy links to this prefab
	// TODO - This shouldn't be done if hierarchy is part of some other prefab instance. In that case we need to clear the instance prefab delta
	// and rely on UpdateFromPrefab to update the parent prefab to the version with correct IDs.
	B3D_ASSERT(mUUID != UUID::kEmpty);
	sceneObject->IterateHierarchy([this, &remappedGameObjectIDs](const HSceneObject& sceneObject)
	{
		if(sceneObject->HasFlag(SOF_DontSave))
			return false;

		if(auto found = remappedGameObjectIDs.find(sceneObject.GetId()); B3D_ENSURE(found != remappedGameObjectIDs.end()))
		{
			sceneObject->SetPrefabObjectId(found->second);
			sceneObject->SetPrefabResourceId(mUUID);
		}

		return true;
	},
	[this, &remappedGameObjectIDs](const HComponent& component)
	{
		if(auto found = remappedGameObjectIDs.find(component.GetId()); B3D_ENSURE(found != remappedGameObjectIDs.end()))
			component->SetPrefabObjectId(found->second);
	});

	// Generate deltas for nested prefabs
	newRoot->IterateHierarchy([this](const HSceneObject& sceneObject) {
		if(sceneObject->IsPrefabInstanceRoot()) // TODO - Should skip any that aren't first level of nested object
			PrefabUtility::RecordPrefabDelta(sceneObject);

		return true;
	}, nullptr);

	if(mRoot.IsValid())
		mRoot->Destroy(true);

	mRoot = newRoot; // Note: PrefabIdUtility::RestoreOriginalPrefabIds() depends on this being changed after it has been called, as it may try to access the original prefab root
	mGameObjectCollection = newGameObjectCollection;
	mPrefabVersion = UUIDGenerator::GenerateRandom();
}

void Prefab::Update(const HSceneObject& sceneObject)
{
	Initialize(sceneObject);
}

void Prefab::UpdateChildInstancesInternal() const
{
	if(!mRoot.IsValid())
		return;

	mRoot->IterateHierarchy([this](const HSceneObject& sceneObject)
	{
		if(sceneObject->IsPrefabInstanceRoot())
			PrefabUtility::UpdateInstanceFromPrefab(sceneObject);

		return true;
	}, nullptr);
}

HSceneObject Prefab::Instantiate(const SPtr<SceneInstance>& sceneInstance, bool preserveIds) const
{
	if(mRoot == nullptr)
		return HSceneObject();

#if B3D_IS_ENGINE
	if(GetCoreApplication().IsEditor())
	{
		// Update any child prefab instances in case their prefabs changed
		UpdateChildInstancesInternal();
	}
#endif

	SPtr<GameObjectCollection> gameObjectCollection;
	if(sceneInstance != nullptr)
		gameObjectCollection = sceneInstance->GetGameObjectCollection();
	else
		gameObjectCollection = GameObjectCollection::Create();

	HSceneObject clone = Clone(gameObjectCollection, preserveIds);
	PrefabUtility::AssignPrefabInstanceIds(clone, mRoot, mUUID);

	SPtr<SceneInstance> finalSceneInstance;
	if(sceneInstance != nullptr)
	{
		finalSceneInstance = sceneInstance;
		clone->SetParent(sceneInstance->GetRoot());
	}
	else
		finalSceneInstance = SceneInstance::Create("PrefabInstance", clone);

	clone->InstantiateInternal();

	return clone;
}

HSceneObject Prefab::Clone(const SPtr<GameObjectCollection>& cloneOwnerCollection, bool preserveIds) const
{
	if(mRoot == nullptr)
		return HSceneObject();

	mRoot->SetPrefabVersion(mPrefabVersion); // TODO - Might make sense to assign this to the entire hierarchy. Also for internal hierarchy, it should be set when internal hierarchy is updated.
	return mRoot->Clone(cloneOwnerCollection, false, preserveIds);
}

RTTITypeBase* Prefab::GetRttiStatic()
{
	return PrefabRTTI::Instance();
}

RTTITypeBase* Prefab::GetRtti() const
{
	return Prefab::GetRttiStatic();
}
