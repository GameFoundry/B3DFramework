//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptFontBitmapInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptCharacterInformation.generated.h"

namespace b3d
{
	ScriptFontBitmapInformation::ScriptFontBitmapInformation(const TShared<FontBitmapInformation>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptFontBitmapInformation::~ScriptFontBitmapInformation()
	{
		UnregisterEvents();
	}

	void ScriptFontBitmapInformation::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCharacterInformation", (void*)&ScriptFontBitmapInformation::InternalGetCharacterInformation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptFontBitmapInformation::InternalGetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptFontBitmapInformation::InternalSetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBaselineOffset", (void*)&ScriptFontBitmapInformation::InternalGetBaselineOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBaselineOffset", (void*)&ScriptFontBitmapInformation::InternalSetBaselineOffset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLineHeight", (void*)&ScriptFontBitmapInformation::InternalGetLineHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLineHeight", (void*)&ScriptFontBitmapInformation::InternalSetLineHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMissingGlyph", (void*)&ScriptFontBitmapInformation::InternalGetMissingGlyph);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMissingGlyph", (void*)&ScriptFontBitmapInformation::InternalSetMissingGlyph);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpaceWidth", (void*)&ScriptFontBitmapInformation::InternalGetSpaceWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpaceWidth", (void*)&ScriptFontBitmapInformation::InternalSetSpaceWidth);

	}

	MonoObject* ScriptFontBitmapInformation::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptFontBitmapInformation::InternalGetCharacterInformation(ScriptFontBitmapInformation* self, uint32_t characterId, __CharacterInformationInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		CharacterInformation tmp__output;
		tmp__output = static_cast<FontBitmapInformation*>(self->GetNativeObject())->GetCharacterInformation(characterId);

		__CharacterInformationInterop interop__output;
		interop__output = ScriptCharacterInformation::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptCharacterInformation::GetMetaData()->ScriptClass->GetInternalClass());
	}

	float ScriptFontBitmapInformation::InternalGetSize(ScriptFontBitmapInformation* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FontBitmapInformation*>(self->GetNativeObject())->Size;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetSize(ScriptFontBitmapInformation* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FontBitmapInformation*>(self->GetNativeObject())->Size = value;
	}

	float ScriptFontBitmapInformation::InternalGetBaselineOffset(ScriptFontBitmapInformation* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FontBitmapInformation*>(self->GetNativeObject())->BaselineOffset;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetBaselineOffset(ScriptFontBitmapInformation* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FontBitmapInformation*>(self->GetNativeObject())->BaselineOffset = value;
	}

	float ScriptFontBitmapInformation::InternalGetLineHeight(ScriptFontBitmapInformation* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FontBitmapInformation*>(self->GetNativeObject())->LineHeight;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetLineHeight(ScriptFontBitmapInformation* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FontBitmapInformation*>(self->GetNativeObject())->LineHeight = value;
	}

	void ScriptFontBitmapInformation::InternalGetMissingGlyph(ScriptFontBitmapInformation* self, __CharacterInformationInterop* __output)
	{
		CharacterInformation tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<FontBitmapInformation*>(self->GetNativeObject())->MissingGlyph;

		__CharacterInformationInterop interop__output;
		interop__output = ScriptCharacterInformation::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptCharacterInformation::GetMetaData()->ScriptClass->GetInternalClass());


	}

	void ScriptFontBitmapInformation::InternalSetMissingGlyph(ScriptFontBitmapInformation* self, __CharacterInformationInterop* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		CharacterInformation tmpvalue;
		tmpvalue = ScriptCharacterInformation::FromInterop(*value);
		static_cast<FontBitmapInformation*>(self->GetNativeObject())->MissingGlyph = tmpvalue;
	}

	float ScriptFontBitmapInformation::InternalGetSpaceWidth(ScriptFontBitmapInformation* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<FontBitmapInformation*>(self->GetNativeObject())->SpaceWidth;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptFontBitmapInformation::InternalSetSpaceWidth(ScriptFontBitmapInformation* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<FontBitmapInformation*>(self->GetNativeObject())->SpaceWidth = value;
	}
}
