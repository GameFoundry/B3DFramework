//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"

namespace bs { struct FontBitmapInformation; }
namespace bs { struct __CharacterInformationInterop; }
namespace bs { struct __FontBitmapPageInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFontBitmapInformation : public TScriptReflectable<ScriptFontBitmapInformation, FontBitmapInformation>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "FontBitmapInformation")

		ScriptFontBitmapInformation(MonoObject* managedInstance, const SPtr<FontBitmapInformation>& value);

		static MonoObject* Create(const SPtr<FontBitmapInformation>& value);

	private:
		static void InternalGetCharacterInformation(ScriptFontBitmapInformation* thisPtr, uint32_t characterId, __CharacterInformationInterop* __output);
		static float InternalGetSize(ScriptFontBitmapInformation* thisPtr);
		static void InternalSetSize(ScriptFontBitmapInformation* thisPtr, float value);
		static float InternalGetBaselineOffset(ScriptFontBitmapInformation* thisPtr);
		static void InternalSetBaselineOffset(ScriptFontBitmapInformation* thisPtr, float value);
		static float InternalGetLineHeight(ScriptFontBitmapInformation* thisPtr);
		static void InternalSetLineHeight(ScriptFontBitmapInformation* thisPtr, float value);
		static void InternalGetMissingGlyph(ScriptFontBitmapInformation* thisPtr, __CharacterInformationInterop* __output);
		static void InternalSetMissingGlyph(ScriptFontBitmapInformation* thisPtr, __CharacterInformationInterop* value);
		static float InternalGetSpaceWidth(ScriptFontBitmapInformation* thisPtr);
		static void InternalSetSpaceWidth(ScriptFontBitmapInformation* thisPtr, float value);
		static MonoArray* InternalGetTexturePages(ScriptFontBitmapInformation* thisPtr);
		static void InternalSetTexturePages(ScriptFontBitmapInformation* thisPtr, MonoArray* value);
	};
}
