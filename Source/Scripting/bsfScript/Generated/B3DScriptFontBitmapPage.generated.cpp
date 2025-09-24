//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptFontBitmapPage.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "BsScriptTexture.generated.h"

namespace b3d
{
	ScriptFontBitmapPage::ScriptFontBitmapPage()
	{ }

	MonoObject* ScriptFontBitmapPage::Box(const __FontBitmapPageInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__FontBitmapPageInterop ScriptFontBitmapPage::Unbox(MonoObject* value)
	{
		return *(__FontBitmapPageInterop*)MonoUtil::Unbox(value);
	}

	FontBitmapPage ScriptFontBitmapPage::FromInterop(const __FontBitmapPageInterop& value)
	{
		FontBitmapPage output;
		TResourceHandle<Texture> tmpTexture;
		ScriptRRefBase* scriptObjectWrapperTexture;
		scriptObjectWrapperTexture = ScriptRRefBase::GetScriptObjectWrapper(value.Texture);
		if(scriptObjectWrapperTexture != nullptr)
			tmpTexture = B3DStaticResourceCast<Texture>(scriptObjectWrapperTexture->GetNativeObject());
		output.Texture = tmpTexture;
		output.Type = value.Type;

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
		output.Type = value.Type;

		return output;
	}

}
