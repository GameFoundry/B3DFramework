//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptAnimationClipState : public ScriptObject<ScriptAnimationClipState>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AnimationClipState")

		static MonoObject* box(const AnimationClipState& value);
		static AnimationClipState unbox(MonoObject* value);

	private:
		ScriptAnimationClipState(MonoObject* managedInstance);

	};
}
