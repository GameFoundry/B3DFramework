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
			: SceneObject(sceneObject), ParentPrefabResourceId(parentPrefabResourceId), PrefabNestingLevel(prefabNestingLevel)
		{}

		HSceneObject SceneObject;
		UUID ParentPrefabResourceId;
		i32 PrefabNestingLevel = 0;
	};

	/**
	 * Deduces original game object ID, prefab object ID and prefab resource ID, as they would be stored in the prefab resource, from an instance of that prefab
	 * and original prefab hierarchy. To be used when updating a prefab from a prefab instance.
	 *
	 * @param gameObject							Game object instance for which to deduce the IDs.
	 * @param associatedSceneObjectInformation		Information about the parent scene object that owns @p gameObject. If @p gameObject is a SceneObject, this should point to the same object.
	 * @param originalPrefabHierarchyIds			Game object ID -> { prefab object ID, prefab resource ID } mapping for all objects in the original hierarchy in the prefab resource.
	 * @param outPrefabGameObjectId					Game object ID to assign to the object in the prefab hierarchy.
	 * @param outNestedPrefabLinkInformation		Prefab object ID and prefab resource ID to assign to the object in the prefab hierarchy.
	 */
	static void DeduceOriginalPrefabIds(const HGameObject& gameObject, const SceneObjectInformation& associatedSceneObjectInformation, const UnorderedMap<UUID, PrefabLinkInformation>& originalPrefabHierarchyIds, UUID& outPrefabGameObjectId, PrefabLinkInformation& outNestedPrefabLinkInformation);

	/**
	 * Updates all objects in @p clonedInstanceHierarchyRoot with IDs so they match previously stored prefab hierarchy. This method should be called after
	 * updating the prefab from an instance.
	 *
	 * Returns a map containing a remapping from instance game object IDs to prefab game object IDs.
	 */
	static UnorderedMap<UUID, UUID> RestoreOriginalPrefabIds(const HSceneObject& originalPrefabHierarchyRoot, const HSceneObject& clonedInstanceHierarchyRoot, const UUID& prefabResourceId);
};

void PrefabIdUtility::DeduceOriginalPrefabIds(const HGameObject& gameObject, const SceneObjectInformation& associatedSceneObjectInformation, const UnorderedMap<UUID, PrefabLinkInformation>& originalPrefabHierarchyIds, UUID& outPrefabGameObjectId, PrefabLinkInformation& outNestedPrefabLinkInformation)
{
	// Try to find or generate a new scene object ID and prefab object ID
	bool generateNewId = false;
	UUID prefabObjectId = UUID::kEmpty;
	{
		const UUID& instancePrefabObjectId = gameObject->GetPrefabObjectId();
		if(gameObject->GetPrefabObjectId().Empty()) // New object if no prefab ID assigned
			generateNewId = true;
		else
		{
			auto found = originalPrefabHierarchyIds.find(instancePrefabObjectId);
			if(found != originalPrefabHierarchyIds.end())
			{
				outPrefabGameObjectId = instancePrefabObjectId;
				prefabObjectId = found->second.PrefabObjectId;
			}
			else // Treat as new object if it cannot be found in the original prefab hierarchy
				generateNewId = true;
		}
	}

	if(generateNewId)
	{
		outPrefabGameObjectId = UUIDGenerator::GenerateRandom();
		prefabObjectId = gameObject->GetPrefabObjectId();
	}

	if(associatedSceneObjectInformation.PrefabNestingLevel == 0) // Root objects don't need prefab object and resource ids
	{
		outNestedPrefabLinkInformation.PrefabObjectId = UUID::kEmpty;
		outNestedPrefabLinkInformation.PrefabResourceId = UUID::kEmpty;
	}
	else if(associatedSceneObjectInformation.PrefabNestingLevel == 1) // First-level nested prefabs need to record a link to their prefab
	{
		outNestedPrefabLinkInformation.PrefabObjectId = prefabObjectId;
		outNestedPrefabLinkInformation.PrefabResourceId = associatedSceneObjectInformation.SceneObject->GetPrefabResourceId();
	}
	else // Any lower level prefab instances are treated as instance-modifications of the top-most nested prefab
	{
		outNestedPrefabLinkInformation.PrefabObjectId = UUID::kEmpty;
		outNestedPrefabLinkInformation.PrefabResourceId = associatedSceneObjectInformation.ParentPrefabResourceId;
	}
}

UnorderedMap<UUID, UUID> PrefabIdUtility::RestoreOriginalPrefabIds(const HSceneObject& originalPrefabHierarchyRoot, const HSceneObject& clonedInstanceHierarchyRoot, const UUID& prefabResourceId)
{
	UnorderedMap<UUID, PrefabLinkInformation> originalPrefabHierarchyIds = PrefabUtility::GetInstanceToPrefabLinkInformationMap(originalPrefabHierarchyRoot, true);
	UnorderedMap<UUID, UUID> remappedGameObjectIDs;

	FrameScope frameScope;
	FrameStack<SceneObjectInformation> todo;
	todo.emplace(clonedInstanceHierarchyRoot, prefabResourceId, 0);

	 while(!todo.empty())
	{
		const SceneObjectInformation currentObjectToProcess = todo.top();
		todo.pop();

		HSceneObject sceneObject = currentObjectToProcess.SceneObject;

		{
			const UUID originalGameObjectId = currentObjectToProcess.SceneObject.GetId();
			UUID prefabGameObjectId = UUID::kEmpty;
			PrefabLinkInformation prefabLinkInformation;

			DeduceOriginalPrefabIds(currentObjectToProcess.SceneObject, currentObjectToProcess, originalPrefabHierarchyIds, prefabGameObjectId, prefabLinkInformation);

			sceneObject->SetId(prefabGameObjectId);
			sceneObject.GetSharedHandleData()->Id = prefabGameObjectId;

			sceneObject->SetPrefabObjectId(prefabLinkInformation.PrefabObjectId);
			sceneObject->SetPrefabResourceId(prefabLinkInformation.PrefabResourceId);

			remappedGameObjectIDs[originalGameObjectId] = prefabGameObjectId;
		}

		for(auto& component : sceneObject->GetComponents())
		{
			const UUID originalGameObjectId = component.GetId();
			UUID prefabGameObjectId = UUID::kEmpty;
			PrefabLinkInformation prefabLinkInformation;

			DeduceOriginalPrefabIds(component, currentObjectToProcess, originalPrefabHierarchyIds, prefabGameObjectId, prefabLinkInformation);

			component->SetId(prefabGameObjectId);
			component.GetSharedHandleData()->Id = prefabGameObjectId;

			component->SetPrefabObjectId(prefabLinkInformation.PrefabObjectId);

			remappedGameObjectIDs[originalGameObjectId] = prefabGameObjectId;
		}

		const u32 childCount = currentObjectToProcess.SceneObject->GetChildCount();
		for(u32 childIndex = 0; childIndex < childCount; childIndex++)
		{
			const HSceneObject childSceneObject = currentObjectToProcess.SceneObject->GetChild(childIndex);

			// If we've reached a new prefab instance, increment nesting level
			u32 currentPrefabNestingLevel = currentObjectToProcess.PrefabNestingLevel;
			{
				const UUID& instancePrefabResourceId = sceneObject->GetPrefabResourceId();
				if(instancePrefabResourceId != currentObjectToProcess.ParentPrefabResourceId)
					currentPrefabNestingLevel++;
			}

			UUID newParentPrefabResourceId;
			if(currentPrefabNestingLevel <= 1)
				newParentPrefabResourceId = currentObjectToProcess.SceneObject->GetPrefabResourceId();
			else
				newParentPrefabResourceId = currentObjectToProcess.ParentPrefabResourceId;

			todo.emplace(childSceneObject, newParentPrefabResourceId, currentPrefabNestingLevel);
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

	PrefabUtility::ClearPrefabIds(sceneObject);
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

	if(!mIsScene)
	{
		// Ensure the prefab hierarchy keeps the original ids
		UnorderedMap<UUID, UUID> remappedGameObjectIDs = PrefabIdUtility::RestoreOriginalPrefabIds(mRoot, newRoot, mUUID);

		// Ensure the instance hierarchy links to this prefab
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
			if(sceneObject->IsPrefabInstanceRoot())
				PrefabUtility::RecordPrefabDelta(sceneObject);

			return true;
		}, nullptr);
	}

	if(mRoot.IsValid())
		mRoot->Destroy(true);

	mRoot = newRoot;
	mGameObjectCollection = newGameObjectCollection;

	// TODO - Write some unit tests to ensure this works correctly
	// - Create a non-nested prefab from a scene object hierarchy, instantiate it, make changes, instantiate again, ensure all links are still valid
	// - Create a nested prefab, make changes to it and the root (add new objects), ensure all links are still valid
	// - But probably just try to get the simple case working correctly

	// TODO - Make sure to add a test with 2 levels of nested prefabs


	// TODO - Don't forget to re-visit UpdateFromPrefab (see comment there)
}

void Prefab::Update(const HSceneObject& sceneObject)
{
	Initialize(sceneObject);

	mHash++;
}

void Prefab::UpdateChildInstancesInternal() const
{
	if(!mRoot.IsValid())
		return;

	mRoot->IterateHierarchy([this](const HSceneObject& sceneObject)
	{
		if(sceneObject->IsPrefabInstanceRoot())
			PrefabUtility::UpdateFromPrefab(sceneObject);

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

	SPtr<SceneInstance> finalSceneInstance = sceneInstance != nullptr ? sceneInstance : GetSceneManager().GetMainScene(); // TODO - Null scene instance should be disallowed
	SPtr<GameObjectCollection> gameObjectCollection = finalSceneInstance->GetGameObjectCollection();

	HSceneObject clone = Clone(gameObjectCollection, preserveIds);
	PrefabUtility::AssignPrefabInstanceIds(clone, mRoot, mUUID);
	clone->InstantiateInternal();

	return clone;
}

HSceneObject Prefab::Clone(const SPtr<GameObjectCollection>& cloneOwnerCollection, bool preserveIds) const
{
	if(mRoot == nullptr)
		return HSceneObject();

	mRoot->mPrefabHash = mHash;
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
