//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVideoMode.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptVideoMode::ScriptVideoMode(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptVideoMode::initRuntimeData()
	{ }

	MonoObject*ScriptVideoMode::box(const VideoMode& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	VideoMode ScriptVideoMode::unbox(MonoObject* value)
	{
		return *(VideoMode*)MonoUtil::unbox(value);
	}

#endif
}
