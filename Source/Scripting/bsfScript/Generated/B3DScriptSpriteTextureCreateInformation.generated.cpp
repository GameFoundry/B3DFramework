//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteTextureCreateInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "BsScriptTexture.generated.h"
#include "../../../Foundation/bsfUtility/Math/BsArea2.h"
#include "BsScriptTArea2.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "BsScriptSpriteSheetGridAnimation.generated.h"

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
