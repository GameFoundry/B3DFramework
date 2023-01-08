//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptHString.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Localization/BsHString.h"

namespace bs
{
	ScriptLocString::ScriptLocString(MonoObject* managedInstance, const SPtr<HString>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptLocString::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_HString", (void*)&ScriptLocString::InternalHString);
		metaData.ScriptClass->AddInternalCall("Internal_HString0", (void*)&ScriptLocString::InternalHString0);
		metaData.ScriptClass->AddInternalCall("Internal_HString1", (void*)&ScriptLocString::InternalHString1);
		metaData.ScriptClass->AddInternalCall("Internal_HString2", (void*)&ScriptLocString::InternalHString2);
		metaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptLocString::InternalGetValue);
		metaData.ScriptClass->AddInternalCall("Internal_SetParameter", (void*)&ScriptLocString::InternalSetParameter);

	}

	MonoObject* ScriptLocString::Create(const SPtr<HString>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptLocString>()) ScriptLocString(managedInstance, value);
		return managedInstance;
	}
	void ScriptLocString::InternalHString(MonoObject* managedInstance, MonoString* identifier, uint32_t stringTableId)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		SPtr<HString> instance = B3DMakeShared<HString>(tmpidentifier, stringTableId);
		new (B3DAllocate<ScriptLocString>())ScriptLocString(managedInstance, instance);
	}

	void ScriptLocString::InternalHString0(MonoObject* managedInstance, MonoString* identifier, MonoString* defaultString, uint32_t stringTableId)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::MonoToString(identifier);
		String tmpdefaultString;
		tmpdefaultString = MonoUtil::MonoToString(defaultString);
		SPtr<HString> instance = B3DMakeShared<HString>(tmpidentifier, tmpdefaultString, stringTableId);
		new (B3DAllocate<ScriptLocString>())ScriptLocString(managedInstance, instance);
	}

	void ScriptLocString::InternalHString1(MonoObject* managedInstance, uint32_t stringTableId)
	{
		SPtr<HString> instance = B3DMakeShared<HString>(stringTableId);
		new (B3DAllocate<ScriptLocString>())ScriptLocString(managedInstance, instance);
	}

	void ScriptLocString::InternalHString2(MonoObject* managedInstance)
	{
		SPtr<HString> instance = B3DMakeShared<HString>();
		new (B3DAllocate<ScriptLocString>())ScriptLocString(managedInstance, instance);
	}

	MonoString* ScriptLocString::InternalGetValue(ScriptLocString* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->GetInternal()->GetValue();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptLocString::InternalSetParameter(ScriptLocString* thisPtr, uint32_t idx, MonoString* value)
	{
		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		thisPtr->GetInternal()->SetParameter(idx, tmpvalue);
	}
}
