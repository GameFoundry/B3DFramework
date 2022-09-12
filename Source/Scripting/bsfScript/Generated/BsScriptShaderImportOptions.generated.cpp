//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptShaderImportOptions.generated.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptShaderImportOptions::ScriptShaderImportOptions(MonoObject* managedInstance, const SPtr<ShaderImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptShaderImportOptions::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setDefine", (void*)&ScriptShaderImportOptions::Internal_setDefine);
		metaData.scriptClass->AddInternalCall("Internal_getDefine", (void*)&ScriptShaderImportOptions::Internal_getDefine);
		metaData.scriptClass->AddInternalCall("Internal_hasDefine", (void*)&ScriptShaderImportOptions::Internal_hasDefine);
		metaData.scriptClass->AddInternalCall("Internal_removeDefine", (void*)&ScriptShaderImportOptions::Internal_removeDefine);
		metaData.scriptClass->AddInternalCall("Internal_getlanguages", (void*)&ScriptShaderImportOptions::Internal_getlanguages);
		metaData.scriptClass->AddInternalCall("Internal_setlanguages", (void*)&ScriptShaderImportOptions::Internal_setlanguages);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptShaderImportOptions::Internal_create);

	}

	MonoObject* ScriptShaderImportOptions::create(const SPtr<ShaderImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptShaderImportOptions>()) ScriptShaderImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptShaderImportOptions::Internal_setDefine(ScriptShaderImportOptions* thisPtr, MonoString* define, MonoString* value)
	{
		String tmpdefine;
		tmpdefine = MonoUtil::monoToString(define);
		String tmpvalue;
		tmpvalue = MonoUtil::monoToString(value);
		thisPtr->GetInternal()->setDefine(tmpdefine, tmpvalue);
	}

	bool ScriptShaderImportOptions::Internal_getDefine(ScriptShaderImportOptions* thisPtr, MonoString* define, MonoString** value)
	{
		bool tmp__output;
		String tmpdefine;
		tmpdefine = MonoUtil::monoToString(define);
		String tmpvalue;
		tmp__output = thisPtr->GetInternal()->getDefine(tmpdefine, tmpvalue);

		bool __output;
		__output = tmp__output;
		MonoUtil::referenceCopy(value,  (MonoObject*)MonoUtil::stringToMono(tmpvalue));

		return __output;
	}

	bool ScriptShaderImportOptions::Internal_hasDefine(ScriptShaderImportOptions* thisPtr, MonoString* define)
	{
		bool tmp__output;
		String tmpdefine;
		tmpdefine = MonoUtil::monoToString(define);
		tmp__output = thisPtr->GetInternal()->hasDefine(tmpdefine);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderImportOptions::Internal_removeDefine(ScriptShaderImportOptions* thisPtr, MonoString* define)
	{
		String tmpdefine;
		tmpdefine = MonoUtil::monoToString(define);
		thisPtr->GetInternal()->removeDefine(tmpdefine);
	}

	void ScriptShaderImportOptions::Internal_create(MonoObject* managedInstance)
	{
		SPtr<ShaderImportOptions> instance = ShaderImportOptions::create();
		new (bs_alloc<ScriptShaderImportOptions>())ScriptShaderImportOptions(managedInstance, instance);
	}
	ShadingLanguageFlag ScriptShaderImportOptions::Internal_getlanguages(ScriptShaderImportOptions* thisPtr)
	{
		Flags<ShadingLanguageFlag> tmp__output;
		tmp__output = thisPtr->GetInternal()->languages;

		ShadingLanguageFlag __output;
		__output = (ShadingLanguageFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptShaderImportOptions::Internal_setlanguages(ScriptShaderImportOptions* thisPtr, ShadingLanguageFlag value)
	{
		thisPtr->GetInternal()->languages = value;
	}
#endif
}
