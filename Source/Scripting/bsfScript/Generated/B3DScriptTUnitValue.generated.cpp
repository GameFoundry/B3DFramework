//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTUnitValue.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptTUnitValue_int32_t__LogicalPixel_::ScriptTUnitValue_int32_t__LogicalPixel_()
	{ }

	MonoObject* ScriptTUnitValue_int32_t__LogicalPixel_::Box(const TUnitValue<int32_t, LogicalPixel>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TUnitValue<int32_t, LogicalPixel> ScriptTUnitValue_int32_t__LogicalPixel_::Unbox(MonoObject* value)
	{
		return *(TUnitValue<int32_t, LogicalPixel>*)MonoUtil::Unbox(value);
	}


	ScriptTUnitValue_float__LogicalPixel_::ScriptTUnitValue_float__LogicalPixel_()
	{ }

	MonoObject* ScriptTUnitValue_float__LogicalPixel_::Box(const TUnitValue<float, LogicalPixel>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TUnitValue<float, LogicalPixel> ScriptTUnitValue_float__LogicalPixel_::Unbox(MonoObject* value)
	{
		return *(TUnitValue<float, LogicalPixel>*)MonoUtil::Unbox(value);
	}


	ScriptTUnitValue_int32_t__PhysicalPixel_::ScriptTUnitValue_int32_t__PhysicalPixel_()
	{ }

	MonoObject* ScriptTUnitValue_int32_t__PhysicalPixel_::Box(const TUnitValue<int32_t, PhysicalPixel>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TUnitValue<int32_t, PhysicalPixel> ScriptTUnitValue_int32_t__PhysicalPixel_::Unbox(MonoObject* value)
	{
		return *(TUnitValue<int32_t, PhysicalPixel>*)MonoUtil::Unbox(value);
	}


	ScriptTUnitValue_float__PhysicalPixel_::ScriptTUnitValue_float__PhysicalPixel_()
	{ }

	MonoObject* ScriptTUnitValue_float__PhysicalPixel_::Box(const TUnitValue<float, PhysicalPixel>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TUnitValue<float, PhysicalPixel> ScriptTUnitValue_float__PhysicalPixel_::Unbox(MonoObject* value)
	{
		return *(TUnitValue<float, PhysicalPixel>*)MonoUtil::Unbox(value);
	}

}
