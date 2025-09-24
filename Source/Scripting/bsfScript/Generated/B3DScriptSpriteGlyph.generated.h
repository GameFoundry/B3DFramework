//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "B3DScriptSpriteImage.generated.h"
#include "../../../Foundation/Core/Image/B3DSpriteGlyph.h"

namespace b3d { class SpriteGlyph; }
namespace b3d { struct __SpriteGlyphCreateInformationInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSpriteGlyph : public TScriptResourceWrapper<SpriteGlyph, ScriptSpriteGlyph, ScriptSpriteImageWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SpriteGlyph")

		ScriptSpriteGlyph(const TResourceHandle<SpriteGlyph>& nativeObject);
		~ScriptSpriteGlyph();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptSpriteGlyph* self);

		static void InternalCreate(MonoObject* scriptObject, MonoObject* font, uint32_t glyph, float size);
		static void InternalCreate0(MonoObject* scriptObject, __SpriteGlyphCreateInformationInterop* createInformation);
	};
}
