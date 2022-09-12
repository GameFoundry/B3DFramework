//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	MonoObject*ScriptGUIElementStateStyle::box(const __GUIElementStateStyleInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__GUIElementStateStyleInterop ScriptGUIElementStateStyle::Unbox(MonoObject* value)
	{
		return *(__GUIElementStateStyleInterop*)MonoUtil::unbox(value);
	}

	GUIElementStateStyle ScriptGUIElementStateStyle::FromInterop(const __GUIElementStateStyleInterop& value)
	{
		GUIElementStateStyle output;
		ResourceHandle<SpriteTexture> tmptexture;
		ScriptSpriteTexture* scripttexture;
		scripttexture = ScriptSpriteTexture::toNative(value.texture);
		if(scripttexture != nullptr)
			tmptexture = scripttexture->GetHandle();
		output.texture = tmptexture;
		output.textColor = value.textColor;

		return output;
	}

	__GUIElementStateStyleInterop ScriptGUIElementStateStyle::ToInterop(const GUIElementStateStyle& value)
	{
		__GUIElementStateStyleInterop output;
		ScriptResourceBase* scripttexture;
		scripttexture = ScriptResourceManager::instance().GetScriptResource(value.texture, true);
		MonoObject* tmptexture;
		if(scripttexture != nullptr)
			tmptexture = scripttexture->GetManagedInstance();
		else
			tmptexture = nullptr;
		output.texture = tmptexture;
		output.textColor = value.textColor;

		return output;
	}

}
