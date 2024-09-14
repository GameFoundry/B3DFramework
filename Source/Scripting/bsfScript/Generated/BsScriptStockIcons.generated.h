//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Text/BsStockIcons.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Text/BsStockIcons.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptStockIcons : public ScriptObject<ScriptStockIcons>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "StockIcons")

		ScriptStockIcons(MonoObject* managedInstance);

	private:
		static MonoObject* InternalGetIcon(StockIcon icon, float size);
		static uint32_t InternalGetUnicode(StockIcon icon);
		static MonoObject* InternalGetFont(StockIcon icon);
		static StockIcon InternalParseIconName(MonoString* name);
	};
}
