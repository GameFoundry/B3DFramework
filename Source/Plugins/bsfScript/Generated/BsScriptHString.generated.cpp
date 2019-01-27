//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptHString.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Localization/BsHString.h"

namespace bs
{
	ScriptHString::ScriptHString(MonoObject* managedInstance, const SPtr<HString>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptHString::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_HString", (void*)&ScriptHString::Internal_HString);
		metaData.scriptClass->addInternalCall("Internal_HString0", (void*)&ScriptHString::Internal_HString0);
		metaData.scriptClass->addInternalCall("Internal_HString1", (void*)&ScriptHString::Internal_HString1);
		metaData.scriptClass->addInternalCall("Internal_HString2", (void*)&ScriptHString::Internal_HString2);
		metaData.scriptClass->addInternalCall("Internal_getValue", (void*)&ScriptHString::Internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_setParameter", (void*)&ScriptHString::Internal_setParameter);

	}

	MonoObject* ScriptHString::create(const SPtr<HString>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptHString>()) ScriptHString(managedInstance, value);
		return managedInstance;
	}
	void ScriptHString::Internal_HString(MonoObject* managedInstance, MonoString* identifier, uint32_t stringTableId)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::monoToString(identifier);
		SPtr<HString> instance = bs_shared_ptr_new<HString>(tmpidentifier, stringTableId);
		new (bs_alloc<ScriptHString>())ScriptHString(managedInstance, instance);
	}

	void ScriptHString::Internal_HString0(MonoObject* managedInstance, MonoString* identifier, MonoString* defaultString, uint32_t stringTableId)
	{
		String tmpidentifier;
		tmpidentifier = MonoUtil::monoToString(identifier);
		String tmpdefaultString;
		tmpdefaultString = MonoUtil::monoToString(defaultString);
		SPtr<HString> instance = bs_shared_ptr_new<HString>(tmpidentifier, tmpdefaultString, stringTableId);
		new (bs_alloc<ScriptHString>())ScriptHString(managedInstance, instance);
	}

	void ScriptHString::Internal_HString1(MonoObject* managedInstance, uint32_t stringTableId)
	{
		SPtr<HString> instance = bs_shared_ptr_new<HString>(stringTableId);
		new (bs_alloc<ScriptHString>())ScriptHString(managedInstance, instance);
	}

	void ScriptHString::Internal_HString2(MonoObject* managedInstance)
	{
		SPtr<HString> instance = bs_shared_ptr_new<HString>();
		new (bs_alloc<ScriptHString>())ScriptHString(managedInstance, instance);
	}

	MonoString* ScriptHString::Internal_getValue(ScriptHString* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->getInternal()->getValue();

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	void ScriptHString::Internal_setParameter(ScriptHString* thisPtr, uint32_t idx, MonoString* value)
	{
		String tmpvalue;
		tmpvalue = MonoUtil::monoToString(value);
		thisPtr->getInternal()->setParameter(idx, tmpvalue);
	}
}
