//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIContentImages.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "../../../../../Foundation/bsfCore/Image/BsSpriteTexture.h"
#include "BsScriptSpriteTexture.generated.h"

namespace bs
{
	ScriptGUIContentImages::ScriptGUIContentImages(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptGUIContentImages::initRuntimeData()
	{ }

	MonoObject*ScriptGUIContentImages::box(const __GUIContentImagesInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__GUIContentImagesInterop ScriptGUIContentImages::unbox(MonoObject* value)
	{
		return *(__GUIContentImagesInterop*)MonoUtil::unbox(value);
	}

	GUIContentImages ScriptGUIContentImages::fromInterop(const __GUIContentImagesInterop& value)
	{
		GUIContentImages output;
		ResourceHandle<SpriteTexture> tmpnormal;
		ScriptSpriteTexture* scriptnormal;
		scriptnormal = ScriptSpriteTexture::toNative(value.normal);
		if(scriptnormal != nullptr)
			tmpnormal = scriptnormal->getHandle();
		output.normal = tmpnormal;
		ResourceHandle<SpriteTexture> tmphover;
		ScriptSpriteTexture* scripthover;
		scripthover = ScriptSpriteTexture::toNative(value.hover);
		if(scripthover != nullptr)
			tmphover = scripthover->getHandle();
		output.hover = tmphover;
		ResourceHandle<SpriteTexture> tmpactive;
		ScriptSpriteTexture* scriptactive;
		scriptactive = ScriptSpriteTexture::toNative(value.active);
		if(scriptactive != nullptr)
			tmpactive = scriptactive->getHandle();
		output.active = tmpactive;
		ResourceHandle<SpriteTexture> tmpfocused;
		ScriptSpriteTexture* scriptfocused;
		scriptfocused = ScriptSpriteTexture::toNative(value.focused);
		if(scriptfocused != nullptr)
			tmpfocused = scriptfocused->getHandle();
		output.focused = tmpfocused;
		ResourceHandle<SpriteTexture> tmpnormalOn;
		ScriptSpriteTexture* scriptnormalOn;
		scriptnormalOn = ScriptSpriteTexture::toNative(value.normalOn);
		if(scriptnormalOn != nullptr)
			tmpnormalOn = scriptnormalOn->getHandle();
		output.normalOn = tmpnormalOn;
		ResourceHandle<SpriteTexture> tmphoverOn;
		ScriptSpriteTexture* scripthoverOn;
		scripthoverOn = ScriptSpriteTexture::toNative(value.hoverOn);
		if(scripthoverOn != nullptr)
			tmphoverOn = scripthoverOn->getHandle();
		output.hoverOn = tmphoverOn;
		ResourceHandle<SpriteTexture> tmpactiveOn;
		ScriptSpriteTexture* scriptactiveOn;
		scriptactiveOn = ScriptSpriteTexture::toNative(value.activeOn);
		if(scriptactiveOn != nullptr)
			tmpactiveOn = scriptactiveOn->getHandle();
		output.activeOn = tmpactiveOn;
		ResourceHandle<SpriteTexture> tmpfocusedOn;
		ScriptSpriteTexture* scriptfocusedOn;
		scriptfocusedOn = ScriptSpriteTexture::toNative(value.focusedOn);
		if(scriptfocusedOn != nullptr)
			tmpfocusedOn = scriptfocusedOn->getHandle();
		output.focusedOn = tmpfocusedOn;

		return output;
	}

	__GUIContentImagesInterop ScriptGUIContentImages::toInterop(const GUIContentImages& value)
	{
		__GUIContentImagesInterop output;
		ScriptResourceBase* scriptnormal;
		scriptnormal = ScriptResourceManager::instance().getScriptResource(value.normal, true);
		MonoObject* tmpnormal;
		if(scriptnormal != nullptr)
			tmpnormal = scriptnormal->getManagedInstance();
		else
			tmpnormal = nullptr;
		output.normal = tmpnormal;
		ScriptResourceBase* scripthover;
		scripthover = ScriptResourceManager::instance().getScriptResource(value.hover, true);
		MonoObject* tmphover;
		if(scripthover != nullptr)
			tmphover = scripthover->getManagedInstance();
		else
			tmphover = nullptr;
		output.hover = tmphover;
		ScriptResourceBase* scriptactive;
		scriptactive = ScriptResourceManager::instance().getScriptResource(value.active, true);
		MonoObject* tmpactive;
		if(scriptactive != nullptr)
			tmpactive = scriptactive->getManagedInstance();
		else
			tmpactive = nullptr;
		output.active = tmpactive;
		ScriptResourceBase* scriptfocused;
		scriptfocused = ScriptResourceManager::instance().getScriptResource(value.focused, true);
		MonoObject* tmpfocused;
		if(scriptfocused != nullptr)
			tmpfocused = scriptfocused->getManagedInstance();
		else
			tmpfocused = nullptr;
		output.focused = tmpfocused;
		ScriptResourceBase* scriptnormalOn;
		scriptnormalOn = ScriptResourceManager::instance().getScriptResource(value.normalOn, true);
		MonoObject* tmpnormalOn;
		if(scriptnormalOn != nullptr)
			tmpnormalOn = scriptnormalOn->getManagedInstance();
		else
			tmpnormalOn = nullptr;
		output.normalOn = tmpnormalOn;
		ScriptResourceBase* scripthoverOn;
		scripthoverOn = ScriptResourceManager::instance().getScriptResource(value.hoverOn, true);
		MonoObject* tmphoverOn;
		if(scripthoverOn != nullptr)
			tmphoverOn = scripthoverOn->getManagedInstance();
		else
			tmphoverOn = nullptr;
		output.hoverOn = tmphoverOn;
		ScriptResourceBase* scriptactiveOn;
		scriptactiveOn = ScriptResourceManager::instance().getScriptResource(value.activeOn, true);
		MonoObject* tmpactiveOn;
		if(scriptactiveOn != nullptr)
			tmpactiveOn = scriptactiveOn->getManagedInstance();
		else
			tmpactiveOn = nullptr;
		output.activeOn = tmpactiveOn;
		ScriptResourceBase* scriptfocusedOn;
		scriptfocusedOn = ScriptResourceManager::instance().getScriptResource(value.focusedOn, true);
		MonoObject* tmpfocusedOn;
		if(scriptfocusedOn != nullptr)
			tmpfocusedOn = scriptfocusedOn->getManagedInstance();
		else
			tmpfocusedOn = nullptr;
		output.focusedOn = tmpfocusedOn;

		return output;
	}

}
