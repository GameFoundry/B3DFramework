//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpriteSheetGridAnimation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSpriteSheetGridAnimation::ScriptSpriteSheetGridAnimation(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptSpriteSheetGridAnimation::InitRuntimeData()
	{}

	MonoObject* ScriptSpriteSheetGridAnimation::Box(const SpriteSheetGridAnimation& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	SpriteSheetGridAnimation ScriptSpriteSheetGridAnimation::Unbox(MonoObject* value)
	{
		return *(SpriteSheetGridAnimation*)MonoUtil::Unbox(value);
	}

} // namespace bs
