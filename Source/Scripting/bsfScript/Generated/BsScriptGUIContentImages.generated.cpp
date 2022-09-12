//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIContentImages.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "BsScriptSpriteTexture.generated.h"

namespace bs
{
	ScriptGUIContentImages::ScriptGUIContentImages(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptGUIContentImages::InitRuntimeData()
	{ }

	MonoObject*ScriptGUIContentImages::box(const __GUIContentImagesInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__GUIContentImagesInterop ScriptGUIContentImages::Unbox(MonoObject* value)
	{
		return *(__GUIContentImagesInterop*)MonoUtil::unbox(value);
	}

	GUIContentImages ScriptGUIContentImages::FromInterop(const __GUIContentImagesInterop& value)
	{
		GUIContentImages output;
		ResourceHandle<SpriteTexture> tmpnormal;
		ScriptSpriteTexture* scriptnormal;
		scriptnormal = ScriptSpriteTexture::toNative(value.normal);
		if(scriptnormal != nullptr)
			tmpnormal = scriptnormal->GetHandle();
		output.normal = tmpnormal;
		ResourceHandle<SpriteTexture> tmphover;
		ScriptSpriteTexture* scripthover;
		scripthover = ScriptSpriteTexture::toNative(value.hover);
		if(scripthover != nullptr)
			tmphover = scripthover->GetHandle();
		output.hover = tmphover;
		ResourceHandle<SpriteTexture> tmpactive;
		ScriptSpriteTexture* scriptactive;
		scriptactive = ScriptSpriteTexture::toNative(value.active);
		if(scriptactive != nullptr)
			tmpactive = scriptactive->GetHandle();
		output.active = tmpactive;
		ResourceHandle<SpriteTexture> tmpfocused;
		ScriptSpriteTexture* scriptfocused;
		scriptfocused = ScriptSpriteTexture::toNative(value.focused);
		if(scriptfocused != nullptr)
			tmpfocused = scriptfocused->GetHandle();
		output.focused = tmpfocused;
		ResourceHandle<SpriteTexture> tmpnormalOn;
		ScriptSpriteTexture* scriptnormalOn;
		scriptnormalOn = ScriptSpriteTexture::toNative(value.normalOn);
		if(scriptnormalOn != nullptr)
			tmpnormalOn = scriptnormalOn->GetHandle();
		output.normalOn = tmpnormalOn;
		ResourceHandle<SpriteTexture> tmphoverOn;
		ScriptSpriteTexture* scripthoverOn;
		scripthoverOn = ScriptSpriteTexture::toNative(value.hoverOn);
		if(scripthoverOn != nullptr)
			tmphoverOn = scripthoverOn->GetHandle();
		output.hoverOn = tmphoverOn;
		ResourceHandle<SpriteTexture> tmpactiveOn;
		ScriptSpriteTexture* scriptactiveOn;
		scriptactiveOn = ScriptSpriteTexture::toNative(value.activeOn);
		if(scriptactiveOn != nullptr)
			tmpactiveOn = scriptactiveOn->GetHandle();
		output.activeOn = tmpactiveOn;
		ResourceHandle<SpriteTexture> tmpfocusedOn;
		ScriptSpriteTexture* scriptfocusedOn;
		scriptfocusedOn = ScriptSpriteTexture::toNative(value.focusedOn);
		if(scriptfocusedOn != nullptr)
			tmpfocusedOn = scriptfocusedOn->GetHandle();
		output.focusedOn = tmpfocusedOn;

		return output;
	}

	__GUIContentImagesInterop ScriptGUIContentImages::ToInterop(const GUIContentImages& value)
	{
		__GUIContentImagesInterop output;
		ScriptResourceBase* scriptnormal;
		scriptnormal = ScriptResourceManager::instance().GetScriptResource(value.normal, true);
		MonoObject* tmpnormal;
		if(scriptnormal != nullptr)
			tmpnormal = scriptnormal->GetManagedInstance();
		else
			tmpnormal = nullptr;
		output.normal = tmpnormal;
		ScriptResourceBase* scripthover;
		scripthover = ScriptResourceManager::instance().GetScriptResource(value.hover, true);
		MonoObject* tmphover;
		if(scripthover != nullptr)
			tmphover = scripthover->GetManagedInstance();
		else
			tmphover = nullptr;
		output.hover = tmphover;
		ScriptResourceBase* scriptactive;
		scriptactive = ScriptResourceManager::instance().GetScriptResource(value.active, true);
		MonoObject* tmpactive;
		if(scriptactive != nullptr)
			tmpactive = scriptactive->GetManagedInstance();
		else
			tmpactive = nullptr;
		output.active = tmpactive;
		ScriptResourceBase* scriptfocused;
		scriptfocused = ScriptResourceManager::instance().GetScriptResource(value.focused, true);
		MonoObject* tmpfocused;
		if(scriptfocused != nullptr)
			tmpfocused = scriptfocused->GetManagedInstance();
		else
			tmpfocused = nullptr;
		output.focused = tmpfocused;
		ScriptResourceBase* scriptnormalOn;
		scriptnormalOn = ScriptResourceManager::instance().GetScriptResource(value.normalOn, true);
		MonoObject* tmpnormalOn;
		if(scriptnormalOn != nullptr)
			tmpnormalOn = scriptnormalOn->GetManagedInstance();
		else
			tmpnormalOn = nullptr;
		output.normalOn = tmpnormalOn;
		ScriptResourceBase* scripthoverOn;
		scripthoverOn = ScriptResourceManager::instance().GetScriptResource(value.hoverOn, true);
		MonoObject* tmphoverOn;
		if(scripthoverOn != nullptr)
			tmphoverOn = scripthoverOn->GetManagedInstance();
		else
			tmphoverOn = nullptr;
		output.hoverOn = tmphoverOn;
		ScriptResourceBase* scriptactiveOn;
		scriptactiveOn = ScriptResourceManager::instance().GetScriptResource(value.activeOn, true);
		MonoObject* tmpactiveOn;
		if(scriptactiveOn != nullptr)
			tmpactiveOn = scriptactiveOn->GetManagedInstance();
		else
			tmpactiveOn = nullptr;
		output.activeOn = tmpactiveOn;
		ScriptResourceBase* scriptfocusedOn;
		scriptfocusedOn = ScriptResourceManager::instance().GetScriptResource(value.focusedOn, true);
		MonoObject* tmpfocusedOn;
		if(scriptfocusedOn != nullptr)
			tmpfocusedOn = scriptfocusedOn->GetManagedInstance();
		else
			tmpfocusedOn = nullptr;
		output.focusedOn = tmpfocusedOn;

		return output;
	}

}
