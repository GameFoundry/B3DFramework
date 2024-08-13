//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptSpriteImage.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteGlyph.h"

namespace bs { class SpriteGlyph; }
namespace bs { struct __SpriteGlyphCreateInformationInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteGlyph : public TScriptResource<ScriptSpriteGlyph, SpriteGlyph, ScriptSpriteImageBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SpriteGlyph")

		ScriptSpriteGlyph(MonoObject* managedInstance, const TResourceHandle<SpriteGlyph>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptSpriteGlyph* self);

		static void InternalSetFont(ScriptSpriteGlyph* self, MonoObject* font);
		static void InternalSetGlyph(ScriptSpriteGlyph* self, uint32_t glyph);
		static void InternalSetGlyphSize(ScriptSpriteGlyph* self, float size);
		static void InternalCreate(MonoObject* managedInstance, MonoObject* font, uint32_t glyph, float size);
		static void InternalCreate0(MonoObject* managedInstance, __SpriteGlyphCreateInformationInterop* createInformation);
	};
}
