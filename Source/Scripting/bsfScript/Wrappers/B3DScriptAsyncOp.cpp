//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAsyncOp.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DApplication.h"
#include "Serialization/B3DScriptAssemblyManager.h"

using namespace b3d;
ScriptAsyncOpBase::ScriptAsyncOpBase(const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback)
	: TScriptValueTypeWrapper(op), mConvertCallback(convertCallback)
{}

void ScriptAsyncOpBase::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsComplete", (void*)&ScriptAsyncOpBase::InternalIsComplete);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_BlockUntilComplete", (void*)&ScriptAsyncOpBase::InternalBlockUntilComplete);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptAsyncOpBase::InternalGetValue);
}

MonoObject* ScriptAsyncOpBase::CreateInternal(const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback, u32 rttiId)
{
	MonoClass* returnTypeClass = nullptr;

	const ScriptTypeMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(rttiId);
	if(scriptWrapperObjectMetaData != nullptr)
		returnTypeClass = scriptWrapperObjectMetaData->ScriptClass;

	if(!returnTypeClass)
	{
		B3D_LOG(Error, LogGeneric, "Unable to create a managed AsyncOp. Using an unsupported return value type?");
		return nullptr;
	}

	return CreateInternal(op, convertCallback, returnTypeClass);
}

MonoObject* ScriptAsyncOpBase::CreateInternal(const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback, MonoClass* returnTypeClass)
{
	MonoClass* asyncOpClass = nullptr;
	if(!returnTypeClass)
		asyncOpClass = sInteropMetaData.ScriptClass;
	else
	{
		::MonoClass* rawClass = BindGenericParam(returnTypeClass->GetInternalClass());
		asyncOpClass = MonoManager::Instance().FindClass(rawClass);
	}

	MonoObject* const scriptObject = asyncOpClass->CreateInstance();
	ScriptAsyncOpBase* const scriptWrapper = B3DNew<ScriptAsyncOpBase>(op, convertCallback);
	scriptWrapper->BindToScriptObject(scriptObject);

	return scriptObject;
}

MonoObject* ScriptAsyncOpBase::CreateInternal(const AsyncOp& op, const std::function<MonoObject*(const Any&)>& convertCallback)
{
	MonoObject* const scriptObject = sInteropMetaData.ScriptClass->CreateInstance();
	ScriptAsyncOpBase* const scriptWrapper = B3DNew<ScriptAsyncOpBase>(op, convertCallback);
	scriptWrapper->BindToScriptObject(scriptObject);

	return scriptObject;
}

::MonoClass* ScriptAsyncOpBase::BindGenericParam(::MonoClass* param)
{
	MonoClass* asyncOpClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().GenericAsyncOpClass;

	::MonoClass* params[1] = { param };
	return MonoUtil::BindGenericParameters(asyncOpClass->GetInternalClass(), params, 1);
}

bool ScriptAsyncOpBase::InternalIsComplete(ScriptAsyncOpBase* thisPtr)
{
	return thisPtr->GetNativeObject().HasCompleted();
}

void ScriptAsyncOpBase::InternalBlockUntilComplete(ScriptAsyncOpBase* thisPtr)
{
	thisPtr->GetNativeObject().BlockUntilComplete();
}

MonoObject* ScriptAsyncOpBase::InternalGetValue(ScriptAsyncOpBase* thisPtr)
{
	if(!thisPtr->GetNativeObject().HasCompleted())
		return nullptr;

	if(thisPtr->mConvertCallback == nullptr)
		return nullptr;

	return thisPtr->mConvertCallback(thisPtr->GetNativeObject().GetGenericReturnValue());
}

