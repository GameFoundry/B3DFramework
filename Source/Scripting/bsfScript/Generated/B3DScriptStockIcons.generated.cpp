//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptStockIcons.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Text/B3DStockIcons.h"
#include "B3DScriptSpriteImage.generated.h"
#include "B3DScriptFont.generated.h"

namespace b3d
{
	ScriptStockIcons::ScriptStockIcons()
		:TScriptTypeDefinition()
	{
	}

	void ScriptStockIcons::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIcon", (void*)&ScriptStockIcons::InternalGetIcon);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUnicode", (void*)&ScriptStockIcons::InternalGetUnicode);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFont", (void*)&ScriptStockIcons::InternalGetFont);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ParseIconName", (void*)&ScriptStockIcons::InternalParseIconName);

	}

	MonoObject* ScriptStockIcons::InternalGetIcon(StockIcon icon, float size)
	{
		TResourceHandle<SpriteImage> tmp__output;
		tmp__output = StockIcons::Instance().GetIcon(icon, size);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	uint32_t ScriptStockIcons::InternalGetUnicode(StockIcon icon)
	{
		uint32_t tmp__output;
		tmp__output = StockIcons::Instance().GetUnicode(icon);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptStockIcons::InternalGetFont(StockIcon icon)
	{
		TResourceHandle<Font> tmp__output;
		tmp__output = StockIcons::Instance().GetFont(icon);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptResourceWrapper::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	StockIcon ScriptStockIcons::InternalParseIconName(MonoString* name)
	{
		StockIcon tmp__output;
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		tmp__output = StockIcons::Instance().ParseIconName(tmpname);

		StockIcon __output;
		__output = tmp__output;

		return __output;
	}
}
