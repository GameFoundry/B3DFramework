//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFontImportOptions.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptCharRange.generated.h"
#include "BsScriptFontImportOptions.generated.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptFontImportOptions::ScriptFontImportOptions(MonoObject* managedInstance, const SPtr<FontImportOptions>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptFontImportOptions::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetFontSizes", (void*)&ScriptFontImportOptions::InternalGetFontSizes);
		metaData.ScriptClass->AddInternalCall("Internal_SetFontSizes", (void*)&ScriptFontImportOptions::InternalSetFontSizes);
		metaData.ScriptClass->AddInternalCall("Internal_GetCharIndexRanges", (void*)&ScriptFontImportOptions::InternalGetCharIndexRanges);
		metaData.ScriptClass->AddInternalCall("Internal_SetCharIndexRanges", (void*)&ScriptFontImportOptions::InternalSetCharIndexRanges);
		metaData.ScriptClass->AddInternalCall("Internal_GetDpi", (void*)&ScriptFontImportOptions::InternalGetDpi);
		metaData.ScriptClass->AddInternalCall("Internal_SetDpi", (void*)&ScriptFontImportOptions::InternalSetDpi);
		metaData.ScriptClass->AddInternalCall("Internal_GetRenderMode", (void*)&ScriptFontImportOptions::InternalGetRenderMode);
		metaData.ScriptClass->AddInternalCall("Internal_SetRenderMode", (void*)&ScriptFontImportOptions::InternalSetRenderMode);
		metaData.ScriptClass->AddInternalCall("Internal_GetBold", (void*)&ScriptFontImportOptions::InternalGetBold);
		metaData.ScriptClass->AddInternalCall("Internal_SetBold", (void*)&ScriptFontImportOptions::InternalSetBold);
		metaData.ScriptClass->AddInternalCall("Internal_GetItalic", (void*)&ScriptFontImportOptions::InternalGetItalic);
		metaData.ScriptClass->AddInternalCall("Internal_SetItalic", (void*)&ScriptFontImportOptions::InternalSetItalic);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptFontImportOptions::InternalCreate);

	}

	MonoObject* ScriptFontImportOptions::Create(const SPtr<FontImportOptions>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptFontImportOptions>()) ScriptFontImportOptions(managedInstance, value);
		return managedInstance;
	}
	void ScriptFontImportOptions::InternalCreate(MonoObject* managedInstance)
	{
		SPtr<FontImportOptions> instance = FontImportOptions::Create();
		new (B3DAllocate<ScriptFontImportOptions>())ScriptFontImportOptions(managedInstance, instance);
	}
	MonoArray* ScriptFontImportOptions::InternalGetFontSizes(ScriptFontImportOptions* thisPtr)
	{
		Vector<float> nativeArray__output;
		nativeArray__output = thisPtr->GetInternal()->FontSizes;

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

	void ScriptFontImportOptions::InternalSetFontSizes(ScriptFontImportOptions* thisPtr, MonoArray* value)
	{
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
		thisPtr->GetInternal()->FontSizes = nativeArrayvalue;
	}

	MonoArray* ScriptFontImportOptions::InternalGetCharIndexRanges(ScriptFontImportOptions* thisPtr)
	{
		Vector<CharRange> nativeArray__output;
		nativeArray__output = thisPtr->GetInternal()->CharIndexRanges;

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

	void ScriptFontImportOptions::InternalSetCharIndexRanges(ScriptFontImportOptions* thisPtr, MonoArray* value)
	{
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
		thisPtr->GetInternal()->CharIndexRanges = nativeArrayvalue;
	}

	uint32_t ScriptFontImportOptions::InternalGetDpi(ScriptFontImportOptions* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->Dpi;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::InternalSetDpi(ScriptFontImportOptions* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->Dpi = value;
	}

	FontRenderMode ScriptFontImportOptions::InternalGetRenderMode(ScriptFontImportOptions* thisPtr)
	{
		FontRenderMode tmp__output;
		tmp__output = thisPtr->GetInternal()->RenderMode;

		FontRenderMode __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::InternalSetRenderMode(ScriptFontImportOptions* thisPtr, FontRenderMode value)
	{
		thisPtr->GetInternal()->RenderMode = value;
	}

	bool ScriptFontImportOptions::InternalGetBold(ScriptFontImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Bold;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::InternalSetBold(ScriptFontImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Bold = value;
	}

	bool ScriptFontImportOptions::InternalGetItalic(ScriptFontImportOptions* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Italic;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontImportOptions::InternalSetItalic(ScriptFontImportOptions* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Italic = value;
	}
#endif
}
