//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"

namespace bs { class FontImportOptions; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptFontImportOptions : public TScriptReflectable<ScriptFontImportOptions, FontImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "FontImportOptions")

		ScriptFontImportOptions(MonoObject* managedInstance, const SPtr<FontImportOptions>& value);

		static MonoObject* create(const SPtr<FontImportOptions>& value);

	private:
		static MonoArray* Internal_getfontSizes(ScriptFontImportOptions* thisPtr);
		static void Internal_setfontSizes(ScriptFontImportOptions* thisPtr, MonoArray* value);
		static MonoArray* Internal_getcharIndexRanges(ScriptFontImportOptions* thisPtr);
		static void Internal_setcharIndexRanges(ScriptFontImportOptions* thisPtr, MonoArray* value);
		static uint32_t Internal_getdpi(ScriptFontImportOptions* thisPtr);
		static void Internal_setdpi(ScriptFontImportOptions* thisPtr, uint32_t value);
		static FontRenderMode Internal_getrenderMode(ScriptFontImportOptions* thisPtr);
		static void Internal_setrenderMode(ScriptFontImportOptions* thisPtr, FontRenderMode value);
		static bool Internal_getbold(ScriptFontImportOptions* thisPtr);
		static void Internal_setbold(ScriptFontImportOptions* thisPtr, bool value);
		static bool Internal_getitalic(ScriptFontImportOptions* thisPtr);
		static void Internal_setitalic(ScriptFontImportOptions* thisPtr, bool value);
		static void Internal_create(MonoObject* managedInstance);
	};
#endif
}
