//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIScrollAreaContent.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptGUIScrollAreaContent::ScriptGUIScrollAreaContent()
	{ }

	MonoObject* ScriptGUIScrollAreaContent::Box(const GUIScrollAreaContent& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	GUIScrollAreaContent ScriptGUIScrollAreaContent::Unbox(MonoObject* value)
	{
		return *(GUIScrollAreaContent*)MonoUtil::Unbox(value);
	}

}
