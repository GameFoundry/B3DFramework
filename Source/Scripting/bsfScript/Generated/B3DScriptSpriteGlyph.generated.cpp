//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSpriteGlyph.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Image/B3DSpriteGlyph.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Image/B3DSpriteGlyph.h"
#include "B3DScriptSpriteGlyphCreateInformation.generated.h"
#include "../../../Foundation/Core/Text/B3DFont.h"

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
