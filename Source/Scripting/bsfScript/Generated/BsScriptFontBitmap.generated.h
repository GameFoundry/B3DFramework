//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "../../../Foundation/bsfCore/Text/BsFontDesc.h"

namespace bs
{
	struct FontBitmap;
}

namespace bs
{
	struct __CharDescInterop;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptFontBitmap : public TScriptReflectable<ScriptFontBitmap, FontBitmap>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "FontBitmap")

		ScriptFontBitmap(MonoObject* managedInstance, const SPtr<FontBitmap>& value);

		static MonoObject* Create(const SPtr<FontBitmap>& value);

	private:
		static void InternalGetCharDesc(ScriptFontBitmap* thisPtr, uint32_t charId, __CharDescInterop* __output);
		static uint32_t InternalGetSize(ScriptFontBitmap* thisPtr);
		static void InternalSetSize(ScriptFontBitmap* thisPtr, uint32_t value);
		static int32_t InternalGetBaselineOffset(ScriptFontBitmap* thisPtr);
		static void InternalSetBaselineOffset(ScriptFontBitmap* thisPtr, int32_t value);
		static uint32_t InternalGetLineHeight(ScriptFontBitmap* thisPtr);
		static void InternalSetLineHeight(ScriptFontBitmap* thisPtr, uint32_t value);
		static void InternalGetMissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* __output);
		static void InternalSetMissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* value);
		static uint32_t InternalGetSpaceWidth(ScriptFontBitmap* thisPtr);
		static void InternalSetSpaceWidth(ScriptFontBitmap* thisPtr, uint32_t value);
		static MonoArray* InternalGetTexturePages(ScriptFontBitmap* thisPtr);
		static void InternalSetTexturePages(ScriptFontBitmap* thisPtr, MonoArray* value);
	};
} // namespace bs
