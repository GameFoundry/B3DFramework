//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIPanelContent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptGUIPanelContent::ScriptGUIPanelContent()
	{ }

	MonoObject* ScriptGUIPanelContent::Box(const GUIPanelContent& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	GUIPanelContent ScriptGUIPanelContent::Unbox(MonoObject* value)
	{
		return *(GUIPanelContent*)MonoUtil::Unbox(value);
	}

}
