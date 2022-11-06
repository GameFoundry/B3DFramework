//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationClipState : public ScriptObject<ScriptAnimationClipState>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AnimationClipState")

		static MonoObject* Box(const AnimationClipState& value);
		static AnimationClipState Unbox(MonoObject* value);

	private:
		ScriptAnimationClipState(MonoObject* managedInstance);

	};
}
