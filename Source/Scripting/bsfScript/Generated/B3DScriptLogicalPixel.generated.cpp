//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLogicalPixel.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptLogicalPixel::ScriptLogicalPixel()
	{ }

	MonoObject* ScriptLogicalPixel::Box(const LogicalPixel& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	LogicalPixel ScriptLogicalPixel::Unbox(MonoObject* value)
	{
		return *(LogicalPixel*)MonoUtil::Unbox(value);
	}

}
