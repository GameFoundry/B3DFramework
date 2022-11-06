//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptShaderImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptShaderImportOptions.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptShaderImportOptions::ScriptShaderImportOptions(MonoObject* managedInstance, const SPtr<ShaderImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptShaderImportOptions::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetDefine", (void*)&ScriptShaderImportOptions::InternalSetDefine);
		metaData.ScriptClass->AddInternalCall("Internal_GetDefine", (void*)&ScriptShaderImportOptions::InternalGetDefine);
		metaData.ScriptClass->AddInternalCall("Internal_HasDefine", (void*)&ScriptShaderImportOptions::InternalHasDefine);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveDefine", (void*)&ScriptShaderImportOptions::InternalRemoveDefine);
		metaData.ScriptClass->AddInternalCall("Internal_GetLanguages", (void*)&ScriptShaderImportOptions::InternalGetLanguages);
		metaData.ScriptClass->AddInternalCall("Internal_SetLanguages", (void*)&ScriptShaderImportOptions::InternalSetLanguages);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptShaderImportOptions::InternalCreate);

	}

	MonoObject* ScriptShaderImportOptions::Create(const SPtr<ShaderImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptShaderImportOptions>()) ScriptShaderImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptShaderImportOptions::InternalSetDefine(ScriptShaderImportOptions* thisPtr, MonoString* define, MonoString* value)
	{
		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		thisPtr->GetInternal()->SetDefine(tmpdefine, tmpvalue);
	}

	bool ScriptShaderImportOptions::InternalGetDefine(ScriptShaderImportOptions* thisPtr, MonoString* define, MonoString** value)
	{
		bool tmp__output;
		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		String tmpvalue;
		tmp__output = thisPtr->GetInternal()->GetDefine(tmpdefine, tmpvalue);

		bool __output;
		__output = tmp__output;
		MonoUtil::ReferenceCopy(value,  (MonoObject*)MonoUtil::StringToMono(tmpvalue));

		return __output;
	}

	bool ScriptShaderImportOptions::InternalHasDefine(ScriptShaderImportOptions* thisPtr, MonoString* define)
	{
		bool tmp__output;
		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		tmp__output = thisPtr->GetInternal()->HasDefine(tmpdefine);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderImportOptions::InternalRemoveDefine(ScriptShaderImportOptions* thisPtr, MonoString* define)
	{
		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		thisPtr->GetInternal()->RemoveDefine(tmpdefine);
	}

	void ScriptShaderImportOptions::InternalCreate(MonoObject* managedInstance)
	{
		SPtr<ShaderImportOptions> instance = ShaderImportOptions::Create();
		new (B3DAllocate<ScriptShaderImportOptions>())ScriptShaderImportOptions(managedInstance, instance);
	}
	ShadingLanguageFlag ScriptShaderImportOptions::InternalGetLanguages(ScriptShaderImportOptions* thisPtr)
	{
		Flags<ShadingLanguageFlag> tmp__output;
		tmp__output = thisPtr->GetInternal()->Languages;

		ShadingLanguageFlag __output;
		__output = (ShadingLanguageFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptShaderImportOptions::InternalSetLanguages(ScriptShaderImportOptions* thisPtr, ShadingLanguageFlag value)
	{
		thisPtr->GetInternal()->Languages = value;
	}
#endif
}
