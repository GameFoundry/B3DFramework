//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIContentImages.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "BsScriptSpriteImage.generated.h"

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
		TResourceHandle<SpriteImage> tmpNormal;
		ScriptSpriteImageBase* scriptNormal;
		scriptNormal = (ScriptSpriteImageBase*)ScriptSpriteImage::ToNative(value.Normal);
		if(scriptNormal != nullptr)
			tmpNormal = B3DStaticResourceCast<SpriteImage>(scriptNormal->GetGenericHandle());
		output.Normal = tmpNormal;
		TResourceHandle<SpriteImage> tmpHover;
		ScriptSpriteImageBase* scriptHover;
		scriptHover = (ScriptSpriteImageBase*)ScriptSpriteImage::ToNative(value.Hover);
		if(scriptHover != nullptr)
			tmpHover = B3DStaticResourceCast<SpriteImage>(scriptHover->GetGenericHandle());
		output.Hover = tmpHover;
		TResourceHandle<SpriteImage> tmpActive;
		ScriptSpriteImageBase* scriptActive;
		scriptActive = (ScriptSpriteImageBase*)ScriptSpriteImage::ToNative(value.Active);
		if(scriptActive != nullptr)
			tmpActive = B3DStaticResourceCast<SpriteImage>(scriptActive->GetGenericHandle());
		output.Active = tmpActive;
		TResourceHandle<SpriteImage> tmpFocused;
		ScriptSpriteImageBase* scriptFocused;
		scriptFocused = (ScriptSpriteImageBase*)ScriptSpriteImage::ToNative(value.Focused);
		if(scriptFocused != nullptr)
			tmpFocused = B3DStaticResourceCast<SpriteImage>(scriptFocused->GetGenericHandle());
		output.Focused = tmpFocused;
		TResourceHandle<SpriteImage> tmpNormalOn;
		ScriptSpriteImageBase* scriptNormalOn;
		scriptNormalOn = (ScriptSpriteImageBase*)ScriptSpriteImage::ToNative(value.NormalOn);
		if(scriptNormalOn != nullptr)
			tmpNormalOn = B3DStaticResourceCast<SpriteImage>(scriptNormalOn->GetGenericHandle());
		output.NormalOn = tmpNormalOn;
		TResourceHandle<SpriteImage> tmpHoverOn;
		ScriptSpriteImageBase* scriptHoverOn;
		scriptHoverOn = (ScriptSpriteImageBase*)ScriptSpriteImage::ToNative(value.HoverOn);
		if(scriptHoverOn != nullptr)
			tmpHoverOn = B3DStaticResourceCast<SpriteImage>(scriptHoverOn->GetGenericHandle());
		output.HoverOn = tmpHoverOn;
		TResourceHandle<SpriteImage> tmpActiveOn;
		ScriptSpriteImageBase* scriptActiveOn;
		scriptActiveOn = (ScriptSpriteImageBase*)ScriptSpriteImage::ToNative(value.ActiveOn);
		if(scriptActiveOn != nullptr)
			tmpActiveOn = B3DStaticResourceCast<SpriteImage>(scriptActiveOn->GetGenericHandle());
		output.ActiveOn = tmpActiveOn;
		TResourceHandle<SpriteImage> tmpFocusedOn;
		ScriptSpriteImageBase* scriptFocusedOn;
		scriptFocusedOn = (ScriptSpriteImageBase*)ScriptSpriteImage::ToNative(value.FocusedOn);
		if(scriptFocusedOn != nullptr)
			tmpFocusedOn = B3DStaticResourceCast<SpriteImage>(scriptFocusedOn->GetGenericHandle());
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
