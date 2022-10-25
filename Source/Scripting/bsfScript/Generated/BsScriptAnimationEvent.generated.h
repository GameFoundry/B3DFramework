//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"

namespace bs
{
	struct __AnimationEventInterop
	{
		MonoString* Name;
		float Time;
	};

	class BS_SCR_BE_EXPORT ScriptAnimationEvent : public ScriptObject<ScriptAnimationEvent>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AnimationEvent")

		static MonoObject* Box(const __AnimationEventInterop& value);
		static __AnimationEventInterop Unbox(MonoObject* value);
		static AnimationEvent FromInterop(const __AnimationEventInterop& value);
		static __AnimationEventInterop ToInterop(const AnimationEvent& value);

	private:
		ScriptAnimationEvent(MonoObject* managedInstance);
	};
} // namespace bs
