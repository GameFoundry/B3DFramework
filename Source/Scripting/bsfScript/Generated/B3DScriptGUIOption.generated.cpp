//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIOption.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "BsScriptTUnitValue.generated.h"

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
