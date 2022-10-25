//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTextureSurface.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptTextureSurface::ScriptTextureSurface(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptTextureSurface::InitRuntimeData()
	{}

	MonoObject* ScriptTextureSurface::Box(const TextureSurface& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TextureSurface ScriptTextureSurface::Unbox(MonoObject* value)
	{
		return *(TextureSurface*)MonoUtil::Unbox(value);
	}

} // namespace bs
