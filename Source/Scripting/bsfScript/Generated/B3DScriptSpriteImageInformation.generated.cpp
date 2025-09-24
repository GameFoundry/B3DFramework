//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSpriteImageInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptSpriteSheetGridAnimation.generated.h"

namespace b3d
{
	ScriptSpriteImageInformation::ScriptSpriteImageInformation()
	{ }

	MonoObject* ScriptSpriteImageInformation::Box(const __SpriteImageInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__SpriteImageInformationInterop ScriptSpriteImageInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteImageInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteImageInformation ScriptSpriteImageInformation::FromInterop(const __SpriteImageInformationInterop& value)
	{
		SpriteImageInformation output;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

	__SpriteImageInformationInterop ScriptSpriteImageInformation::ToInterop(const SpriteImageInformation& value)
	{
		__SpriteImageInformationInterop output;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
