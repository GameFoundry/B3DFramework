//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAnimationEvent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptAnimationEvent::ScriptAnimationEvent(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptAnimationEvent::initRuntimeData()
	{ }

	MonoObject*ScriptAnimationEvent::box(const __AnimationEventInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__AnimationEventInterop ScriptAnimationEvent::unbox(MonoObject* value)
	{
		return *(__AnimationEventInterop*)MonoUtil::unbox(value);
	}

	AnimationEvent ScriptAnimationEvent::fromInterop(const __AnimationEventInterop& value)
	{
		AnimationEvent output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.time = value.time;

		return output;
	}

	__AnimationEventInterop ScriptAnimationEvent::toInterop(const AnimationEvent& value)
	{
		__AnimationEventInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.time = value.time;

		return output;
	}

}
