//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPixelVolume.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptPixelVolume::ScriptPixelVolume()
	{ }

	MonoObject* ScriptPixelVolume::Box(const PixelVolume& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	PixelVolume ScriptPixelVolume::Unbox(MonoObject* value)
	{
		return *(PixelVolume*)MonoUtil::Unbox(value);
	}

}
