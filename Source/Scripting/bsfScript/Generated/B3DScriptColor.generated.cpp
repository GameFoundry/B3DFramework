//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptColor.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptColor::ScriptColor()
	{ }

	MonoObject* ScriptColor::Box(const Color& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	Color ScriptColor::Unbox(MonoObject* value)
	{
		return *(Color*)MonoUtil::Unbox(value);
	}

}
