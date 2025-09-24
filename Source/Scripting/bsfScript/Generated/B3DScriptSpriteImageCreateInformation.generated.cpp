//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSpriteImageCreateInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Image/B3DSpriteImage.h"
#include "B3DScriptSpriteSheetGridAnimation.generated.h"

namespace b3d
{
	ScriptSpriteImageCreateInformation::ScriptSpriteImageCreateInformation()
	{ }

	MonoObject* ScriptSpriteImageCreateInformation::Box(const __SpriteImageCreateInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__SpriteImageCreateInformationInterop ScriptSpriteImageCreateInformation::Unbox(MonoObject* value)
	{
		return *(__SpriteImageCreateInformationInterop*)MonoUtil::Unbox(value);
	}

	SpriteImageCreateInformation ScriptSpriteImageCreateInformation::FromInterop(const __SpriteImageCreateInformationInterop& value)
	{
		SpriteImageCreateInformation output;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

	__SpriteImageCreateInformationInterop ScriptSpriteImageCreateInformation::ToInterop(const SpriteImageCreateInformation& value)
	{
		__SpriteImageCreateInformationInterop output;
		output.AnimationPlayback = value.AnimationPlayback;
		output.Animation = value.Animation;

		return output;
	}

}
