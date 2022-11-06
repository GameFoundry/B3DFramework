//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFontImportOptions : public TScriptReflectable<ScriptFontImportOptions, FontImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "FontImportOptions")

		ScriptFontImportOptions(MonoObject* managedInstance, const SPtr<FontImportOptions>& value);

		static MonoObject* Create(const SPtr<FontImportOptions>& value);

	private:
		static MonoArray* InternalGetFontSizes(ScriptFontImportOptions* thisPtr);
		static void InternalSetFontSizes(ScriptFontImportOptions* thisPtr, MonoArray* value);
		static MonoArray* InternalGetCharIndexRanges(ScriptFontImportOptions* thisPtr);
		static void InternalSetCharIndexRanges(ScriptFontImportOptions* thisPtr, MonoArray* value);
		static uint32_t InternalGetDpi(ScriptFontImportOptions* thisPtr);
		static void InternalSetDpi(ScriptFontImportOptions* thisPtr, uint32_t value);
		static FontRenderMode InternalGetRenderMode(ScriptFontImportOptions* thisPtr);
		static void InternalSetRenderMode(ScriptFontImportOptions* thisPtr, FontRenderMode value);
		static bool InternalGetBold(ScriptFontImportOptions* thisPtr);
		static void InternalSetBold(ScriptFontImportOptions* thisPtr, bool value);
		static bool InternalGetItalic(ScriptFontImportOptions* thisPtr);
		static void InternalSetItalic(ScriptFontImportOptions* thisPtr, bool value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
}
