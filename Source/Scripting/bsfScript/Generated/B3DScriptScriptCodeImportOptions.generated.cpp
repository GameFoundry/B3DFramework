//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptScriptCodeImportOptions.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptScriptCodeImportOptions.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptScriptCodeImportOptions::ScriptScriptCodeImportOptions(const TShared<ScriptCodeImportOptions>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptScriptCodeImportOptions::~ScriptScriptCodeImportOptions()
	{
		UnregisterEvents();
	}

	void ScriptScriptCodeImportOptions::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEditorScript", (void*)&ScriptScriptCodeImportOptions::InternalGetEditorScript);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEditorScript", (void*)&ScriptScriptCodeImportOptions::InternalSetEditorScript);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptScriptCodeImportOptions::InternalCreate);

	}

	MonoObject* ScriptScriptCodeImportOptions::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptScriptCodeImportOptions::InternalCreate(MonoObject* scriptObject)
	{
		TShared<ScriptCodeImportOptions> nativeObject = ScriptCodeImportOptions::Create();
		ScriptObjectWrapper::Create<ScriptScriptCodeImportOptions>(nativeObject, scriptObject);
	}
	bool ScriptScriptCodeImportOptions::InternalGetEditorScript(ScriptScriptCodeImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<ScriptCodeImportOptions*>(self->GetNativeObject())->EditorScript;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScriptCodeImportOptions::InternalSetEditorScript(ScriptScriptCodeImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<ScriptCodeImportOptions*>(self->GetNativeObject())->EditorScript = value;
	}
#endif
}
