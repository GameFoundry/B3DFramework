//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAsyncOp.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptResourceManager.h"
#include "BsApplication.h"
#include "Serialization/BsScriptAssemblyManager.h"

namespace bs
{
	ScriptAsyncOpBase::ScriptAsyncOpBase(MonoObject* instance, const AsyncOpBase& op,
		const std::function<MonoObject*(const Any&)>& convertCallback)
		:ScriptObject(instance), mOp(op), mConvertCallback(convertCallback)
	{ }

	void ScriptAsyncOpBase::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_IsComplete", (void*)&ScriptAsyncOpBase::internal_IsComplete);
		metaData.scriptClass->addInternalCall("Internal_BlockUntilComplete", (void*)&ScriptAsyncOpBase::internal_BlockUntilComplete);
		metaData.scriptClass->addInternalCall("Internal_GetValue", (void*)&ScriptAsyncOpBase::internal_GetValue);
	}

	MonoObject* ScriptAsyncOpBase::_create(const AsyncOpBase& op,
		const std::function<MonoObject*(const Any&)>& convertCallback, UINT32 rttiId)
	{
		MonoClass* returnTypeClass = nullptr;
		BuiltinResourceInfo* resInfo = ScriptAssemblyManager::instance().getBuiltinResourceInfo(rttiId);
		if(resInfo)
			returnTypeClass = resInfo->monoClass;

		ReflectableTypeInfo* reflTypeInfo = ScriptAssemblyManager::instance().getReflectableTypeInfo(rttiId);
		if(reflTypeInfo)
			returnTypeClass = reflTypeInfo->monoClass;

		if(!returnTypeClass)
		{
			BS_LOG(Error, Generic, "Unable to create a managed AsyncOp. Using an unsupported return value type?");
			return nullptr;
		}

		return _create(op, convertCallback, returnTypeClass);
	}

	MonoObject* ScriptAsyncOpBase::_create(const AsyncOpBase& op,
		const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass)
	{
		MonoClass* asyncOpClass = nullptr;
		if(!returnTypeClass)
			asyncOpClass = metaData.scriptClass;
		else
		{
			::MonoClass* rawClass = bindGenericParam(returnTypeClass->_getInternalClass());
			asyncOpClass = MonoManager::instance().findClass(rawClass);
		}

		MonoObject* obj = asyncOpClass->createInstance();
		new (bs_alloc<ScriptAsyncOpBase>()) ScriptAsyncOpBase(obj, op, convertCallback);

		return obj;
	}

	MonoObject* ScriptAsyncOpBase::_create(const AsyncOpBase& op,
		const std::function<MonoObject*(const Any&)>& convertCallback)
	{
		MonoObject* obj = metaData.scriptClass->createInstance();
		new (bs_alloc<ScriptAsyncOpBase>()) ScriptAsyncOpBase(obj, op, convertCallback);

		return obj;
	}

	::MonoClass* ScriptAsyncOpBase::bindGenericParam(::MonoClass* param)
	{
		MonoClass* asyncOpClass = ScriptAssemblyManager::instance().getBuiltinClasses().genericAsyncOpClass;

		::MonoClass* params[1] = { param };
		return MonoUtil::bindGenericParameters(asyncOpClass->_getInternalClass(), params, 1);
	}

	bool ScriptAsyncOpBase::internal_IsComplete(ScriptAsyncOpBase* thisPtr)
	{
		return thisPtr->mOp.hasCompleted();
	}

	void ScriptAsyncOpBase::internal_BlockUntilComplete(ScriptAsyncOpBase* thisPtr)
	{
		thisPtr->mOp.blockUntilComplete();
	}

	MonoObject* ScriptAsyncOpBase::internal_GetValue(ScriptAsyncOpBase* thisPtr)
	{
		if (!thisPtr->mOp.hasCompleted())
			return nullptr;

		if (thisPtr->mConvertCallback == nullptr)
			return nullptr;

		return thisPtr->mConvertCallback(thisPtr->mOp.getGenericReturnValue());
	}

}

