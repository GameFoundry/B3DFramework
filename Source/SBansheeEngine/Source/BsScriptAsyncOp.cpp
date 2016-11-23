//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptAsyncOp.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsCoreThread.h"

namespace bs
{
	ScriptAsyncOp::ScriptAsyncOp(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptAsyncOp::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptAsyncOp::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_IsComplete", &ScriptAsyncOp::internal_isComplete);
		metaData.scriptClass->addInternalCall("Internal_GetReturnValue", &ScriptAsyncOp::internal_getReturnValue);
		metaData.scriptClass->addInternalCall("Internal_BlockUntilComplete", &ScriptAsyncOp::internal_blockUntilComplete);
	}

	MonoObject* ScriptAsyncOp::create(const AsyncOp& op, 
		std::function<MonoObject*(const AsyncOp&)> asyncOpToReturnValue)
	{
		MonoObject* managedInstance = metaData.scriptClass->createInstance();

		ScriptAsyncOp* scriptAsyncOp = toNative(managedInstance);
		scriptAsyncOp->initialize(op, asyncOpToReturnValue);

		return managedInstance;
	}

	void ScriptAsyncOp::initialize(const AsyncOp& op, std::function<MonoObject*(const AsyncOp&)> asyncOpToReturnValue)
	{
		mAsyncOp = op;
		mConvertCallback = asyncOpToReturnValue;
	}

	void ScriptAsyncOp::internal_createInstance(MonoObject* managedInstance)
	{
		new (bs_alloc<ScriptAsyncOp>()) ScriptAsyncOp(managedInstance);
	}

	void ScriptAsyncOp::internal_isComplete(ScriptAsyncOp* thisPtr, bool* value)
	{
		*value = thisPtr->mAsyncOp.hasCompleted();
	}

	MonoObject* ScriptAsyncOp::internal_getReturnValue(ScriptAsyncOp* thisPtr)
	{
		if (!thisPtr->mAsyncOp.hasCompleted())
			return nullptr;

		if (thisPtr->mConvertCallback == nullptr)
			return nullptr;

		return thisPtr->mConvertCallback(thisPtr->mAsyncOp);
	}

	void ScriptAsyncOp::internal_blockUntilComplete(ScriptAsyncOp* thisPtr)
	{
		if (!thisPtr->mAsyncOp.hasCompleted())
		{
			gCoreThread().submit();
			thisPtr->mAsyncOp.blockUntilComplete();
		}
	}
}