//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysicalPixel.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptPhysicalPixel::ScriptPhysicalPixel()
	{ }

	MonoObject* ScriptPhysicalPixel::Box(const PhysicalPixel& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	PhysicalPixel ScriptPhysicalPixel::Unbox(MonoObject* value)
	{
		return *(PhysicalPixel*)MonoUtil::Unbox(value);
	}

}
