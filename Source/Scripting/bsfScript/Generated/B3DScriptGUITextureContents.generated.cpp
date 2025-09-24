//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUITextureContents.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptSpriteImage.generated.h"

namespace b3d
{
	ScriptGUITextureContents::ScriptGUITextureContents()
	{ }

	MonoObject* ScriptGUITextureContents::Box(const __GUITextureContentsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__GUITextureContentsInterop ScriptGUITextureContents::Unbox(MonoObject* value)
	{
		return *(__GUITextureContentsInterop*)MonoUtil::Unbox(value);
	}

	GUITextureContents ScriptGUITextureContents::FromInterop(const __GUITextureContentsInterop& value)
	{
		GUITextureContents output;
		TResourceHandle<SpriteImage> tmpImage;
		ScriptSpriteImageWrapperBase* scriptObjectWrapperImage;
		scriptObjectWrapperImage = (ScriptSpriteImageWrapperBase*)ScriptSpriteImage::GetScriptObjectWrapper(value.Image);
		if(scriptObjectWrapperImage != nullptr)
			tmpImage = B3DStaticResourceCast<SpriteImage>(scriptObjectWrapperImage->GetBaseNativeObjectAsHandle());
		output.Image = tmpImage;
		output.ScaleMode = value.ScaleMode;
		output.IsTransparent = value.IsTransparent;

		return output;
	}

	__GUITextureContentsInterop ScriptGUITextureContents::ToInterop(const GUITextureContents& value)
	{
		__GUITextureContentsInterop output;
		MonoObject* tmpImage;
		MonoObject* temptmpImage = nullptr;
		if(value.Image)
		temptmpImage = ScriptResourceWrapper::GetOrCreateScriptObject(value.Image);
		tmpImage = temptmpImage;
		output.Image = tmpImage;
		output.ScaleMode = value.ScaleMode;
		output.IsTransparent = value.IsTransparent;

		return output;
	}

}
