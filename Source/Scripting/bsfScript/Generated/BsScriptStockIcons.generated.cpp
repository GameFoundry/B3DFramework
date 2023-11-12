//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptStockIcons.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Text/BsStockIcons.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"

namespace bs
{
	ScriptStockIcons::ScriptStockIcons(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{
	}

	void ScriptStockIcons::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetIcon", (void*)&ScriptStockIcons::InternalGetIcon);
		metaData.ScriptClass->AddInternalCall("Internal_GetUnicode", (void*)&ScriptStockIcons::InternalGetUnicode);
		metaData.ScriptClass->AddInternalCall("Internal_GetFont", (void*)&ScriptStockIcons::InternalGetFont);
		metaData.ScriptClass->AddInternalCall("Internal_ParseIconName", (void*)&ScriptStockIcons::InternalParseIconName);

	}

	MonoObject* ScriptStockIcons::InternalGetIcon(StockIcon icon, float size)
	{
		ResourceHandle<SpriteImage> tmp__output;
		tmp__output = StockIcons::Instance().GetIcon(icon, size);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

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
		ResourceHandle<Font> tmp__output;
		tmp__output = StockIcons::Instance().GetFont(icon);

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

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
