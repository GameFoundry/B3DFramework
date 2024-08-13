//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"

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
		static MonoArray* InternalGetFontSizes(ScriptFontImportOptions* self);
		static void InternalSetFontSizes(ScriptFontImportOptions* self, MonoArray* value);
		static MonoArray* InternalGetCharIndexRanges(ScriptFontImportOptions* self);
		static void InternalSetCharIndexRanges(ScriptFontImportOptions* self, MonoArray* value);
		static uint32_t InternalGetDpi(ScriptFontImportOptions* self);
		static void InternalSetDpi(ScriptFontImportOptions* self, uint32_t value);
		static FontRenderMode InternalGetRenderMode(ScriptFontImportOptions* self);
		static void InternalSetRenderMode(ScriptFontImportOptions* self, FontRenderMode value);
		static bool InternalGetBold(ScriptFontImportOptions* self);
		static void InternalSetBold(ScriptFontImportOptions* self, bool value);
		static bool InternalGetItalic(ScriptFontImportOptions* self);
		static void InternalSetItalic(ScriptFontImportOptions* self, bool value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
}
