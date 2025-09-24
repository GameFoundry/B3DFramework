//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteGlyph.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteGlyph.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteGlyph.h"
#include "BsScriptSpriteGlyphCreateInformation.generated.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"

namespace b3d
{
	ScriptSpriteGlyph::ScriptSpriteGlyph(const TResourceHandle<SpriteGlyph>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSpriteGlyph::~ScriptSpriteGlyph()
	{
		UnregisterEvents();
	}

	void ScriptSpriteGlyph::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptSpriteGlyph::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptSpriteGlyph::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptSpriteGlyph::InternalCreate0);

	}

	MonoObject* ScriptSpriteGlyph::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptSpriteGlyph::InternalGetRef(ScriptSpriteGlyph* self)
	{
		return self->GetOrCreateResourceReference();
	}

	void ScriptSpriteGlyph::InternalCreate(MonoObject* scriptObject, MonoObject* font, uint32_t glyph, float size)
	{
		TResourceHandle<Font> tmpfont;
		ScriptRRefBase* scriptObjectWrapperfont;
		scriptObjectWrapperfont = ScriptRRefBase::GetScriptObjectWrapper(font);
		if(scriptObjectWrapperfont != nullptr)
			tmpfont = B3DStaticResourceCast<Font>(scriptObjectWrapperfont->GetNativeObject());
		TResourceHandle<SpriteGlyph> nativeObject = SpriteGlyph::Create(tmpfont, glyph, size);
		ScriptObjectWrapper::Create<ScriptSpriteGlyph>(nativeObject, scriptObject);
	}

	void ScriptSpriteGlyph::InternalCreate0(MonoObject* scriptObject, __SpriteGlyphCreateInformationInterop* createInformation)
	{
		SpriteGlyphCreateInformation tmpcreateInformation;
		tmpcreateInformation = ScriptSpriteGlyphCreateInformation::FromInterop(*createInformation);
		TResourceHandle<SpriteGlyph> nativeObject = SpriteGlyph::Create(tmpcreateInformation);
		ScriptObjectWrapper::Create<ScriptSpriteGlyph>(nativeObject, scriptObject);
	}
}
