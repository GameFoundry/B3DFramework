//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAsyncOpEx.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Extensions/BsAsyncOpEx.h"

namespace bs
{
	ScriptAsyncOpEx::ScriptAsyncOpEx(MonoObject* managedInstance, const SPtr<AsyncOpEx>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptAsyncOpEx::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_isComplete", (void*)&ScriptAsyncOpEx::Internal_isComplete);
		metaData.scriptClass->addInternalCall("Internal_getReturnValue", (void*)&ScriptAsyncOpEx::Internal_getReturnValue);
		metaData.scriptClass->addInternalCall("Internal_blockUntilComplete", (void*)&ScriptAsyncOpEx::Internal_blockUntilComplete);

	}

	MonoObject* ScriptAsyncOpEx::create(const SPtr<AsyncOpEx>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptAsyncOpEx>()) ScriptAsyncOpEx(managedInstance, value);
		return managedInstance;
	}
	bool ScriptAsyncOpEx::Internal_isComplete(ScriptAsyncOpEx* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->isComplete();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptAsyncOpEx::Internal_getReturnValue(ScriptAsyncOpEx* thisPtr)
	{
		MonoObject* tmp__output;
		tmp__output = thisPtr->getInternal()->getReturnValue();

		MonoObject* __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAsyncOpEx::Internal_blockUntilComplete(ScriptAsyncOpEx* thisPtr)
	{
		thisPtr->getInternal()->blockUntilComplete();
	}
}
