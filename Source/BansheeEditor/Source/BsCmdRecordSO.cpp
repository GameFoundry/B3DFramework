//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsCmdRecordSO.h"
#include "BsSceneObject.h"
#include "BsComponent.h"
#include "BsMemorySerializer.h"

namespace BansheeEngine
{
	CmdRecordSO::CmdRecordSO(const WString& description, const HSceneObject& sceneObject, bool recordHierarchy)
		: EditorCommand(description), mSceneObject(sceneObject), mRecordHierarchy(recordHierarchy)
		, mSerializedObject(nullptr), mSerializedObjectSize(0)
	{

	}

	CmdRecordSO::~CmdRecordSO()
	{
		mSceneObject = nullptr;
		clear();
	}

	void CmdRecordSO::clear()
	{
		mSerializedObjectSize = 0;

		if (mSerializedObject != nullptr)
		{
			bs_free(mSerializedObject);
			mSerializedObject = nullptr;
		}
	}

	void CmdRecordSO::execute(const HSceneObject& sceneObject, bool recordHierarchy, const WString& description)
	{
		// Register command and commit it
		CmdRecordSO* command = new (bs_alloc<CmdRecordSO>()) CmdRecordSO(description, sceneObject, recordHierarchy);
		UndoRedo::instance().registerCommand(command);
		command->commit();
	}

	void CmdRecordSO::commit()
	{
		clear();

		if (mSceneObject == nullptr || mSceneObject.isDestroyed())
			return;

		recordSO(mSceneObject);
	}

	void CmdRecordSO::revert()
	{
		if (mSceneObject == nullptr || mSceneObject.isDestroyed())
			return;

		HSceneObject parent = mSceneObject->getParent();

		UINT32 numChildren = mSceneObject->getNumChildren();
		HSceneObject* children = nullptr;
		if (!mRecordHierarchy)
		{
			children = bs_stack_new<HSceneObject>(numChildren);
			for (UINT32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = mSceneObject->getChild(i);
				children[i] = child;

				child->setParent(HSceneObject());
			}
		}

		mSceneObject->destroy(true);

		GameObjectManager::instance().setDeserializationMode(GODM_RestoreExternal | GODM_UseNewIds);

		MemorySerializer serializer;
		SPtr<SceneObject> restored = std::static_pointer_cast<SceneObject>(serializer.decode(mSerializedObject, mSerializedObjectSize));

		EditorUtility::restoreIds(restored->getHandle(), mSceneObjectProxy);
		restored->setParent(parent);

		if (!mRecordHierarchy)
		{
			for (UINT32 i = 0; i < numChildren; i++)
				children[i]->setParent(restored->getHandle());

			bs_stack_delete(children, numChildren);
		}

		restored->_instantiate();
	}

	void CmdRecordSO::recordSO(const HSceneObject& sceneObject)
	{
		UINT32 numChildren = mSceneObject->getNumChildren();
		HSceneObject* children = nullptr;

		if (!mRecordHierarchy)
		{
			children = bs_stack_new<HSceneObject>(numChildren);
			for (UINT32 i = 0; i < numChildren; i++)
			{
				HSceneObject child = mSceneObject->getChild(i);
				children[i] = child;

				child->setParent(HSceneObject());
			}
		}

		bool isInstantiated = !mSceneObject->hasFlag(SOF_DontInstantiate);
		mSceneObject->_setFlags(SOF_DontInstantiate);

		MemorySerializer serializer;
		mSerializedObject = serializer.encode(mSceneObject.get(), mSerializedObjectSize);

		if (isInstantiated)
			mSceneObject->_unsetFlags(SOF_DontInstantiate);

		mSceneObjectProxy = EditorUtility::createProxy(mSceneObject);

		if (!mRecordHierarchy)
		{
			for (UINT32 i = 0; i < numChildren; i++)
				children[i]->setParent(sceneObject->getHandle());

			bs_stack_delete(children, numChildren);
		}
	}
}