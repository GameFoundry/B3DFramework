//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedEditorCommand.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"

namespace bs
{
	MonoMethod* ScriptCmdManaged::sCommitMethod = nullptr;
	MonoMethod* ScriptCmdManaged::sRevertMethod = nullptr;

	ScriptCmdManaged::ScriptCmdManaged(MonoObject* managedInstance)
		:ScriptObject(managedInstance), mManagedCommand(nullptr)
	{
		mManagedCommand = bs_shared_ptr(new (bs_alloc<CmdManaged>()) CmdManaged(this));

		mGCHandle = MonoUtil::newWeakGCHandle(managedInstance);
		mWeakHandle = true;
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

	void ScriptCmdManaged::allocGCHandle()
	{
		if (mWeakHandle)
		{
			MonoObject* obj = MonoUtil::getObjectFromGCHandle(mGCHandle);
			mGCHandle = MonoUtil::newGCHandle(obj);
			mWeakHandle = false;
		}
	}

	void ScriptCmdManaged::freeGCHandle()
	{
		MonoObject* obj = nullptr;
		if (mGCHandle)
			obj = MonoUtil::getObjectFromGCHandle(mGCHandle);

		if (!mWeakHandle && mGCHandle != 0)
			MonoUtil::freeGCHandle(mGCHandle);

		// Note: Re-creating the weak handle might not be necessary as the command shouldn't be allowed to be re-added
		// after it has been removed from the undo-redo stack (which should be the only place this method is called from).
		if(obj)
			mGCHandle = MonoUtil::newWeakGCHandle(obj);

		mWeakHandle = true;
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
			// assembly refresh happens those commands will be destroyed but not removed from the stack. To fix the issue
			// implement assembly refresh handling for such commands.

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
			// assembly refresh happens those commands will be destroyed but not removed from the stack. To fix the issue
			// implement assembly refresh handling for such commands.

			return;
		}

		mScriptObj->triggerRevert();
	}

	void CmdManaged::onCommandAdded()
	{
		if(mScriptObj)
			mScriptObj->allocGCHandle();

		mRefCount++;
	}

	void CmdManaged::onCommandRemoved()
	{
		assert(mRefCount > 0);

		mRefCount--;

		if (mRefCount == 0)
			mScriptObj->freeGCHandle();
	}

	void CmdManaged::notifyScriptInstanceDestroyed()
	{
		mScriptObj = nullptr;
	}
}
