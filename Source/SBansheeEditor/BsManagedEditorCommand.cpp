//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedEditorCommand.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsMemorySerializer.h"
#include "Serialization/BsManagedSerializableObject.h"

namespace bs
{
	MonoMethod* ScriptCmdManaged::sCommitMethod = nullptr;
	MonoMethod* ScriptCmdManaged::sRevertMethod = nullptr;

	ScriptCmdManaged::ScriptCmdManaged(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
		MonoUtil::getClassName(managedInstance, mNamespace, mType);

		if(!ScriptAssemblyManager::instance().hasSerializableObjectInfo(mNamespace, mType))
		{
			LOGWRN("UndoableCommand created without [SerializeObject] attribute. The command will not be able to \
				persist assembly refresh.");
		}

		mManagedCommand = bs_shared_ptr(new (bs_alloc<CmdManaged>()) CmdManaged(this));

		mGCHandle = MonoUtil::newWeakGCHandle(managedInstance);
		mInUndoRedoStack = false;
	}

	ScriptCmdManaged::~ScriptCmdManaged()
	{
		if(mManagedCommand != nullptr)
		{
			mManagedCommand->notifyScriptInstanceDestroyed();
			mManagedCommand = nullptr;
		}
	}

	void ScriptCmdManaged::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptCmdManaged::internal_CreateInstance);

		sCommitMethod = metaData.scriptClass->getMethod("Commit");
		sRevertMethod = metaData.scriptClass->getMethod("Revert");
	}

	void ScriptCmdManaged::internal_CreateInstance(MonoObject* managedInstance)
	{
		new (bs_alloc<ScriptCmdManaged>()) ScriptCmdManaged(managedInstance);
	}

	void ScriptCmdManaged::triggerCommit()
	{
		if (sCommitMethod == nullptr)
			return;

		MonoObject* obj = MonoUtil::getObjectFromGCHandle(mGCHandle);
		sCommitMethod->invokeVirtual(obj, nullptr);
	}

	void ScriptCmdManaged::triggerRevert()
	{
		if (sRevertMethod == nullptr)
			return;

		MonoObject* obj = MonoUtil::getObjectFromGCHandle(mGCHandle);
		sRevertMethod->invokeVirtual(obj, nullptr);
	}

	void ScriptCmdManaged::notifyCommandDestroyed()
	{
		mManagedCommand = nullptr;
	}

	void ScriptCmdManaged::notifyStackAdded()
	{
		if (!mInUndoRedoStack)
		{
			MonoObject* obj = MonoUtil::getObjectFromGCHandle(mGCHandle);
			mGCHandle = MonoUtil::newGCHandle(obj, false);
			mInUndoRedoStack = true;
		}
	}

	void ScriptCmdManaged::notifyStackRemoved()
	{
		MonoObject* obj = nullptr;
		if (mGCHandle)
			obj = MonoUtil::getObjectFromGCHandle(mGCHandle);

		if (mInUndoRedoStack && mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}

		// Note: Re-creating the weak handle might not be necessary as the command shouldn't be allowed to be re-added
		// after it has been removed from the undo-redo stack (which should be the only place this method is called from).
		if(obj)
			mGCHandle = MonoUtil::newWeakGCHandle(obj);

		mInUndoRedoStack = false;
	}

	MonoObject* ScriptCmdManaged::_createManagedInstance(bool construct)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// If not in undo-redo stack then this object should have been deleted
		assert(mInUndoRedoStack);

		// See if this type even still exists
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mType, currentObjInfo))
		{
			mTypeMissing = true;
			return nullptr;
		}

		MonoObject* instance = currentObjInfo->mMonoClass->createInstance(construct);
		mGCHandle = MonoUtil::newGCHandle(instance, false);

		mTypeMissing = false;
		return instance;
	}

	void ScriptCmdManaged::_clearManagedInstance()
	{
		if(mInUndoRedoStack)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	ScriptObjectBackup ScriptCmdManaged::beginRefresh()
	{
		RawBackupData backupData;

		if(mInUndoRedoStack)
		{
			// If type is not missing, restore saved data, otherwise keep the data for later
			SPtr<ManagedSerializableObject> serializableObject;
			if(!mTypeMissing)
			{
				MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
				serializableObject = ManagedSerializableObject::createFromExisting(instance);
			}
			else
				serializableObject = mSerializedObjectData;

			if (serializableObject != nullptr)
			{
				MemorySerializer ms;
				backupData.data = ms.encode(serializableObject.get(), backupData.size);
			}
		}

		return ScriptObjectBackup(backupData);
	}

	void ScriptCmdManaged::endRefresh(const ScriptObjectBackup& backupData)
	{
		const RawBackupData& data = any_cast_ref<RawBackupData>(backupData.data);

		MemorySerializer ms;
		SPtr<ManagedSerializableObject> serializableObject = std::static_pointer_cast<ManagedSerializableObject>(
			ms.decode(data.data, data.size));

		if(!mTypeMissing)
		{
			SPtr<ManagedSerializableObjectInfo> objInfo;
			ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mType, objInfo);

			MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
			serializableObject->deserialize(instance, objInfo);

			mSerializedObjectData = nullptr;
		}
		else
			mSerializedObjectData = serializableObject;
	}

	void ScriptCmdManaged::_onManagedInstanceDeleted(bool assemblyRefresh)
	{
		if(!mInUndoRedoStack || !assemblyRefresh)
			ScriptObjectBase::_onManagedInstanceDeleted(assemblyRefresh);
	}

	CmdManaged::CmdManaged(ScriptCmdManaged* scriptObj)
		: EditorCommand(L""), mScriptObj(scriptObj), mRefCount(0)
	{

	}

	CmdManaged::~CmdManaged()
	{
		if (mScriptObj != nullptr)
			mScriptObj->notifyCommandDestroyed();
	}

	void CmdManaged::commit()
	{
		if(mScriptObj == nullptr)
		{
			LOGWRN("Trying to execute a managed undo/redo command whose managed object has been destroyed, ignoring.");
			// Note: This can most likely happen if managed undo commands are queued on a global undo/redo stack. When
			// assembly refresh happens those commands will be rebuilt, but this can fail if the user removed the command
			// type from the assembly, or the command isn't serializable.

			return;
		}

		mScriptObj->triggerCommit();
	}

	void CmdManaged::revert()
	{
		if (mScriptObj == nullptr)
		{
			LOGWRN("Trying to execute a managed undo/redo command whose managed object has been destroyed, ignoring.");
			// Note: This can most likely happen if managed undo commands are queued on a global undo/redo stack. When
			// assembly refresh happens those commands will be rebuilt, but this can fail if the user removed the command
			// type from the assembly, or the command isn't serializable.

			return;
		}

		mScriptObj->triggerRevert();
	}

	void CmdManaged::onCommandAdded()
	{
		if(mScriptObj)
			mScriptObj->notifyStackAdded();

		mRefCount++;
	}

	void CmdManaged::onCommandRemoved()
	{
		assert(mRefCount > 0);

		mRefCount--;

		if (mRefCount == 0)
			mScriptObj->notifyStackRemoved();
	}

	void CmdManaged::notifyScriptInstanceDestroyed()
	{
		mScriptObj = nullptr;
	}
}
