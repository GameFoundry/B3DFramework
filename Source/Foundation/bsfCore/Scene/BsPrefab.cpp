//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Scene/BsPrefab.h"
#include "Private/RTTI/BsPrefabRTTI.h"
#include "Resources/BsResources.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsPrefabUtility.h"
#include "BsCoreApplication.h"

namespace bs
{
	Prefab::Prefab()
		:Resource(false)
	{
		
	}

	Prefab::~Prefab()
	{
		if (mRoot != nullptr)
			mRoot->Destroy(true);
	}

	HPrefab Prefab::Create(const HSceneObject& sceneObject, bool isScene)
	{
		SPtr<Prefab> newPrefab = createEmpty();
		newPrefab->mIsScene = isScene;

		PrefabUtility::clearPrefabIds(sceneObject, true, false);
		newPrefab->Initialize(sceneObject);

		HPrefab handle = static_resource_cast<Prefab>(gResources()._createResourceHandle(newPrefab));
		newPrefab->mUUID = handle.GetUUID();
		sceneObject->mPrefabLinkUUID = newPrefab->mUUID;
		newPrefab->_getRoot()->mPrefabLinkUUID = newPrefab->mUUID;

		return handle;
	}

	SPtr<Prefab> Prefab::CreateEmpty()
	{
		SPtr<Prefab> newPrefab = bs_core_ptr<Prefab>(new (bs_alloc<Prefab>()) Prefab());
		newPrefab->_setThisPtr(newPrefab);

		return newPrefab;
	}

	void Prefab::Initialize(const HSceneObject& sceneObject)
	{
		sceneObject->mPrefabDiff = nullptr;
		PrefabUtility::generatePrefabIds(sceneObject);

		// If there are any child prefab instances, make sure to update their diffs so they are saved with this prefab
		Stack<HSceneObject> todo;
		todo.Push(sceneObject);

		while (!todo.Empty())
		{
			HSceneObject current = todo.Top();
			todo.Pop();

			UINT32 childCount = current->GetNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = current->GetChild(i);

				if (!child->mPrefabLinkUUID.Empty())
					PrefabUtility::recordPrefabDiff(child);
				else
					todo.Push(child);
			}
		}

		// Clone the hierarchy for internal storage
		if (mRoot != nullptr)
			mRoot->Destroy(true);

		mRoot = sceneObject->Clone(false, true);
		mRoot->mParent = nullptr;
		mRoot->mLinkId = -1;

		// Remove objects with "dont save" flag
		todo.Push(mRoot);

		while (!todo.Empty())
		{
			HSceneObject current = todo.Top();
			todo.Pop();

			if (current->HasFlag(SOF_DontSave))
				current->Destroy();
			else
			{
				UINT32 numChildren = current->GetNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.Push(current->GetChild(i));
			}
		}
	}

	void Prefab::Update(const HSceneObject& sceneObject)
	{
		initialize(sceneObject);
		sceneObject->mPrefabLinkUUID = mUUID;
		mRoot->mPrefabLinkUUID = mUUID;

		mHash++;
	}

	void Prefab::_updateChildInstances() const
	{
		Stack<HSceneObject> todo;
		todo.Push(mRoot);

		while (!todo.Empty())
		{
			HSceneObject current = todo.Top();
			todo.Pop();

			UINT32 childCount = current->GetNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = current->GetChild(i);

				if (!child->mPrefabLinkUUID.Empty())
					PrefabUtility::updateFromPrefab(child);
				else
					todo.Push(child);
			}
		}
	}

	HSceneObject Prefab::_instantiate(bool preserveUUIDs) const
	{
		if (mRoot == nullptr)
			return HSceneObject();

#if BS_IS_BANSHEE3D
		if (gCoreApplication().IsEditor())
		{
			// Update any child prefab instances in case their prefabs changed
			_updateChildInstances();
		}
#endif

		HSceneObject clone = _clone(preserveUUIDs);
		clone->_instantiate();
		
		return clone;
	}

	HSceneObject Prefab::_clone(bool preserveUUIDs) const
	{
		if (mRoot == nullptr)
			return HSceneObject();

		mRoot->mPrefabHash = mHash;
		mRoot->mLinkId = -1;

		return mRoot->Clone(false, preserveUUIDs);
	}

	RTTITypeBase* Prefab::getRTTIStatic()
	{
		return PrefabRTTI::Instance();
	}

	RTTITypeBase* Prefab::getRTTI() const
	{
		return Prefab::GetRTTIStatic();
	}
}
