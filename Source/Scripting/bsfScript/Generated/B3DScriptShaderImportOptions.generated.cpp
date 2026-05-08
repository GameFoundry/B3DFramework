//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptShaderImportOptions.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptShaderImportOptions.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptShaderImportOptions::ScriptShaderImportOptions(const TShared<ShaderImportOptions>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptShaderImportOptions::~ScriptShaderImportOptions()
	{
		UnregisterEvents();
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
		if(!self->IsNativeObjectValid())
			return;

		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<ShaderImportOptions*>(self->GetNativeObject())->SetDefine(tmpdefine, tmpvalue);
	}

	bool ScriptShaderImportOptions::InternalGetDefine(ScriptShaderImportOptions* self, MonoString* define, MonoString** outValue)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		String tmpoutValue;
		tmp__output = static_cast<ShaderImportOptions*>(self->GetNativeObject())->GetDefine(tmpdefine, tmpoutValue);

		bool __output;
		__output = tmp__output;
		MonoUtil::ReferenceCopy(outValue,  (MonoObject*)MonoUtil::StringToMono(tmpoutValue));

		return __output;
	}

	bool ScriptShaderImportOptions::InternalHasDefine(ScriptShaderImportOptions* self, MonoString* define)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		tmp__output = static_cast<ShaderImportOptions*>(self->GetNativeObject())->HasDefine(tmpdefine);

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptShaderImportOptions::InternalRemoveDefine(ScriptShaderImportOptions* self, MonoString* define)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpdefine;
		tmpdefine = MonoUtil::MonoToString(define);
		static_cast<ShaderImportOptions*>(self->GetNativeObject())->RemoveDefine(tmpdefine);
	}

	void ScriptShaderImportOptions::InternalCreate(MonoObject* scriptObject)
	{
		TShared<ShaderImportOptions> nativeObject = ShaderImportOptions::Create();
		ScriptObjectWrapper::Create<ScriptShaderImportOptions>(nativeObject, scriptObject);
	}
	ShadingLanguageFlag ScriptShaderImportOptions::InternalGetLanguages(ScriptShaderImportOptions* self)
	{
		Flags<ShadingLanguageFlag> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ShaderImportOptions*>(self->GetNativeObject())->Languages;

		ShadingLanguageFlag __output;
		__output = (ShadingLanguageFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptShaderImportOptions::InternalSetLanguages(ScriptShaderImportOptions* self, ShadingLanguageFlag value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ShaderImportOptions*>(self->GetNativeObject())->Languages = value;
	}
#endif
}
