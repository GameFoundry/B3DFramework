//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCmdBreakPrefab.h"
#include "BsSceneObject.h"

namespace BansheeEngine
{
	CmdBreakPrefab::CmdBreakPrefab(const WString& description, const HSceneObject& sceneObject)
		:EditorCommand(description), mSceneObject(sceneObject)
	{

	}

	CmdBreakPrefab::~CmdBreakPrefab()
	{
		mSceneObject = nullptr;

	}
	
	void CmdBreakPrefab::execute(const HSceneObject& sceneObject, const WString& description)
	{
		// Register command and commit it
		CmdBreakPrefab* command = new (bs_alloc<CmdBreakPrefab>()) CmdBreakPrefab(description, sceneObject);
		SPtr<CmdBreakPrefab> commandPtr = bs_shared_ptr(command);

		UndoRedo::instance().registerCommand(commandPtr);
		commandPtr->commit();
	}

	void CmdBreakPrefab::commit()
	{
		clear();

		if (mSceneObject == nullptr || mSceneObject.isDestroyed())
			return;

		HSceneObject rootObj = mSceneObject;

		while (rootObj != nullptr)
		{
			if (!rootObj->_getPrefabLinkUUID().empty())
				break;

			if (rootObj->getParent() != nullptr)
				rootObj = rootObj->getParent();
			else
				rootObj = nullptr;
		}

		if (rootObj != nullptr)
		{
			mPrefabRoot = rootObj;
			mPrefabLinkUUID = rootObj->_getPrefabLinkUUID();
			mPrefabDiff = rootObj->_getPrefabDiff();

			Stack<HSceneObject> todo;
			todo.push(mPrefabRoot);

			while (!todo.empty())
			{
				HSceneObject currentSO = todo.top();
				todo.pop();

				const Vector<HComponent>& components = currentSO->getComponents();
				for (auto& component : components)
				{
					UINT32 linkId = component->getLinkId();

					if (linkId != (UINT32)-1)
						mLinkIds[component->getInstanceId()] = linkId;

					mLinkIds[component->getInstanceId()] = component->getLinkId();
				}

				UINT32 numChildren = (UINT32)currentSO->getNumChildren();
				for (UINT32 i = 0; i < numChildren; i++)
				{
					HSceneObject child = currentSO->getChild(i);
					UINT32 linkId = child->getLinkId();

					if (linkId != (UINT32)-1)
						mLinkIds[child->getInstanceId()] = linkId;

					if (child->_getPrefabLinkUUID().empty())
						todo.push(child);
				}
			}
		}

		mSceneObject->breakPrefabLink();
	}

	void CmdBreakPrefab::revert()
	{
		if (mPrefabRoot == nullptr || mPrefabRoot.isDestroyed())
			return;

		mPrefabRoot->_setPrefabLinkUUID(mPrefabLinkUUID);
		mPrefabRoot->_setPrefabDiff(mPrefabDiff);

		Stack<HSceneObject> todo;
		todo.push(mPrefabRoot);

		while (!todo.empty())
		{
			HSceneObject currentSO = todo.top();
			todo.pop();

			const Vector<HComponent>& components = currentSO->getComponents();
			for (auto& component : components)
			{
				auto iterFind = mLinkIds.find(component->getInstanceId());
				if (iterFind != mLinkIds.end())
					component->_setLinkId(iterFind->second);
			}

			UINT32 numChildren = (UINT32)currentSO->getNumChildren();
			for (UINT32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = currentSO->getChild(i);

				auto iterFind = mLinkIds.find(child->getInstanceId());
				if (iterFind != mLinkIds.end())
					child->_setLinkId(iterFind->second);

				if (child->_getPrefabLinkUUID().empty())
					todo.push(child);
			}
		}
	}

	void CmdBreakPrefab::clear()
	{
		mPrefabRoot = nullptr;
		mPrefabLinkUUID = "";
		mPrefabDiff = nullptr;
		mLinkIds.clear();
	}
}