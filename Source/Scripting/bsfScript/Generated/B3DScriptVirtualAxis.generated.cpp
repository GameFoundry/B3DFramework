//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptVirtualAxis.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptVirtualAxis::ScriptVirtualAxis()
	{ }

	MonoObject* ScriptVirtualAxis::Box(const VirtualAxis& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	VirtualAxis ScriptVirtualAxis::Unbox(MonoObject* value)
	{
		return *(VirtualAxis*)MonoUtil::Unbox(value);
	}

}
