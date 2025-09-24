//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"

namespace b3d
{
	struct __AnimationEventInterop
	{
		MonoString* Name;
		float Time;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationEvent : public TScriptTypeDefinition<ScriptAnimationEvent>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AnimationEvent")

		static MonoObject* Box(const __AnimationEventInterop& value);
		static __AnimationEventInterop Unbox(MonoObject* value);
		static AnimationEvent FromInterop(const __AnimationEventInterop& value);
		static __AnimationEventInterop ToInterop(const AnimationEvent& value);

	private:
		ScriptAnimationEvent();

	};
}
