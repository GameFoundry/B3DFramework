//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteVectorPathCreateInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/VectorGraphics/BsVectorGraphics.h"
#include "BsScriptVectorPath.generated.h"
#include "Utility/BsUtil.h"
#include "Wrappers/BsScriptSize.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "BsScriptSpriteSheetGridAnimation.generated.h"

namespace bs
{
	ScriptSpriteVectorPathCreateInformation::ScriptSpriteVectorPathCreateInformation(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSpriteVectorPathCreateInformation::InitRuntimeData()
	{ }

	MonoObject*ScriptSpriteVectorPathCreateInformation::Box(const __SpriteVectorPathCreateInformationInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__SpriteVectorPathCreateInformationInterop ScriptSpriteVectorPathCreateInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteVectorPathCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteVectorPathCreateInformation ScriptSpriteVectorPathCreateInformation::FromInterop(const __SpriteVectorPathCreateInformationInterop& value)
	{
		SpriteVectorPathCreateInformation output;
		ResourceHandle<VectorPath> tmpVectorPath;
		ScriptRRefBase* scriptVectorPath;
		scriptVectorPath = ScriptRRefBase::ToNative(value.VectorPath);
		if(scriptVectorPath != nullptr)
			tmpVectorPath = B3DStaticResourceCast<VectorPath>(scriptVectorPath->GetHandle());
		output.VectorPath = tmpVectorPath;
		output.Size = value.Size;
		output.ScalingMode = value.ScalingMode;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

	__SpriteVectorPathCreateInformationInterop ScriptSpriteVectorPathCreateInformation::ToInterop(const SpriteVectorPathCreateInformation& value)
	{
		__SpriteVectorPathCreateInformationInterop output;
		MonoObject* tmpVectorPath;
		ScriptRRefBase* scriptVectorPath;
		scriptVectorPath = ScriptResourceManager::Instance().GetScriptRRef(value.VectorPath);
		if(scriptVectorPath != nullptr)
			tmpVectorPath = scriptVectorPath->GetManagedInstance();
		else
			tmpVectorPath = nullptr;
		output.VectorPath = tmpVectorPath;
		output.Size = value.Size;
		output.ScalingMode = value.ScalingMode;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
