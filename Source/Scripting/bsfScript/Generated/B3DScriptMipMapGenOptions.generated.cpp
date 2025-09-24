//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMipMapGenOptions.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptMipMapGenOptions::ScriptMipMapGenOptions()
	{ }

	MonoObject* ScriptMipMapGenOptions::Box(const MipMapGenOptions& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	MipMapGenOptions ScriptMipMapGenOptions::Unbox(MonoObject* value)
	{
		return *(MipMapGenOptions*)MonoUtil::Unbox(value);
	}

}
