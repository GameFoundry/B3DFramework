//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptHString.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptHString::ScriptHString(MonoObject* instance, const HString& string)
		:ScriptObject(instance), mString(string)
	{ }

	void ScriptHString::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptHString::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetParameter", &ScriptHString::internal_setParameter);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptHString::internal_getValue);
	}

	void ScriptHString::internal_createInstance(MonoObject* instance, MonoString* identifier, UINT32 tableId)
	{
		HString string(MonoUtil::monoToWString(identifier), tableId);
		
		new (bs_alloc<ScriptHString>()) ScriptHString(instance, string);
	}

	void ScriptHString::internal_setParameter(ScriptHString* nativeInstance, UINT32 idx, MonoString* value)
	{
		nativeInstance->mString.setParameter(idx, MonoUtil::monoToWString(value));
	}

	void ScriptHString::internal_getValue(ScriptHString* nativeInstance, MonoString** value)
	{
		*value = MonoUtil::wstringToMono(nativeInstance->mString.getValue());
	}
}