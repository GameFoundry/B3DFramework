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
	void ScriptFontBitmapInformation::InternalGetCharacterInformation(ScriptFontBitmapInformation* self, uint32_t characterId, __CharacterInformationInterop* __output)
	{
		CharacterInformation tmp__output;
		tmp__output = self->GetInternal()->GetCharacterInformation(characterId);

		__CharacterInformationInterop interop__output;
		interop__output = ScriptCharacterInformation::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptCharacterInformation::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	float ScriptFontBitmapInformation::InternalGetSize(ScriptFontBitmapInformation* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->Size;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetSize(ScriptFontBitmapInformation* self, float value)
	{
		self->GetInternal()->Size = value;
	}

	float ScriptFontBitmapInformation::InternalGetBaselineOffset(ScriptFontBitmapInformation* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->BaselineOffset;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetBaselineOffset(ScriptFontBitmapInformation* self, float value)
	{
		self->GetInternal()->BaselineOffset = value;
	}

	float ScriptFontBitmapInformation::InternalGetLineHeight(ScriptFontBitmapInformation* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->LineHeight;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetLineHeight(ScriptFontBitmapInformation* self, float value)
	{
		self->GetInternal()->LineHeight = value;
	}

	void ScriptFontBitmapInformation::InternalGetMissingGlyph(ScriptFontBitmapInformation* self, __CharacterInformationInterop* __output)
	{
		CharacterInformation tmp__output;
		tmp__output = self->GetInternal()->MissingGlyph;

		__CharacterInformationInterop interop__output;
		interop__output = ScriptCharacterInformation::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptCharacterInformation::GetMetaData()->ScriptClass->GetInternalClassInternal());


	}

	void ScriptFontBitmapInformation::InternalSetMissingGlyph(ScriptFontBitmapInformation* self, __CharacterInformationInterop* value)
	{
		CharacterInformation tmpvalue;
		tmpvalue = ScriptCharacterInformation::FromInterop(*value);
		self->GetInternal()->MissingGlyph = tmpvalue;
	}

	float ScriptFontBitmapInformation::InternalGetSpaceWidth(ScriptFontBitmapInformation* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->SpaceWidth;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetSpaceWidth(ScriptFontBitmapInformation* self, float value)
	{
		self->GetInternal()->SpaceWidth = value;
	}

	MonoArray* ScriptFontBitmapInformation::InternalGetTexturePages(ScriptFontBitmapInformation* self)
	{
		Vector<FontBitmapPage> nativeArray__output;
		nativeArray__output = self->GetInternal()->TexturePages;

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptFontBitmapPage>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptFontBitmapPage::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetTexturePages(ScriptFontBitmapInformation* self, MonoArray* value)
	{
		Vector<FontBitmapPage> nativeArrayvalue;
		if(value != nullptr)
		{
			ScriptArray scriptArrayvalue(value);
			nativeArrayvalue.resize(scriptArrayvalue.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayvalue.Size(); elementIndex++)
			{
				nativeArrayvalue[elementIndex] = ScriptFontBitmapPage::FromInterop(scriptArrayvalue.Get<__FontBitmapPageInterop>(elementIndex));
			}

		}
		self->GetInternal()->TexturePages = nativeArrayvalue;
	}
}
