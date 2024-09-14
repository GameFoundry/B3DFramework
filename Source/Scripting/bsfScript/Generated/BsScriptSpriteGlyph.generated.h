//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "BsScriptSpriteImage.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteGlyph.h"

namespace bs { class SpriteGlyph; }
namespace bs { struct __SpriteGlyphCreateInformationInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteGlyph : public TScriptResourceWrapper<SpriteGlyph, ScriptSpriteGlyph, ScriptSpriteImageWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteGlyph")

		ScriptSpriteGlyph(const TResourceHandle<SpriteGlyph>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptSpriteGlyph* self);

		static void InternalSetFont(ScriptSpriteGlyph* self, MonoObject* font);
		static void InternalSetGlyph(ScriptSpriteGlyph* self, uint32_t glyph);
		static void InternalSetGlyphSize(ScriptSpriteGlyph* self, float size);
		static void InternalCreate(MonoObject* scriptObject, MonoObject* font, uint32_t glyph, float size);
		static void InternalCreate0(MonoObject* scriptObject, __SpriteGlyphCreateInformationInterop* createInformation);
	};
}
