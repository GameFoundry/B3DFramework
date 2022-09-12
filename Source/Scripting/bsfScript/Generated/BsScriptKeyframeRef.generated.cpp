//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptKeyframeRef.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptKeyframeRef::ScriptKeyframeRef(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptKeyframeRef::InitRuntimeData()
	{ }

	MonoObject*ScriptKeyframeRef::box(const KeyframeRef& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	KeyframeRef ScriptKeyframeRef::Unbox(MonoObject* value)
	{
		return *(KeyframeRef*)MonoUtil::unbox(value);
	}

}
