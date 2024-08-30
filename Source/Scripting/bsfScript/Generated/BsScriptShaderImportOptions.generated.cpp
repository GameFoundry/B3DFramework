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
	ScriptShaderImportOptions::ScriptShaderImportOptions(const SPtr<ShaderImportOptions>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptShaderImportOptions::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDefine", (void*)&ScriptShaderImportOptions::InternalSetDefine);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDefine", (void*)&ScriptShaderImportOptions::InternalGetDefine);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_HasDefine", (void*)&ScriptShaderImportOptions::InternalHasDefine);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveDefine", (void*)&ScriptShaderImportOptions::InternalRemoveDefine);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLanguages", (void*)&ScriptShaderImportOptions::InternalGetLanguages);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLanguages", (void*)&ScriptShaderImportOptions::InternalSetLanguages);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptShaderImportOptions::InternalCreate);

	}

	MonoObject* ScriptShaderImportOptions::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptShaderImportOptions::InternalSetDefine(ScriptShaderImportOptions* self, MonoString* define, MonoString* value)
	{
		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ShaderImportOptions*>(self->GetNativeObject())->SetDefine(tmpdefine, tmpvalue);
	}

	bool ScriptShaderImportOptions::InternalGetDefine(ScriptShaderImportOptions* self, MonoString* define, MonoString** value)
	{
		bool tmp__output;
		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		String tmpvalue;
		tmp__output = static_cast<ShaderImportOptions*>(self->GetNativeObject())->GetDefine(tmpdefine, tmpvalue);

		bool __output;
		__output = tmp__output;
		MonoUtil::ReferenceCopy(value,  (MonoObject*)MonoUtil::StringToMono(tmpvalue));

		return __output;
	}

	bool ScriptShaderImportOptions::InternalHasDefine(ScriptShaderImportOptions* self, MonoString* define)
	{
		bool tmp__output;
		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		tmp__output = static_cast<ShaderImportOptions*>(self->GetNativeObject())->HasDefine(tmpdefine);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderImportOptions::InternalRemoveDefine(ScriptShaderImportOptions* self, MonoString* define)
	{
		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		static_cast<ShaderImportOptions*>(self->GetNativeObject())->RemoveDefine(tmpdefine);
	}

	void ScriptShaderImportOptions::InternalCreate(MonoObject* scriptObject)
	{
		SPtr<ShaderImportOptions> nativeObject = ShaderImportOptions::Create();
		B3DNew<ScriptShaderImportOptions>(nativeObject, scriptObject);
	}
	ShadingLanguageFlag ScriptShaderImportOptions::InternalGetLanguages(ScriptShaderImportOptions* self)
	{
		Flags<ShadingLanguageFlag> tmp__output;
		tmp__output = static_cast<ShaderImportOptions*>(self->GetNativeObject())->Languages;

		ShadingLanguageFlag __output;
		__output = (ShadingLanguageFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptShaderImportOptions::InternalSetLanguages(ScriptShaderImportOptions* self, ShadingLanguageFlag value)
	{
		static_cast<ShaderImportOptions*>(self->GetNativeObject())->Languages = value;
	}
#endif
}
