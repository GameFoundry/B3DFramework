//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteGlyph.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteGlyph.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptSpriteGlyphCreateInformation.generated.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteGlyph.h"

namespace bs
{
	ScriptSpriteGlyph::ScriptSpriteGlyph(MonoObject* managedInstance, const TResourceHandle<SpriteGlyph>& value)
		:TScriptResource(managedInstance, value)
	{
	}

	void ScriptSpriteGlyph::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteGlyph::InternalGetRef);
		metaData.ScriptClass->AddInternalCall("Internal_SetFont", (void*)&ScriptSpriteGlyph::InternalSetFont);
		metaData.ScriptClass->AddInternalCall("Internal_SetGlyph", (void*)&ScriptSpriteGlyph::InternalSetGlyph);
		metaData.ScriptClass->AddInternalCall("Internal_SetGlyphSize", (void*)&ScriptSpriteGlyph::InternalSetGlyphSize);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSpriteGlyph::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptSpriteGlyph::InternalCreate0);

	}

	 MonoObject*ScriptSpriteGlyph::CreateInstance()
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		return metaData.ScriptClass->CreateInstance("bool", ctorParams);
	}
	MonoObject* ScriptSpriteGlyph::InternalGetRef(ScriptSpriteGlyph* thisPtr)
	{
		return thisPtr->GetRRef();
	}

	void ScriptSpriteGlyph::InternalSetFont(ScriptSpriteGlyph* thisPtr, MonoObject* font)
	{
		TResourceHandle<Font> tmpfont;
		ScriptRRefBase* scriptObjectWrapperfont;
		scriptObjectWrapperfont = ScriptRRefBase::ToNative(font);
		if(scriptObjectWrapperfont != nullptr)
			tmpfont = B3DStaticResourceCast<Font>(scriptObjectWrapperfont->GetHandle());
		thisPtr->GetHandle()->SetFont(tmpfont);
	}

	void ScriptSpriteGlyph::InternalSetGlyph(ScriptSpriteGlyph* thisPtr, uint32_t glyph)
	{
		thisPtr->GetHandle()->SetGlyph(glyph);
	}

	void ScriptSpriteGlyph::InternalSetGlyphSize(ScriptSpriteGlyph* thisPtr, float size)
	{
		thisPtr->GetHandle()->SetGlyphSize(size);
	}

	void ScriptSpriteGlyph::InternalCreate(MonoObject* managedInstance, MonoObject* font, uint32_t glyph, float size)
	{
		TResourceHandle<Font> tmpfont;
		ScriptRRefBase* scriptObjectWrapperfont;
		scriptObjectWrapperfont = ScriptRRefBase::ToNative(font);
		if(scriptObjectWrapperfont != nullptr)
			tmpfont = B3DStaticResourceCast<Font>(scriptObjectWrapperfont->GetHandle());
		TResourceHandle<SpriteGlyph> instance = SpriteGlyph::Create(tmpfont, glyph, size);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}

	void ScriptSpriteGlyph::InternalCreate0(MonoObject* managedInstance, __SpriteGlyphCreateInformationInterop* createInformation)
	{
		SpriteGlyphCreateInformation tmpcreateInformation;
		tmpcreateInformation = ScriptSpriteGlyphCreateInformation::FromInterop(*createInformation);
		TResourceHandle<SpriteGlyph> instance = SpriteGlyph::Create(tmpcreateInformation);
		ScriptResourceManager::Instance().CreateBuiltinScriptResource(instance, managedInstance);
	}
}
