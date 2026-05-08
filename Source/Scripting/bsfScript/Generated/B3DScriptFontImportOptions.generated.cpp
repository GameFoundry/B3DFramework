//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptFontImportOptions.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptCharRange.generated.h"
#include "B3DScriptFontImportOptions.generated.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptFontImportOptions::ScriptFontImportOptions(const TShared<FontImportOptions>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptFontImportOptions::~ScriptFontImportOptions()
	{
		UnregisterEvents();
	}

	void ScriptFontImportOptions::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFontSizes", (void*)&ScriptFontImportOptions::InternalGetFontSizes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFontSizes", (void*)&ScriptFontImportOptions::InternalSetFontSizes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCharIndexRanges", (void*)&ScriptFontImportOptions::InternalGetCharIndexRanges);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCharIndexRanges", (void*)&ScriptFontImportOptions::InternalSetCharIndexRanges);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDpi", (void*)&ScriptFontImportOptions::InternalGetDpi);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDpi", (void*)&ScriptFontImportOptions::InternalSetDpi);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRenderMode", (void*)&ScriptFontImportOptions::InternalGetRenderMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRenderMode", (void*)&ScriptFontImportOptions::InternalSetRenderMode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBold", (void*)&ScriptFontImportOptions::InternalGetBold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBold", (void*)&ScriptFontImportOptions::InternalSetBold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetItalic", (void*)&ScriptFontImportOptions::InternalGetItalic);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetItalic", (void*)&ScriptFontImportOptions::InternalSetItalic);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptFontImportOptions::InternalCreate);

	}

	MonoObject* ScriptFontImportOptions::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptFontImportOptions::InternalCreate(MonoObject* scriptObject)
	{
		TShared<FontImportOptions> nativeObject = FontImportOptions::Create();
		ScriptObjectWrapper::Create<ScriptFontImportOptions>(nativeObject, scriptObject);
	}
	MonoArray* ScriptFontImportOptions::InternalGetFontSizes(ScriptFontImportOptions* self)
	{
		Vector<float> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<FontImportOptions*>(self->GetNativeObject())->FontSizes;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<float>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptFontImportOptions::InternalSetFontSizes(ScriptFontImportOptions* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<float> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<float>(elementIndex);
			}

		}
		static_cast<FontImportOptions*>(self->GetNativeObject())->FontSizes = nativeArrayvalue;
	}

	MonoArray* ScriptFontImportOptions::InternalGetCharIndexRanges(ScriptFontImportOptions* self)
	{
		Vector<CharRange> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<FontImportOptions*>(self->GetNativeObject())->CharIndexRanges;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptCharRange>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, nativeArray__output[elementIndex]);
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptFontImportOptions::InternalSetCharIndexRanges(ScriptFontImportOptions* self, MonoArray* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		Vector<CharRange> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = scriptArrayvalue.Get<CharRange>(elementIndex);
			}

		}
		static_cast<FontImportOptions*>(self->GetNativeObject())->CharIndexRanges = nativeArrayvalue;
	}

	uint32_t ScriptFontImportOptions::InternalGetDpi(ScriptFontImportOptions* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FontImportOptions*>(self->GetNativeObject())->Dpi;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::InternalSetDpi(ScriptFontImportOptions* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FontImportOptions*>(self->GetNativeObject())->Dpi = value;
	}

	FontRenderMode ScriptFontImportOptions::InternalGetRenderMode(ScriptFontImportOptions* self)
	{
		FontRenderMode tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FontImportOptions*>(self->GetNativeObject())->RenderMode;

		FontRenderMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::InternalSetRenderMode(ScriptFontImportOptions* self, FontRenderMode value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FontImportOptions*>(self->GetNativeObject())->RenderMode = value;
	}

	bool ScriptFontImportOptions::InternalGetBold(ScriptFontImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FontImportOptions*>(self->GetNativeObject())->Bold;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::InternalSetBold(ScriptFontImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FontImportOptions*>(self->GetNativeObject())->Bold = value;
	}

	bool ScriptFontImportOptions::InternalGetItalic(ScriptFontImportOptions* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FontImportOptions*>(self->GetNativeObject())->Italic;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::InternalSetItalic(ScriptFontImportOptions* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FontImportOptions*>(self->GetNativeObject())->Italic = value;
	}
#endif
}
