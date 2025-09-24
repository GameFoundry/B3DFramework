//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVirtualButton.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptVirtualButton::ScriptVirtualButton()
	{ }

	MonoObject* ScriptVirtualButton::Box(const VirtualButton& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	VirtualButton ScriptVirtualButton::Unbox(MonoObject* value)
	{
		return *(VirtualButton*)MonoUtil::Unbox(value);
	}

}
