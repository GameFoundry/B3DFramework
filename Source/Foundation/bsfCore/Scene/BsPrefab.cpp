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
			mRoot->destroy(true);
	}

	HPrefab Prefab::create(const HSceneObject& sceneObject, bool isScene)
	{
		SPtr<Prefab> newPrefab = createEmpty();
		newPrefab->mIsScene = isScene;

		PrefabUtility::clearPrefabIds(sceneObject, true, false);
		newPrefab->initialize(sceneObject);

		HPrefab handle = static_resource_cast<Prefab>(gResources()._createResourceHandle(newPrefab));
		newPrefab->mUUID = handle.getUUID();
		sceneObject->mPrefabLinkUUID = newPrefab->mUUID;
		newPrefab->_getRoot()->mPrefabLinkUUID = newPrefab->mUUID;

		return handle;
	}

	SPtr<Prefab> Prefab::createEmpty()
	{
		SPtr<Prefab> newPrefab = bs_core_ptr<Prefab>(new (bs_alloc<Prefab>()) Prefab());
		newPrefab->_setThisPtr(newPrefab);

		return newPrefab;
	}

	void Prefab::initialize(const HSceneObject& sceneObject)
	{
		sceneObject->mPrefabDiff = nullptr;
		PrefabUtility::generatePrefabIds(sceneObject);

		// If there are any child prefab instances, make sure to update their diffs so they are saved with this prefab
		Stack<HSceneObject> todo;
		todo.push(sceneObject);

		while (!todo.empty())
		{
			HSceneObject current = todo.top();
			todo.pop();

			UINT32 childCount = current->getNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = current->getChild(i);

				if (!child->mPrefabLinkUUID.empty())
					PrefabUtility::recordPrefabDiff(child);
				else
					todo.push(child);
			}
		}

		// Clone the hierarchy for internal storage
		if (mRoot != nullptr)
			mRoot->destroy(true);

		mRoot = sceneObject->clone(false, true);
		mRoot->mParent = nullptr;
		mRoot->mLinkId = -1;

		// Remove objects with "dont save" flag
		todo.push(mRoot);

		while (!todo.empty())
		{
			HSceneObject current = todo.top();
			todo.pop();

			if (current->hasFlag(SOF_DontSave))
				current->destroy();
			else
			{
				UINT32 numChildren = current->getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
					todo.push(current->getChild(i));
			}
		}
	}

	void Prefab::update(const HSceneObject& sceneObject)
	{
		initialize(sceneObject);
		sceneObject->mPrefabLinkUUID = mUUID;
		mRoot->mPrefabLinkUUID = mUUID;

		mHash++;
	}

	void Prefab::_updateChildInstances() const
	{
		Stack<HSceneObject> todo;
		todo.push(mRoot);

		while (!todo.empty())
		{
			HSceneObject current = todo.top();
			todo.pop();

			UINT32 childCount = current->getNumChildren();
			for (UINT32 i = 0; i < childCount; i++)
			{
				HSceneObject child = current->getChild(i);

				if (!child->mPrefabLinkUUID.empty())
					PrefabUtility::updateFromPrefab(child);
				else
					todo.push(child);
			}
		}
	}

	HSceneObject Prefab::_instantiate(bool preserveUUIDs) const
	{
		if (mRoot == nullptr)
			return HSceneObject();

#if BS_IS_BANSHEE3D
		if (gCoreApplication().isEditor())
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

		return mRoot->clone(false, preserveUUIDs);
	}

	RTTITypeBase* Prefab::getRTTIStatic()
	{
		return PrefabRTTI::instance();
	}

	RTTITypeBase* Prefab::getRTTI() const
	{
		return Prefab::getRTTIStatic();
	}
}