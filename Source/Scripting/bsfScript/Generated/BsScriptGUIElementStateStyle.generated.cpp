//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIElementStateStyle.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "BsScriptSpriteTexture.generated.h"
#include "Image/BsColor.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	ScriptGUIElementStateStyle::ScriptGUIElementStateStyle(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptGUIElementStateStyle::InitRuntimeData()
	{ }

	MonoObject*ScriptGUIElementStateStyle::Box(const __GUIElementStateStyleInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__GUIElementStateStyleInterop ScriptGUIElementStateStyle::Unbox(MonoObject* value)
	{
		return *(__GUIElementStateStyleInterop*)MonoUtil::Unbox(value);
	}

	GUIElementStateStyle ScriptGUIElementStateStyle::FromInterop(const __GUIElementStateStyleInterop& value)
	{
		GUIElementStateStyle output;
		ResourceHandle<SpriteTexture> tmpTexture;
		ScriptSpriteTexture* scriptTexture;
		scriptTexture = ScriptSpriteTexture::ToNative(value.Texture);
		if(scriptTexture != nullptr)
			tmpTexture = scriptTexture->GetHandle();
		output.Texture = tmpTexture;
		output.TextColor = value.TextColor;

		return output;
	}

	__GUIElementStateStyleInterop ScriptGUIElementStateStyle::ToInterop(const GUIElementStateStyle& value)
	{
		__GUIElementStateStyleInterop output;
		MonoObject* tmpTexture;
		ScriptResourceBase* scriptTexture;
		scriptTexture = ScriptResourceManager::Instance().GetScriptResource(value.Texture, true);
		if(scriptTexture != nullptr)
			tmpTexture = scriptTexture->GetManagedInstance();
		else
			tmpTexture = nullptr;
		output.Texture = tmpTexture;
		output.TextColor = value.TextColor;

		return output;
	}

}
