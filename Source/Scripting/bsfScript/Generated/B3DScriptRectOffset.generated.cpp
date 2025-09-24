//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRectOffset.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptRectOffset::ScriptRectOffset()
	{ }

	MonoObject* ScriptRectOffset::Box(const RectOffset& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	RectOffset ScriptRectOffset::Unbox(MonoObject* value)
	{
		return *(RectOffset*)MonoUtil::Unbox(value);
	}

}
