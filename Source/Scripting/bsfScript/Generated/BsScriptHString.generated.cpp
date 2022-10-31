//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptHString.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Localization/BsHString.h"

using namespace bs;
ScriptHString::ScriptHString(MonoObject* managedInstance, const SPtr<HString>& value)
	: ScriptObject(managedInstance), mInternal(value)
{
}

void ScriptHString::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_HString", (void*)&ScriptHString::InternalHString);
	metaData.ScriptClass->AddInternalCall("Internal_HString0", (void*)&ScriptHString::InternalHString0);
	metaData.ScriptClass->AddInternalCall("Internal_HString1", (void*)&ScriptHString::InternalHString1);
	metaData.ScriptClass->AddInternalCall("Internal_HString2", (void*)&ScriptHString::InternalHString2);
	metaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptHString::InternalGetValue);
	metaData.ScriptClass->AddInternalCall("Internal_SetParameter", (void*)&ScriptHString::InternalSetParameter);
}

MonoObject* ScriptHString::Create(const SPtr<HString>& value)
{
	if(value == nullptr) return nullptr;

	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
	new(B3DAllocate<ScriptHString>()) ScriptHString(managedInstance, value);
	return managedInstance;
}

void ScriptHString::InternalHString(MonoObject* managedInstance, MonoString* identifier, uint32_t stringTableId)
{
	String tmpidentifier;
	tmpidentifier = MonoUtil::MonoToString(identifier);
	SPtr<HString> instance = B3DMakeShared<HString>(tmpidentifier, stringTableId);
	new(B3DAllocate<ScriptHString>()) ScriptHString(managedInstance, instance);
}

void ScriptHString::InternalHString0(MonoObject* managedInstance, MonoString* identifier, MonoString* defaultString, uint32_t stringTableId)
{
	String tmpidentifier;
	tmpidentifier = MonoUtil::MonoToString(identifier);
	String tmpdefaultString;
	tmpdefaultString = MonoUtil::MonoToString(defaultString);
	SPtr<HString> instance = B3DMakeShared<HString>(tmpidentifier, tmpdefaultString, stringTableId);
	new(B3DAllocate<ScriptHString>()) ScriptHString(managedInstance, instance);
}

void ScriptHString::InternalHString1(MonoObject* managedInstance, uint32_t stringTableId)
{
	SPtr<HString> instance = B3DMakeShared<HString>(stringTableId);
	new(B3DAllocate<ScriptHString>()) ScriptHString(managedInstance, instance);
}

void ScriptHString::InternalHString2(MonoObject* managedInstance)
{
	SPtr<HString> instance = B3DMakeShared<HString>();
	new(B3DAllocate<ScriptHString>()) ScriptHString(managedInstance, instance);
}

MonoString* ScriptHString::InternalGetValue(ScriptHString* thisPtr)
{
	String tmp__output;
	tmp__output = thisPtr->GetInternal()->GetValue();

	MonoString* __output;
	__output = MonoUtil::StringToMono(tmp__output);

	return __output;
}

void ScriptHString::InternalSetParameter(ScriptHString* thisPtr, uint32_t idx, MonoString* value)
{
	String tmpvalue;
	tmpvalue = MonoUtil::MonoToString(value);
	thisPtr->GetInternal()->SetParameter(idx, tmpvalue);
}
