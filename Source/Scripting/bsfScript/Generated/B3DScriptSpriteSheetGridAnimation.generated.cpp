//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteSheetGridAnimation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptSpriteSheetGridAnimation::ScriptSpriteSheetGridAnimation()
	{ }

	MonoObject* ScriptSpriteSheetGridAnimation::Box(const SpriteSheetGridAnimation& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	SpriteSheetGridAnimation ScriptSpriteSheetGridAnimation::Unbox(MonoObject* value)
	{
		return *(SpriteSheetGridAnimation*)MonoUtil::Unbox(value);
	}

}
