//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPixelVolume.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptPixelVolume::ScriptPixelVolume(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPixelVolume::initRuntimeData()
	{ }

	MonoObject*ScriptPixelVolume::box(const PixelVolume& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	PixelVolume ScriptPixelVolume::unbox(MonoObject* value)
	{
		return *(PixelVolume*)MonoUtil::unbox(value);
	}

}
