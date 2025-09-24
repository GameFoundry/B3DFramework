//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DAnimation.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationClipState : public TScriptTypeDefinition<ScriptAnimationClipState>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AnimationClipState")

		static MonoObject* Box(const AnimationClipState& value);
		static AnimationClipState Unbox(MonoObject* value);

	private:
		ScriptAnimationClipState();

	};
}
