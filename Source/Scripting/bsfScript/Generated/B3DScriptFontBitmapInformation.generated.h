//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Text/B3DFont.h"
#include "../../../Engine/Core/Text/B3DFont.h"

namespace b3d { struct FontBitmapInformation; }
namespace b3d { struct __CharacterInformationInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFontBitmapInformation : public TScriptReflectableWrapper<FontBitmapInformation, ScriptFontBitmapInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FontBitmapInformation")

		ScriptFontBitmapInformation(const TShared<FontBitmapInformation>& nativeObject);
		~ScriptFontBitmapInformation();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalGetCharacterInformation(ScriptFontBitmapInformation* self, uint32_t characterId, __CharacterInformationInterop* __output);
		static float InternalGetSize(ScriptFontBitmapInformation* self);
		static void InternalSetSize(ScriptFontBitmapInformation* self, float value);
		static float InternalGetBaselineOffset(ScriptFontBitmapInformation* self);
		static void InternalSetBaselineOffset(ScriptFontBitmapInformation* self, float value);
		static float InternalGetLineHeight(ScriptFontBitmapInformation* self);
		static void InternalSetLineHeight(ScriptFontBitmapInformation* self, float value);
		static void InternalGetMissingGlyph(ScriptFontBitmapInformation* self, __CharacterInformationInterop* __output);
		static void InternalSetMissingGlyph(ScriptFontBitmapInformation* self, __CharacterInformationInterop* value);
		static float InternalGetSpaceWidth(ScriptFontBitmapInformation* self);
		static void InternalSetSpaceWidth(ScriptFontBitmapInformation* self, float value);
	};
}
