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

	MonoObject*ScriptGUIContentImages::Box(const __GUIContentImagesInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__GUIContentImagesInterop ScriptGUIContentImages::Unbox(MonoObject* value)
	{
		return *(__GUIContentImagesInterop*)MonoUtil::Unbox(value);
	}

	GUIContentImages ScriptGUIContentImages::FromInterop(const __GUIContentImagesInterop& value)
	{
		GUIContentImages output;
		ResourceHandle<SpriteTexture> tmpNormal;
		ScriptSpriteTexture* scriptNormal;
		scriptNormal = ScriptSpriteTexture::ToNative(value.Normal);
		if(scriptNormal != nullptr)
			tmpNormal = scriptNormal->GetHandle();
		output.Normal = tmpNormal;
		ResourceHandle<SpriteTexture> tmpHover;
		ScriptSpriteTexture* scriptHover;
		scriptHover = ScriptSpriteTexture::ToNative(value.Hover);
		if(scriptHover != nullptr)
			tmpHover = scriptHover->GetHandle();
		output.Hover = tmpHover;
		ResourceHandle<SpriteTexture> tmpActive;
		ScriptSpriteTexture* scriptActive;
		scriptActive = ScriptSpriteTexture::ToNative(value.Active);
		if(scriptActive != nullptr)
			tmpActive = scriptActive->GetHandle();
		output.Active = tmpActive;
		ResourceHandle<SpriteTexture> tmpFocused;
		ScriptSpriteTexture* scriptFocused;
		scriptFocused = ScriptSpriteTexture::ToNative(value.Focused);
		if(scriptFocused != nullptr)
			tmpFocused = scriptFocused->GetHandle();
		output.Focused = tmpFocused;
		ResourceHandle<SpriteTexture> tmpNormalOn;
		ScriptSpriteTexture* scriptNormalOn;
		scriptNormalOn = ScriptSpriteTexture::ToNative(value.NormalOn);
		if(scriptNormalOn != nullptr)
			tmpNormalOn = scriptNormalOn->GetHandle();
		output.NormalOn = tmpNormalOn;
		ResourceHandle<SpriteTexture> tmpHoverOn;
		ScriptSpriteTexture* scriptHoverOn;
		scriptHoverOn = ScriptSpriteTexture::ToNative(value.HoverOn);
		if(scriptHoverOn != nullptr)
			tmpHoverOn = scriptHoverOn->GetHandle();
		output.HoverOn = tmpHoverOn;
		ResourceHandle<SpriteTexture> tmpActiveOn;
		ScriptSpriteTexture* scriptActiveOn;
		scriptActiveOn = ScriptSpriteTexture::ToNative(value.ActiveOn);
		if(scriptActiveOn != nullptr)
			tmpActiveOn = scriptActiveOn->GetHandle();
		output.ActiveOn = tmpActiveOn;
		ResourceHandle<SpriteTexture> tmpFocusedOn;
		ScriptSpriteTexture* scriptFocusedOn;
		scriptFocusedOn = ScriptSpriteTexture::ToNative(value.FocusedOn);
		if(scriptFocusedOn != nullptr)
			tmpFocusedOn = scriptFocusedOn->GetHandle();
		output.FocusedOn = tmpFocusedOn;

		return output;
	}

	__GUIContentImagesInterop ScriptGUIContentImages::ToInterop(const GUIContentImages& value)
	{
		__GUIContentImagesInterop output;
		MonoObject* tmpNormal;
		ScriptResourceBase* scriptNormal;
		scriptNormal = ScriptResourceManager::Instance().GetScriptResource(value.Normal, true);
		if(scriptNormal != nullptr)
			tmpNormal = scriptNormal->GetManagedInstance();
		else
			tmpNormal = nullptr;
		output.Normal = tmpNormal;
		MonoObject* tmpHover;
		ScriptResourceBase* scriptHover;
		scriptHover = ScriptResourceManager::Instance().GetScriptResource(value.Hover, true);
		if(scriptHover != nullptr)
			tmpHover = scriptHover->GetManagedInstance();
		else
			tmpHover = nullptr;
		output.Hover = tmpHover;
		MonoObject* tmpActive;
		ScriptResourceBase* scriptActive;
		scriptActive = ScriptResourceManager::Instance().GetScriptResource(value.Active, true);
		if(scriptActive != nullptr)
			tmpActive = scriptActive->GetManagedInstance();
		else
			tmpActive = nullptr;
		output.Active = tmpActive;
		MonoObject* tmpFocused;
		ScriptResourceBase* scriptFocused;
		scriptFocused = ScriptResourceManager::Instance().GetScriptResource(value.Focused, true);
		if(scriptFocused != nullptr)
			tmpFocused = scriptFocused->GetManagedInstance();
		else
			tmpFocused = nullptr;
		output.Focused = tmpFocused;
		MonoObject* tmpNormalOn;
		ScriptResourceBase* scriptNormalOn;
		scriptNormalOn = ScriptResourceManager::Instance().GetScriptResource(value.NormalOn, true);
		if(scriptNormalOn != nullptr)
			tmpNormalOn = scriptNormalOn->GetManagedInstance();
		else
			tmpNormalOn = nullptr;
		output.NormalOn = tmpNormalOn;
		MonoObject* tmpHoverOn;
		ScriptResourceBase* scriptHoverOn;
		scriptHoverOn = ScriptResourceManager::Instance().GetScriptResource(value.HoverOn, true);
		if(scriptHoverOn != nullptr)
			tmpHoverOn = scriptHoverOn->GetManagedInstance();
		else
			tmpHoverOn = nullptr;
		output.HoverOn = tmpHoverOn;
		MonoObject* tmpActiveOn;
		ScriptResourceBase* scriptActiveOn;
		scriptActiveOn = ScriptResourceManager::Instance().GetScriptResource(value.ActiveOn, true);
		if(scriptActiveOn != nullptr)
			tmpActiveOn = scriptActiveOn->GetManagedInstance();
		else
			tmpActiveOn = nullptr;
		output.ActiveOn = tmpActiveOn;
		MonoObject* tmpFocusedOn;
		ScriptResourceBase* scriptFocusedOn;
		scriptFocusedOn = ScriptResourceManager::Instance().GetScriptResource(value.FocusedOn, true);
		if(scriptFocusedOn != nullptr)
			tmpFocusedOn = scriptFocusedOn->GetManagedInstance();
		else
			tmpFocusedOn = nullptr;
		output.FocusedOn = tmpFocusedOn;

		return output;
	}

}
