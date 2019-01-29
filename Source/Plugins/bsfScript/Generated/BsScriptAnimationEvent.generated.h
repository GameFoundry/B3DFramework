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
		MonoString* name;
		float time;
	};

	class BS_SCR_BE_EXPORT ScriptAnimationEvent : public ScriptObject<ScriptAnimationEvent>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AnimationEvent")

		static MonoObject* box(const __AnimationEventInterop& value);
		static __AnimationEventInterop unbox(MonoObject* value);
		static AnimationEvent fromInterop(const __AnimationEventInterop& value);
		static __AnimationEventInterop toInterop(const AnimationEvent& value);

	private:
		ScriptAnimationEvent(MonoObject* managedInstance);

	};
}
