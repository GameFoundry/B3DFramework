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

	void ScriptGUIElementStateStyle::initRuntimeData()
	{ }

	MonoObject*ScriptGUIElementStateStyle::box(const __GUIElementStateStyleInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__GUIElementStateStyleInterop ScriptGUIElementStateStyle::unbox(MonoObject* value)
	{
		return *(__GUIElementStateStyleInterop*)MonoUtil::unbox(value);
	}

	GUIElementStateStyle ScriptGUIElementStateStyle::fromInterop(const __GUIElementStateStyleInterop& value)
	{
		GUIElementStateStyle output;
		ResourceHandle<SpriteTexture> tmptexture;
		ScriptSpriteTexture* scripttexture;
		scripttexture = ScriptSpriteTexture::toNative(value.texture);
		if(scripttexture != nullptr)
			tmptexture = scripttexture->getHandle();
		output.texture = tmptexture;
		output.textColor = value.textColor;

		return output;
	}

	__GUIElementStateStyleInterop ScriptGUIElementStateStyle::toInterop(const GUIElementStateStyle& value)
	{
		__GUIElementStateStyleInterop output;
		ScriptResourceBase* scripttexture;
		scripttexture = ScriptResourceManager::instance().getScriptResource(value.texture, true);
		MonoObject* tmptexture;
		if(scripttexture != nullptr)
			tmptexture = scripttexture->getManagedInstance();
		else
			tmptexture = nullptr;
		output.texture = tmptexture;
		output.textColor = value.textColor;

		return output;
	}

}
