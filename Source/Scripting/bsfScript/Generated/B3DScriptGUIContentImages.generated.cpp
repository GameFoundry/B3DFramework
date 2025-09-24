//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIContentImages.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptSpriteImage.generated.h"

namespace b3d
{
	ScriptGUIContentImages::ScriptGUIContentImages()
	{ }

	MonoObject* ScriptGUIContentImages::Box(const __GUIContentImagesInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__GUIContentImagesInterop ScriptGUIContentImages::Unbox(MonoObject* value)
	{
		return *(__GUIContentImagesInterop*)MonoUtil::Unbox(value);
	}

	GUIContentImages ScriptGUIContentImages::FromInterop(const __GUIContentImagesInterop& value)
	{
		GUIContentImages output;
		TResourceHandle<SpriteImage> tmpNormal;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperNormal;
		scriptObjectWrapperNormal = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.Normal);
		if(scriptObjectWrapperNormal != nullptr)
			tmpNormal = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperNormal->GetBaseNativeObjectAsHandle());
		output.Normal = tmpNormal;
		TResourceHandle<SpriteImage> tmpHover;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperHover;
		scriptObjectWrapperHover = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.Hover);
		if(scriptObjectWrapperHover != nullptr)
			tmpHover = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperHover->GetBaseNativeObjectAsHandle());
		output.Hover = tmpHover;
		TResourceHandle<SpriteImage> tmpActive;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperActive;
		scriptObjectWrapperActive = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.Active);
		if(scriptObjectWrapperActive != nullptr)
			tmpActive = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperActive->GetBaseNativeObjectAsHandle());
		output.Active = tmpActive;
		TResourceHandle<SpriteImage> tmpFocused;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperFocused;
		scriptObjectWrapperFocused = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.Focused);
		if(scriptObjectWrapperFocused != nullptr)
			tmpFocused = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperFocused->GetBaseNativeObjectAsHandle());
		output.Focused = tmpFocused;
		TResourceHandle<SpriteImage> tmpNormalOn;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperNormalOn;
		scriptObjectWrapperNormalOn = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.NormalOn);
		if(scriptObjectWrapperNormalOn != nullptr)
			tmpNormalOn = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperNormalOn->GetBaseNativeObjectAsHandle());
		output.NormalOn = tmpNormalOn;
		TResourceHandle<SpriteImage> tmpHoverOn;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperHoverOn;
		scriptObjectWrapperHoverOn = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.HoverOn);
		if(scriptObjectWrapperHoverOn != nullptr)
			tmpHoverOn = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperHoverOn->GetBaseNativeObjectAsHandle());
		output.HoverOn = tmpHoverOn;
		TResourceHandle<SpriteImage> tmpActiveOn;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperActiveOn;
		scriptObjectWrapperActiveOn = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.ActiveOn);
		if(scriptObjectWrapperActiveOn != nullptr)
			tmpActiveOn = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperActiveOn->GetBaseNativeObjectAsHandle());
		output.ActiveOn = tmpActiveOn;
		TResourceHandle<SpriteImage> tmpFocusedOn;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperFocusedOn;
		scriptObjectWrapperFocusedOn = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.FocusedOn);
		if(scriptObjectWrapperFocusedOn != nullptr)
			tmpFocusedOn = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperFocusedOn->GetBaseNativeObjectAsHandle());
		output.FocusedOn = tmpFocusedOn;

		return output;
	}

	__GUIContentImagesInterop ScriptGUIContentImages::ToInterop(const GUIContentImages& value)
	{
		__GUIContentImagesInterop output;
		MonoObject* tmpNormal;
		MonoObject* temptmpNormal = nullptr;
		if(value.Normal)
		temptmpNormal = ScriptResourceWrapper::GetOrCreateScriptObject(value.Normal);
		tmpNormal = temptmpNormal;
		output.Normal = tmpNormal;
		MonoObject* tmpHover;
		MonoObject* temptmpHover = nullptr;
		if(value.Hover)
		temptmpHover = ScriptResourceWrapper::GetOrCreateScriptObject(value.Hover);
		tmpHover = temptmpHover;
		output.Hover = tmpHover;
		MonoObject* tmpActive;
		MonoObject* temptmpActive = nullptr;
		if(value.Active)
		temptmpActive = ScriptResourceWrapper::GetOrCreateScriptObject(value.Active);
		tmpActive = temptmpActive;
		output.Active = tmpActive;
		MonoObject* tmpFocused;
		MonoObject* temptmpFocused = nullptr;
		if(value.Focused)
		temptmpFocused = ScriptResourceWrapper::GetOrCreateScriptObject(value.Focused);
		tmpFocused = temptmpFocused;
		output.Focused = tmpFocused;
		MonoObject* tmpNormalOn;
		MonoObject* temptmpNormalOn = nullptr;
		if(value.NormalOn)
		temptmpNormalOn = ScriptResourceWrapper::GetOrCreateScriptObject(value.NormalOn);
		tmpNormalOn = temptmpNormalOn;
		output.NormalOn = tmpNormalOn;
		MonoObject* tmpHoverOn;
		MonoObject* temptmpHoverOn = nullptr;
		if(value.HoverOn)
		temptmpHoverOn = ScriptResourceWrapper::GetOrCreateScriptObject(value.HoverOn);
		tmpHoverOn = temptmpHoverOn;
		output.HoverOn = tmpHoverOn;
		MonoObject* tmpActiveOn;
		MonoObject* temptmpActiveOn = nullptr;
		if(value.ActiveOn)
		temptmpActiveOn = ScriptResourceWrapper::GetOrCreateScriptObject(value.ActiveOn);
		tmpActiveOn = temptmpActiveOn;
		output.ActiveOn = tmpActiveOn;
		MonoObject* tmpFocusedOn;
		MonoObject* temptmpFocusedOn = nullptr;
		if(value.FocusedOn)
		temptmpFocusedOn = ScriptResourceWrapper::GetOrCreateScriptObject(value.FocusedOn);
		tmpFocusedOn = temptmpFocusedOn;
		output.FocusedOn = tmpFocusedOn;

		return output;
	}

}
