//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptManagedMemberStyle.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptManagedMemberStyle::ScriptManagedMemberStyle()
	{ }

	MonoObject* ScriptManagedMemberStyle::Box(const __ManagedMemberStyleInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ManagedMemberStyleInterop ScriptManagedMemberStyle::Unbox(MonoObject* value)
	{
		return *(__ManagedMemberStyleInterop*)MonoUtil::Unbox(value);
	}

	ManagedMemberStyle ScriptManagedMemberStyle::FromInterop(const __ManagedMemberStyleInterop& value)
	{
		ManagedMemberStyle output;
		output.RangeMin = value.RangeMin;
		output.RangeMax = value.RangeMax;
		output.StepIncrement = value.StepIncrement;
		output.DisplayAsSlider = value.DisplayAsSlider;
		String tmpCategoryName;
		tmpCategoryName = MonoUtil::MonoToString(value.CategoryName);
		output.CategoryName = tmpCategoryName;
		output.Order = value.Order;

		return output;
	}

	__ManagedMemberStyleInterop ScriptManagedMemberStyle::ToInterop(const ManagedMemberStyle& value)
	{
		__ManagedMemberStyleInterop output;
		output.RangeMin = value.RangeMin;
		output.RangeMax = value.RangeMax;
		output.StepIncrement = value.StepIncrement;
		output.DisplayAsSlider = value.DisplayAsSlider;
		MonoString* tmpCategoryName;
		tmpCategoryName = MonoUtil::StringToMono(value.CategoryName);
		output.CategoryName = tmpCategoryName;
		output.Order = value.Order;

		return output;
	}

}
