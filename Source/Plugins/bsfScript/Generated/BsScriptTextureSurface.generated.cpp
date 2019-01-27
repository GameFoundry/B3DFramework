//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTextureSurface.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTextureSurface::ScriptTextureSurface(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTextureSurface::initRuntimeData()
	{ }

	MonoObject*ScriptTextureSurface::box(const TextureSurface& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	TextureSurface ScriptTextureSurface::unbox(MonoObject* value)
	{
		return *(TextureSurface*)MonoUtil::unbox(value);
	}

}
