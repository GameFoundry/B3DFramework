//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAnimationEvent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptAnimationEvent::ScriptAnimationEvent()
	{ }

	MonoObject* ScriptAnimationEvent::Box(const __AnimationEventInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__AnimationEventInterop ScriptAnimationEvent::Unbox(MonoObject* value)
	{
		return *(__AnimationEventInterop*)MonoUtil::Unbox(value);
	}

	AnimationEvent ScriptAnimationEvent::FromInterop(const __AnimationEventInterop& value)
	{
		AnimationEvent output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Time = value.Time;

		return output;
	}

	__AnimationEventInterop ScriptAnimationEvent::ToInterop(const AnimationEvent& value)
	{
		__AnimationEventInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Time = value.Time;

		return output;
	}

}
