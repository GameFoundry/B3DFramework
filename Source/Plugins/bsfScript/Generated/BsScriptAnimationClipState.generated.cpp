//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAnimationClipState.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptAnimationClipState::ScriptAnimationClipState(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptAnimationClipState::initRuntimeData()
	{ }

	MonoObject*ScriptAnimationClipState::box(const AnimationClipState& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	AnimationClipState ScriptAnimationClipState::unbox(MonoObject* value)
	{
		return *(AnimationClipState*)MonoUtil::unbox(value);
	}

}
