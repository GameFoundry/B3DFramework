//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefab.h"
#include "Private/RTTI/BsPrefabRTTI.h"
#include "Resources/BsResources.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsPrefabUtility.h"
#include "BsCoreApplication.h"

using namespace bs;

Prefab::Prefab()
	: Resource(false)
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

	PrefabUtility::ClearPrefabIds(sceneObject, true, false);
	newPrefab->Initialize(sceneObject);

	HPrefab handle = B3DStaticResourceCast<Prefab>(GetResources().CreateResourceHandle(newPrefab));
	newPrefab->mUUID = handle.GetId();
	sceneObject->mPrefabLinkUUID = newPrefab->mUUID;
	newPrefab->GetRootInternal()->mPrefabLinkUUID = newPrefab->mUUID;

	return handle;
}

SPtr<Prefab> Prefab::CreateEmpty()
{
	SPtr<Prefab> newPrefab = B3DMakeSharedFromExisting<Prefab>(new(B3DAllocate<Prefab>()) Prefab());
	newPrefab->SetShared(newPrefab);

	return newPrefab;
}

void Prefab::Initialize(const HSceneObject& sceneObject)
{
	sceneObject->mPrefabDiff = nullptr;
	PrefabUtility::GeneratePrefabIds(sceneObject);

	// If there are any child prefab instances, make sure to update their diffs so they are saved with this prefab
	Stack<HSceneObject> todo;
	todo.push(sceneObject);

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		u32 childCount = current->GetNumChildren();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = current->GetChild(i);

			if(!child->mPrefabLinkUUID.Empty())
				PrefabUtility::RecordPrefabDiff(child);
			else
				todo.push(child);
		}
	}

	// Clone the hierarchy for internal storage
	if(mRoot != nullptr)
		mRoot->Destroy(true);

	mRoot = sceneObject->Clone(false, true);
	mRoot->mParent = nullptr;
	mRoot->mLinkId = -1;

	// Remove objects with "dont save" flag
	todo.push(mRoot);

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		if(current->HasFlag(SOF_DontSave))
			current->Destroy();
		else
		{
			u32 numChildren = current->GetNumChildren();
			for(u32 i = 0; i < numChildren; i++)
				todo.push(current->GetChild(i));
		}
	}
}

void Prefab::Update(const HSceneObject& sceneObject)
{
	Initialize(sceneObject);
	sceneObject->mPrefabLinkUUID = mUUID;
	mRoot->mPrefabLinkUUID = mUUID;

	mHash++;
}

void Prefab::UpdateChildInstancesInternal() const
{
	Stack<HSceneObject> todo;
	todo.push(mRoot);

	while(!todo.empty())
	{
		HSceneObject current = todo.top();
		todo.pop();

		u32 childCount = current->GetNumChildren();
		for(u32 i = 0; i < childCount; i++)
		{
			HSceneObject child = current->GetChild(i);

			if(!child->mPrefabLinkUUID.Empty())
				PrefabUtility::UpdateFromPrefab(child);
			else
				todo.push(child);
		}
	}
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

	HSceneObject clone = CloneInternal(preserveUUIDs);
	clone->InstantiateInternal();

	return clone;
}

HSceneObject Prefab::CloneInternal(bool preserveUUIDs) const
{
	if(mRoot == nullptr)
		return HSceneObject();

	mRoot->mPrefabHash = mHash;
	mRoot->mLinkId = -1;

	return mRoot->Clone(false, preserveUUIDs);
}

RTTITypeBase* Prefab::GetRttiStatic()
{
	return PrefabRTTI::Instance();
}

RTTITypeBase* Prefab::GetRtti() const
{
	return Prefab::GetRttiStatic();
}
