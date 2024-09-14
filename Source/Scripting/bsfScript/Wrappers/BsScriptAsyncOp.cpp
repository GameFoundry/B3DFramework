//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAsyncOp.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsApplication.h"
#include "Serialization/BsScriptAssemblyManager.h"

using namespace bs;
ScriptAsyncOpBase::ScriptAsyncOpBase(MonoObject* instance, const AsyncOpBase& op, const std::function<MonoObject*(const Any&)>& convertCallback)
	: ScriptObject(instance), mOp(op), mConvertCallback(convertCallback)
{}

void ScriptAsyncOpBase::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_IsComplete", (void*)&ScriptAsyncOpBase::InternalIsComplete);
	metaData.ScriptClass->AddInternalCall("Internal_BlockUntilComplete", (void*)&ScriptAsyncOpBase::InternalBlockUntilComplete);
	metaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptAsyncOpBase::InternalGetValue);
}

MonoObject* ScriptAsyncOpBase::CreateInternal(const AsyncOpBase& op, const std::function<MonoObject*(const Any&)>& convertCallback, u32 rttiId)
{
	MonoClass* returnTypeClass = nullptr;

	const ScriptTypeMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(rttiId);
	if(scriptWrapperObjectMetaData != nullptr)
		returnTypeClass = scriptWrapperObjectMetaData->ScriptClass;

	if(!returnTypeClass)
	{
		B3D_LOG(Error, Generic, "Unable to create a managed AsyncOp. Using an unsupported return value type?");
		return nullptr;
	}

	return CreateInternal(op, convertCallback, returnTypeClass);
}

MonoObject* ScriptAsyncOpBase::CreateInternal(const AsyncOpBase& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass)
{
	MonoClass* asyncOpClass = nullptr;
	if(!returnTypeClass)
		asyncOpClass = metaData.ScriptClass;
	else
	{
		::MonoClass* rawClass = BindGenericParam(returnTypeClass->GetInternalClass());
		asyncOpClass = MonoManager::Instance().FindClass(rawClass);
	}

	MonoObject* obj = asyncOpClass->CreateInstance();
	new(B3DAllocate<ScriptAsyncOpBase>()) ScriptAsyncOpBase(obj, op, convertCallback);

	return obj;
}

MonoObject* ScriptAsyncOpBase::CreateInternal(const AsyncOpBase& op, const std::function<MonoObject*(const Any&)>& convertCallback)
{
	MonoObject* obj = metaData.ScriptClass->CreateInstance();
	new(B3DAllocate<ScriptAsyncOpBase>()) ScriptAsyncOpBase(obj, op, convertCallback);

	return obj;
}

::MonoClass* ScriptAsyncOpBase::BindGenericParam(::MonoClass* param)
{
	MonoClass* asyncOpClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().GenericAsyncOpClass;

	::MonoClass* params[1] = { param };
	return MonoUtil::BindGenericParameters(asyncOpClass->GetInternalClass(), params, 1);
}

bool ScriptAsyncOpBase::InternalIsComplete(ScriptAsyncOpBase* thisPtr)
{
	return thisPtr->mOp.HasCompleted();
}

void ScriptAsyncOpBase::InternalBlockUntilComplete(ScriptAsyncOpBase* thisPtr)
{
	thisPtr->mOp.BlockUntilComplete();
}

MonoObject* ScriptAsyncOpBase::InternalGetValue(ScriptAsyncOpBase* thisPtr)
{
	if(!thisPtr->mOp.HasCompleted())
		return nullptr;

	if(thisPtr->mConvertCallback == nullptr)
		return nullptr;

	return thisPtr->mConvertCallback(thisPtr->mOp.GetGenericReturnValue());
}

