//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteImageInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Image/BsSpriteImage.h"
#include "BsScriptSpriteSheetGridAnimation.generated.h"

namespace bs
{
	ScriptSpriteImageInformation::ScriptSpriteImageInformation(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSpriteImageInformation::InitRuntimeData()
	{ }

	MonoObject*ScriptSpriteImageInformation::Box(const __SpriteImageInformationInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__SpriteImageInformationInterop ScriptSpriteImageInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteImageInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteImageInformation ScriptSpriteImageInformation::FromInterop(const __SpriteImageInformationInterop& value)
	{
		SpriteImageInformation output;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

	__SpriteImageInformationInterop ScriptSpriteImageInformation::ToInterop(const SpriteImageInformation& value)
	{
		__SpriteImageInformationInterop output;
		output.UVRange = value.UVRange;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
