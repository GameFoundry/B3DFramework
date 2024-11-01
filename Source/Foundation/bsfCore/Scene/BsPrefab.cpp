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
#include "BsSceneObjectHierarchyDelta.h"
#include "Scene/BsPrefabIdRemapper.h"

using namespace bs;

namespace bs
{
	B3D_LOG_CATEGORY(Prefab)
}

void PrefabManager::RegisterPrefab(Prefab& prefab)
{
	if(prefab.IsScene()) // Not keeping track of scenes
		return;

	B3D_ENSURE(mLivePrefabs.insert(&prefab).second);
}

void PrefabManager::UnregisterPrefab(Prefab* prefab)
{
	if(prefab->IsScene()) // Not keeping track of scenes
		return;

	auto found = mLivePrefabs.find(prefab);
	if(B3D_ENSURE(found != mLivePrefabs.end()))
		mLivePrefabs.erase(found);
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

	const UnorderedMap<UUID, UUID>& remappingTable = newPrefab->ReplaceInternalHierarchy(sceneObject);
	PrefabUtility::RemapPrefabInstanceIds(sceneObject, remappingTable, newPrefab->mUUID);
	newPrefab->Initialize();

	return B3DStaticResourceCast<Prefab>(GetResources().CreateResourceHandle(newPrefab, newPrefab->mUUID));
}

SPtr<Prefab> Prefab::CreateEmpty()
{
	SPtr<Prefab> newPrefab = B3DMakeSharedFromExisting<Prefab>(new(B3DAllocate<Prefab>()) Prefab());
	newPrefab->SetShared(newPrefab);

	return newPrefab;
}

void Prefab::Initialize()
{
	Resource::Initialize();

	EnsureMainThread();
	PrefabManager::Instance().RegisterPrefab(*this);

	// Ensure we have all the latest version of child prefabs
	const bool isPrefabUpdated = PrefabUtility::UpdateNestedPrefabInstances(mRoot);
	if(isPrefabUpdated)
	{
		TickPrefabVersion();
		RecordNestedPrefabInstanceDeltas();
	}
}

void Prefab::Destroy()
{
	PrefabManager::Instance().UnregisterPrefab(this);

	Resource::Destroy();
}

UnorderedMap<UUID, UUID> Prefab::ReplaceInternalHierarchy(const HSceneObject& sceneObject)
{
	const SPtr<GameObjectCollection> newGameObjectCollection = GameObjectCollection::Create();
	HSceneObject newRoot = sceneObject->Clone(newGameObjectCollection, true);
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
	PrefabIdRemapper idRemapper(mRoot, mUUID, newGameObjectCollection);
	UnorderedMap<UUID, UUID> remappedGameObjectIDs = idRemapper.RestoreOriginalPrefabIds(newRoot);

	if(mRoot.IsValid())
		mRoot->Destroy(true);

	mRoot = newRoot; // Note: PrefabIdUtility::RestoreOriginalPrefabIds() depends on this being changed after it has been called, as it may try to access the original prefab root
	mGameObjectCollection = newGameObjectCollection;

	TickPrefabVersion();
	RecordNestedPrefabInstanceDeltas();

	return remappedGameObjectIDs;
}

HSceneObject Prefab::Instantiate(const SPtr<SceneInstance>& sceneInstance) const
{
	SPtr<SceneInstance> sceneInstanceMutableShared = sceneInstance;
	return Instantiate(sceneInstanceMutableShared, false);
}

SPtr<SceneInstance> Prefab::Instantiate() const
{
	SPtr<SceneInstance> sceneInstance;
	Instantiate(sceneInstance, false);

	return sceneInstance;
}

HSceneObject Prefab::Instantiate(SPtr<SceneInstance>& inOutSceneInstance, bool preserveIds) const
{
	if(mRoot == nullptr)
		return HSceneObject();


	SPtr<GameObjectCollection> gameObjectCollection;
	if(inOutSceneInstance != nullptr)
		gameObjectCollection = inOutSceneInstance->GetGameObjectCollection();
	else
		gameObjectCollection = GameObjectCollection::Create();

	HSceneObject clone = Clone(gameObjectCollection, preserveIds);
	PrefabUtility::AssignPrefabInstanceIds(clone, mRoot, mUUID);

	if(inOutSceneInstance != nullptr)
		clone->SetParent(inOutSceneInstance->GetRoot());
	else
		inOutSceneInstance = SceneInstance::Create("PrefabInstance", clone, GetId());

	clone->Initialize();
	return clone;
}

HSceneObject Prefab::Clone(const SPtr<GameObjectCollection>& cloneOwnerCollection, bool preserveIds) const
{
	if(mRoot == nullptr)
		return HSceneObject();

	mRoot->SetPrefabVersion(mPrefabVersion); // TODO - Might make sense to assign this to the entire hierarchy. Also for internal hierarchy, it should be set when internal hierarchy is updated.
	return mRoot->Clone(cloneOwnerCollection, preserveIds);
}

void Prefab::TickPrefabVersion()
{
	mPrefabVersion = UUIDGenerator::GenerateRandom();
}

void Prefab::RecordNestedPrefabInstanceDeltas()
{
	//if(!mRoot.IsValid())
	//	return;

	//// Generate deltas for first level of nested prefabs (Any deeper levels will be either part of the
	//// nested prefab themselves, or instance modifications of those nested prefabs)
	//mRoot->IterateHierarchy([this](const HSceneObject& sceneObject) {
	//	if(sceneObject->IsPrefabInstanceRoot())
	//	{
	//		sceneObject->SetPrefabDelta(nullptr);

	//		const UUID& prefabResourceId = sceneObject->GetPrefabResourceId();
	//		if(!prefabResourceId.Empty())
	//		{
	//			HPrefab linkedPrefab = B3DStaticResourceCast<Prefab>(GetResources().LoadFromUuid(prefabResourceId, false, ResourceLoadFlag::None));
	//			if(linkedPrefab.IsLoaded(false))
	//				sceneObject->SetPrefabDelta(SceneObjectHierarchyDelta::Create(linkedPrefab->GetRoot(), sceneObject, SceneObjectHierarchyDeltaFlag::PrefabDelta));
	//			else
	//			{
	//				B3D_LOG(Warning, Prefab, "Cannot record prefab delta for scene object '{0}'. Failed to load prefab with ID: '{1}'.", sceneObject.GetId(), prefabResourceId);
	//			}
	//		}

	//		return false;
	//	}

	//	return true;
	//}, nullptr, false);
}

RTTIType* Prefab::GetRttiStatic()
{
	return PrefabRTTI::Instance();
}

RTTIType* Prefab::GetRtti() const
{
	return Prefab::GetRttiStatic();
}
