//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSpriteTextureCreateInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Image/B3DTexture.h"
#include "B3DScriptTexture.generated.h"
#include "../../../Foundation/Utility/Math/B3DArea2.h"
#include "B3DScriptTArea2.generated.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptSpriteSheetGridAnimation.generated.h"

namespace b3d
{
	ScriptSpriteTextureCreateInformation::ScriptSpriteTextureCreateInformation()
	{ }

	MonoObject* ScriptSpriteTextureCreateInformation::Box(const __SpriteTextureCreateInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__SpriteTextureCreateInformationInterop ScriptSpriteTextureCreateInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteTextureCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteTextureCreateInformation ScriptSpriteTextureCreateInformation::FromInterop(const __SpriteTextureCreateInformationInterop& value)
	{
		SpriteTextureCreateInformation output;
		TResourceHandle<Texture> tmpAtlasTexture;
		ScriptRRefBase* scriptObjectWrapperAtlasTexture;
		scriptObjectWrapperAtlasTexture = ScriptRRefBase::GetScriptObjectWrapper(value.AtlasTexture);
		if(scriptObjectWrapperAtlasTexture != nullptr)
			tmpAtlasTexture = B3DStaticResourceCast<Texture>(scriptObjectWrapperAtlasTexture->GetNativeObject());
		output.AtlasTexture = tmpAtlasTexture;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

	__SpriteTextureCreateInformationInterop ScriptSpriteTextureCreateInformation::ToInterop(const SpriteTextureCreateInformation& value)
	{
		__SpriteTextureCreateInformationInterop output;
		MonoObject* tmpAtlasTexture;
		ScriptRRefBase* scriptWrapperObjectAtlasTexture;
		scriptWrapperObjectAtlasTexture = ScriptResourceManager::Instance().GetScriptRRef(value.AtlasTexture);
		if(scriptWrapperObjectAtlasTexture != nullptr)
			tmpAtlasTexture = scriptWrapperObjectAtlasTexture->GetScriptObject();
		else
			tmpAtlasTexture = nullptr;
		output.AtlasTexture = tmpAtlasTexture;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
