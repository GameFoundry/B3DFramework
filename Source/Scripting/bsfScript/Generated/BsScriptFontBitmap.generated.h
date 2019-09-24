//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"

namespace bs { struct FontBitmap; }
namespace bs { struct __CharDescInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptFontBitmap : public TScriptReflectable<ScriptFontBitmap, FontBitmap>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "FontBitmap")

		ScriptFontBitmap(MonoObject* managedInstance, const SPtr<FontBitmap>& value);

		static MonoObject* create(const SPtr<FontBitmap>& value);

	private:
		static void Internal_getCharDesc(ScriptFontBitmap* thisPtr, uint32_t charId, __CharDescInterop* __output);
		static uint32_t Internal_getsize(ScriptFontBitmap* thisPtr);
		static void Internal_setsize(ScriptFontBitmap* thisPtr, uint32_t value);
		static int32_t Internal_getbaselineOffset(ScriptFontBitmap* thisPtr);
		static void Internal_setbaselineOffset(ScriptFontBitmap* thisPtr, int32_t value);
		static uint32_t Internal_getlineHeight(ScriptFontBitmap* thisPtr);
		static void Internal_setlineHeight(ScriptFontBitmap* thisPtr, uint32_t value);
		static void Internal_getmissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* __output);
		static void Internal_setmissingGlyph(ScriptFontBitmap* thisPtr, __CharDescInterop* value);
		static uint32_t Internal_getspaceWidth(ScriptFontBitmap* thisPtr);
		static void Internal_setspaceWidth(ScriptFontBitmap* thisPtr, uint32_t value);
		static MonoArray* Internal_gettexturePages(ScriptFontBitmap* thisPtr);
		static void Internal_settexturePages(ScriptFontBitmap* thisPtr, MonoArray* value);
	};
}
