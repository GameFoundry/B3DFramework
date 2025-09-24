//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Core/Text/B3DStockIcons.h"
#include "B3DScriptTypeDefinition.h"
#include "../../../Foundation/Core/Text/B3DStockIcons.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptStockIcons : public TScriptTypeDefinition<ScriptStockIcons>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "StockIcons")

		ScriptStockIcons();

		static void SetupScriptBindings();

	private:
		static MonoObject* InternalGetIcon(StockIcon icon, float size);
		static uint32_t InternalGetUnicode(StockIcon icon);
		static MonoObject* InternalGetFont(StockIcon icon);
		static StockIcon InternalParseIconName(MonoString* name);
	};
}
