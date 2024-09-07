//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFontBitmapPage.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "BsScriptTexture.generated.h"

namespace bs
{
	ScriptFontBitmapPage::ScriptFontBitmapPage(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptFontBitmapPage::InitRuntimeData()
	{ }

	MonoObject*ScriptFontBitmapPage::Box(const __FontBitmapPageInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__FontBitmapPageInterop ScriptFontBitmapPage::Unbox(MonoObject* value)
	{
		return *(__FontBitmapPageInterop*)MonoUtil::Unbox(value);
	}

	FontBitmapPage ScriptFontBitmapPage::FromInterop(const __FontBitmapPageInterop& value)
	{
		FontBitmapPage output;
		TResourceHandle<Texture> tmpTexture;
		ScriptRRefBase* scriptWrapperObjectTexture;
		scriptWrapperObjectTexture = ScriptRRefBase::GetScriptObjectWrapper(value.Texture);
		if(scriptWrapperObjectTexture != nullptr)
			tmpTexture = B3DStaticResourceCast<Texture>(scriptWrapperObjectTexture->GetNativeObject());
		output.Texture = tmpTexture;
		output.IsDynamic = value.IsDynamic;

		return output;
	}

	__FontBitmapPageInterop ScriptFontBitmapPage::ToInterop(const FontBitmapPage& value)
	{
		__FontBitmapPageInterop output;
		MonoObject* tmpTexture;
		ScriptRRefBase* scriptWrapperObjectTexture;
		scriptWrapperObjectTexture = ScriptResourceManager::Instance().GetScriptRRef(value.Texture);
		if(scriptWrapperObjectTexture != nullptr)
			tmpTexture = scriptWrapperObjectTexture->GetScriptObject();
		else
			tmpTexture = nullptr;
		output.Texture = tmpTexture;
		output.IsDynamic = value.IsDynamic;

		return output;
	}

}
