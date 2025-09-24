//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIOption.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUIUnits.h"
#include "B3DScriptTUnitValue.generated.h"

namespace b3d
{
	ScriptGUIOption::ScriptGUIOption()
	{ }

	MonoObject* ScriptGUIOption::Box(const __GUIOptionInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__GUIOptionInterop ScriptGUIOption::Unbox(MonoObject* value)
	{
		return *(__GUIOptionInterop*)MonoUtil::Unbox(value);
	}

	GUIOption ScriptGUIOption::FromInterop(const __GUIOptionInterop& value)
	{
		GUIOption output;
		output.mMinimum = value.mMinimum;
		output.mMaximum = value.mMaximum;
		output.mType = value.mType;

		return output;
	}

	__GUIOptionInterop ScriptGUIOption::ToInterop(const GUIOption& value)
	{
		__GUIOptionInterop output;
		output.mMinimum = value.mMinimum;
		output.mMaximum = value.mMaximum;
		output.mType = value.mType;

		return output;
	}

}
