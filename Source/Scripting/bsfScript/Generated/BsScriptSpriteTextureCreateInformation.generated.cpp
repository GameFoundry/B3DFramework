//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteTextureCreateInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "BsScriptTexture.generated.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "BsScriptSpriteSheetGridAnimation.generated.h"

namespace bs
{
	ScriptSpriteTextureCreateInformation::ScriptSpriteTextureCreateInformation(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSpriteTextureCreateInformation::InitRuntimeData()
	{ }

	MonoObject*ScriptSpriteTextureCreateInformation::Box(const __SpriteTextureCreateInformationInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__SpriteTextureCreateInformationInterop ScriptSpriteTextureCreateInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteTextureCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteTextureCreateInformation ScriptSpriteTextureCreateInformation::FromInterop(const __SpriteTextureCreateInformationInterop& value)
	{
		SpriteTextureCreateInformation output;
		TResourceHandle<Texture> tmpAtlasTexture;
		ScriptRRefBase* scriptAtlasTexture;
		scriptAtlasTexture = ScriptRRefBase::ToNative(value.AtlasTexture);
		if(scriptAtlasTexture != nullptr)
			tmpAtlasTexture = B3DStaticResourceCast<Texture>(scriptAtlasTexture->GetHandle());
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
		ScriptRRefBase* scriptAtlasTexture;
		scriptAtlasTexture = ScriptResourceManager::Instance().GetScriptRRef(value.AtlasTexture);
		if(scriptAtlasTexture != nullptr)
			tmpAtlasTexture = scriptAtlasTexture->GetManagedInstance();
		else
			tmpAtlasTexture = nullptr;
		output.AtlasTexture = tmpAtlasTexture;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
