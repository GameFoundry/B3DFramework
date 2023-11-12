//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFontBitmapInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptCharacterInformation.generated.h"
#include "BsScriptFontBitmapPage.generated.h"

namespace bs
{
	ScriptFontBitmapInformation::ScriptFontBitmapInformation(MonoObject* managedInstance, const SPtr<FontBitmapInformation>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptFontBitmapInformation::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetCharacterInformation", (void*)&ScriptFontBitmapInformation::InternalGetCharacterInformation);
		metaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptFontBitmapInformation::InternalGetSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptFontBitmapInformation::InternalSetSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetBaselineOffset", (void*)&ScriptFontBitmapInformation::InternalGetBaselineOffset);
		metaData.ScriptClass->AddInternalCall("Internal_SetBaselineOffset", (void*)&ScriptFontBitmapInformation::InternalSetBaselineOffset);
		metaData.ScriptClass->AddInternalCall("Internal_GetLineHeight", (void*)&ScriptFontBitmapInformation::InternalGetLineHeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetLineHeight", (void*)&ScriptFontBitmapInformation::InternalSetLineHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetMissingGlyph", (void*)&ScriptFontBitmapInformation::InternalGetMissingGlyph);
		metaData.ScriptClass->AddInternalCall("Internal_SetMissingGlyph", (void*)&ScriptFontBitmapInformation::InternalSetMissingGlyph);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpaceWidth", (void*)&ScriptFontBitmapInformation::InternalGetSpaceWidth);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpaceWidth", (void*)&ScriptFontBitmapInformation::InternalSetSpaceWidth);
		metaData.ScriptClass->AddInternalCall("Internal_GetTexturePages", (void*)&ScriptFontBitmapInformation::InternalGetTexturePages);
		metaData.ScriptClass->AddInternalCall("Internal_SetTexturePages", (void*)&ScriptFontBitmapInformation::InternalSetTexturePages);

	}

	MonoObject* ScriptFontBitmapInformation::Create(const SPtr<FontBitmapInformation>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptFontBitmapInformation>()) ScriptFontBitmapInformation(managedInstance, value);
		return managedInstance;
	}
	void ScriptFontBitmapInformation::InternalGetCharacterInformation(ScriptFontBitmapInformation* thisPtr, uint32_t characterId, __CharacterInformationInterop* __output)
	{
		CharacterInformation tmp__output;
		tmp__output = thisPtr->GetInternal()->GetCharacterInformation(characterId);

		__CharacterInformationInterop interop__output;
		interop__output = ScriptCharacterInformation::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptCharacterInformation::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	float ScriptFontBitmapInformation::InternalGetSize(ScriptFontBitmapInformation* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->Size;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetSize(ScriptFontBitmapInformation* thisPtr, float value)
	{
		thisPtr->GetInternal()->Size = value;
	}

	float ScriptFontBitmapInformation::InternalGetBaselineOffset(ScriptFontBitmapInformation* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->BaselineOffset;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetBaselineOffset(ScriptFontBitmapInformation* thisPtr, float value)
	{
		thisPtr->GetInternal()->BaselineOffset = value;
	}

	float ScriptFontBitmapInformation::InternalGetLineHeight(ScriptFontBitmapInformation* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->LineHeight;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetLineHeight(ScriptFontBitmapInformation* thisPtr, float value)
	{
		thisPtr->GetInternal()->LineHeight = value;
	}

	void ScriptFontBitmapInformation::InternalGetMissingGlyph(ScriptFontBitmapInformation* thisPtr, __CharacterInformationInterop* __output)
	{
		CharacterInformation tmp__output;
		tmp__output = thisPtr->GetInternal()->MissingGlyph;

		__CharacterInformationInterop interop__output;
		interop__output = ScriptCharacterInformation::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptCharacterInformation::GetMetaData()->ScriptClass->GetInternalClassInternal());


	}

	void ScriptFontBitmapInformation::InternalSetMissingGlyph(ScriptFontBitmapInformation* thisPtr, __CharacterInformationInterop* value)
	{
		CharacterInformation tmpvalue;
		tmpvalue = ScriptCharacterInformation::FromInterop(*value);
		thisPtr->GetInternal()->MissingGlyph = tmpvalue;
	}

	float ScriptFontBitmapInformation::InternalGetSpaceWidth(ScriptFontBitmapInformation* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->SpaceWidth;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetSpaceWidth(ScriptFontBitmapInformation* thisPtr, float value)
	{
		thisPtr->GetInternal()->SpaceWidth = value;
	}

	MonoArray* ScriptFontBitmapInformation::InternalGetTexturePages(ScriptFontBitmapInformation* thisPtr)
	{
		Vector<FontBitmapPage> vec__output;
		vec__output = thisPtr->GetInternal()->TexturePages;

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptFontBitmapPage>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptFontBitmapPage::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetTexturePages(ScriptFontBitmapInformation* thisPtr, MonoArray* value)
	{
		Vector<FontBitmapPage> vecvalue;
		if(value != nullptr)
		{
			ScriptArray arrayvalue(value);
			vecvalue.resize(arrayvalue.Size());
			for(int i = 0; i < (int)arrayvalue.Size(); i++)
			{
				vecvalue[i] = ScriptFontBitmapPage::FromInterop(arrayvalue.Get<__FontBitmapPageInterop>(i));
			}

		}
		thisPtr->GetInternal()->TexturePages = vecvalue;
	}
}
