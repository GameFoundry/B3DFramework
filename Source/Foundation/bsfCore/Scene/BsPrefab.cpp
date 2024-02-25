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

	// Record original prefab object ids so we know to restore them, then destroy the internal hierarchy
	if(mRoot.IsValid())
		mRoot->Destroy(true);

	const SPtr<GameObjectCollection> newGameObjectCollection = GameObjectCollection::Create();
	mRoot = sceneObject->Clone(newGameObjectCollection, false, true);
	mRoot->mParent = nullptr;

	// Remove objects with "dont save" flag
	FrameScope frameScope;
	FrameVector<HSceneObject> sceneObjectsToDestroy;
	mRoot->IterateHierarchy([&sceneObjectsToDestroy](const HSceneObject& sceneObject) {
		if(sceneObject->HasFlag(SOF_DontSave))
		{
			sceneObjectsToDestroy.push_back(sceneObject);
			return false;
		}

		return true;
	}, nullptr);

	for(const auto& entry : sceneObjectsToDestroy)
		entry->Destroy();

	mGameObjectCollection = newGameObjectCollection;

	if(!mIsScene)
	{
		// Ensure the prefab hierarchy keeps the original ids
		UnorderedMap<UUID, PrefabLinkInformation> instanceHierarchyIds = PrefabUtility::GetInstanceToPrefabLinkInformationMap(sceneObject, true);

		// TODO - What happens with objects with no prefab link, yet part of nested prefabs? Keep them with empty prefab object/resource id?
		// TODO - What happens when creating a template from another templates instance? Just clear the prefab object ids until the first nested prefab instance?
		mRoot->IterateHierarchy([this, &instanceHierarchyIds](const HSceneObject& sceneObject) {
			UUID idInPrefab;
			if(auto found = instanceHierarchyIds.find(sceneObject.GetId()); found != instanceHierarchyIds.end())
				idInPrefab = found->second.PrefabObjectId;
			else
			{
				idInPrefab = UUIDGenerator::GenerateRandom();

				// Record this here, as we'll re-use this map when assigning prefab object IDs to the instance hierarchy
				instanceHierarchyIds.insert(std::make_pair(sceneObject.GetId(), idInPrefab));
			}

			sceneObject->SetId(idInPrefab);
			sceneObject.GetSharedHandleData()->Id = idInPrefab;

			const bool isPartOfRootPrefab = sceneObject->GetPrefabResourceId() == mUUID;
			if(isPartOfRootPrefab)
			{
				// Only nested prefabs should have prefab links within the prefab
				sceneObject->SetPrefabObjectId(UUID::kEmpty);
				sceneObject->SetPrefabResourceId(UUID::kEmpty);
			}
			else
			{
				// Objects not part of the root prefab keep their prefab object and resource ids, as those will point to nested prefabs
			}

			return true;
		},
		[this, &instanceHierarchyIds](const HComponent& component) {
			UUID idInPrefab;
			if(auto found = instanceHierarchyIds.find(component.GetId()); found != instanceHierarchyIds.end())
				idInPrefab = found->second.PrefabObjectId;
			else
			{
				idInPrefab = UUIDGenerator::GenerateRandom();

				// Record this here, as we'll re-use this map when assigning prefab object IDs to the instance hierarchy
				instanceHierarchyIds.insert(std::make_pair(component.GetId(), idInPrefab));
			}

			component->SetId(idInPrefab);
			component.GetSharedHandleData()->Id = idInPrefab;

			const bool isPartOfRootPrefab = component->SceneObject()->GetPrefabResourceId() == mUUID;
			if(isPartOfRootPrefab)
			{
				// Only nested prefabs should have prefab links within the prefab
				component->SetPrefabObjectId(UUID::kEmpty);
			}
			else
			{
				// Objects not part of the original prefab keep their prefab object id, as this will point to nested prefabs
			}
		});

		// Ensure the instance hierarchy links to this prefab
		B3D_ASSERT(mUUID != UUID::kEmpty);
		sceneObject->IterateHierarchy([this, &instanceHierarchyIds](const HSceneObject& sceneObject)
		{
			if(sceneObject->HasFlag(SOF_DontSave))
				return false;

			if(auto found = instanceHierarchyIds.find(sceneObject.GetId()); B3D_ENSURE(found != instanceHierarchyIds.end()))
			{
				sceneObject->SetPrefabObjectId(found->second.PrefabObjectId);
				sceneObject->SetPrefabResourceId(mUUID);
			}

			return true;
		},
		[this, &instanceHierarchyIds](const HComponent& component)
		{
			if(auto found = instanceHierarchyIds.find(component.GetId()); B3D_ENSURE(found != instanceHierarchyIds.end()))
			{
				component->SetPrefabObjectId(found->second.PrefabObjectId);
			}
		});

		// Generate deltas for nested prefabs
		mRoot->IterateHierarchy([this](const HSceneObject& sceneObject) {
			if(sceneObject->IsPrefabInstanceRoot())
				PrefabUtility::RecordPrefabDelta(sceneObject);

			return true;
		}, nullptr);
	}

	// TODO - Write some unit tests to ensure this works correctly
	// - Create a non-nested prefab from a scene object hierarchy, instantiate it, make changes, instantiate again, ensure all links are still valid
	// - Create a nested prefab, make changes to it and the root (add new objects), ensure all links are still valid
	// - But probably just try to get the simple case working correctly


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

HSceneObject Prefab::InstantiateInternal(bool preserveUUIDs) const
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

	SPtr<SceneInstance> scene = GetSceneManager().GetMainScene(); // TODO - Scene should be a a parameter
	SPtr<GameObjectCollection> gameObjectCollection = scene->GetGameObjectCollection();

	HSceneObject clone = CloneInternal(gameObjectCollection, preserveUUIDs);
	clone->InstantiateInternal();

	return clone;
}

HSceneObject Prefab::CloneInternal(const SPtr<GameObjectCollection>& cloneOwnerCollection, bool preserveUUIDs) const
{
	if(mRoot == nullptr)
		return HSceneObject();

	mRoot->mPrefabHash = mHash;
	return mRoot->Clone(cloneOwnerCollection, false, preserveUUIDs);
}

RTTITypeBase* Prefab::GetRttiStatic()
{
	return PrefabRTTI::Instance();
}

RTTITypeBase* Prefab::GetRtti() const
{
	return Prefab::GetRttiStatic();
}
