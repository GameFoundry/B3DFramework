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

	void ScriptTextureSurface::InitRuntimeData()
	{ }

	MonoObject*ScriptTextureSurface::box(const TextureSurface& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	TextureSurface ScriptTextureSurface::Unbox(MonoObject* value)
	{
		return *(TextureSurface*)MonoUtil::unbox(value);
	}

}
