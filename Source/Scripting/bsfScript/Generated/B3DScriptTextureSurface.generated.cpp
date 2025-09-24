//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTextureSurface.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptTextureSurface::ScriptTextureSurface()
	{ }

	MonoObject* ScriptTextureSurface::Box(const TextureSurface& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TextureSurface ScriptTextureSurface::Unbox(MonoObject* value)
	{
		return *(TextureSurface*)MonoUtil::Unbox(value);
	}

}
