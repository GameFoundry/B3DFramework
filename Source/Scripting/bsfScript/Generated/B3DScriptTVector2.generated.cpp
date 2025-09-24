//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTVector2.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "BsScriptTUnitValue.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "BsScriptTUnitValue.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "BsScriptTUnitValue.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIUnits.h"
#include "BsScriptTUnitValue.generated.h"

namespace b3d
{
	ScriptTVector2_float_::ScriptTVector2_float_()
	{ }

	MonoObject* ScriptTVector2_float_::Box(const TVector2<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector2<float> ScriptTVector2_float_::Unbox(MonoObject* value)
	{
		return *(TVector2<float>*)MonoUtil::Unbox(value);
	}


	ScriptTVector2_double_::ScriptTVector2_double_()
	{ }

	MonoObject* ScriptTVector2_double_::Box(const TVector2<double>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector2<double> ScriptTVector2_double_::Unbox(MonoObject* value)
	{
		return *(TVector2<double>*)MonoUtil::Unbox(value);
	}


	ScriptTVector2_int32_t_::ScriptTVector2_int32_t_()
	{ }

	MonoObject* ScriptTVector2_int32_t_::Box(const TVector2<int32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector2<int32_t> ScriptTVector2_int32_t_::Unbox(MonoObject* value)
	{
		return *(TVector2<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptTVector2_uint32_t_::ScriptTVector2_uint32_t_()
	{ }

	MonoObject* ScriptTVector2_uint32_t_::Box(const TVector2<uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector2<uint32_t> ScriptTVector2_uint32_t_::Unbox(MonoObject* value)
	{
		return *(TVector2<uint32_t>*)MonoUtil::Unbox(value);
	}


	ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ScriptTVector2_TUnitValue_int32_t__LogicalPixel__()
	{ }

	MonoObject* ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::Box(const __TVector2_TUnitValue_int32_t__LogicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TVector2_TUnitValue_int32_t__LogicalPixel__Interop ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TVector2_TUnitValue_int32_t__LogicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TVector2<TUnitValue<int32_t, LogicalPixel>> ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::FromInterop(const __TVector2_TUnitValue_int32_t__LogicalPixel__Interop& value)
	{
		TVector2<TUnitValue<int32_t, LogicalPixel>> output;
		output.X = value.X;
		output.Y = value.Y;

		return output;
	}

	__TVector2_TUnitValue_int32_t__LogicalPixel__Interop ScriptTVector2_TUnitValue_int32_t__LogicalPixel__::ToInterop(const TVector2<TUnitValue<int32_t, LogicalPixel>>& value)
	{
		__TVector2_TUnitValue_int32_t__LogicalPixel__Interop output;
		output.X = value.X;
		output.Y = value.Y;

		return output;
	}


	ScriptTVector2_TUnitValue_float__LogicalPixel__::ScriptTVector2_TUnitValue_float__LogicalPixel__()
	{ }

	MonoObject* ScriptTVector2_TUnitValue_float__LogicalPixel__::Box(const __TVector2_TUnitValue_float__LogicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TVector2_TUnitValue_float__LogicalPixel__Interop ScriptTVector2_TUnitValue_float__LogicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TVector2_TUnitValue_float__LogicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TVector2<TUnitValue<float, LogicalPixel>> ScriptTVector2_TUnitValue_float__LogicalPixel__::FromInterop(const __TVector2_TUnitValue_float__LogicalPixel__Interop& value)
	{
		TVector2<TUnitValue<float, LogicalPixel>> output;
		output.X = value.X;
		output.Y = value.Y;

		return output;
	}

	__TVector2_TUnitValue_float__LogicalPixel__Interop ScriptTVector2_TUnitValue_float__LogicalPixel__::ToInterop(const TVector2<TUnitValue<float, LogicalPixel>>& value)
	{
		__TVector2_TUnitValue_float__LogicalPixel__Interop output;
		output.X = value.X;
		output.Y = value.Y;

		return output;
	}


	ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__()
	{ }

	MonoObject* ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::Box(const __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TVector2<TUnitValue<int32_t, PhysicalPixel>> ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::FromInterop(const __TVector2_TUnitValue_int32_t__PhysicalPixel__Interop& value)
	{
		TVector2<TUnitValue<int32_t, PhysicalPixel>> output;
		output.X = value.X;
		output.Y = value.Y;

		return output;
	}

	__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop ScriptTVector2_TUnitValue_int32_t__PhysicalPixel__::ToInterop(const TVector2<TUnitValue<int32_t, PhysicalPixel>>& value)
	{
		__TVector2_TUnitValue_int32_t__PhysicalPixel__Interop output;
		output.X = value.X;
		output.Y = value.Y;

		return output;
	}


	ScriptTVector2_TUnitValue_float__PhysicalPixel__::ScriptTVector2_TUnitValue_float__PhysicalPixel__()
	{ }

	MonoObject* ScriptTVector2_TUnitValue_float__PhysicalPixel__::Box(const __TVector2_TUnitValue_float__PhysicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TVector2_TUnitValue_float__PhysicalPixel__Interop ScriptTVector2_TUnitValue_float__PhysicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TVector2_TUnitValue_float__PhysicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TVector2<TUnitValue<float, PhysicalPixel>> ScriptTVector2_TUnitValue_float__PhysicalPixel__::FromInterop(const __TVector2_TUnitValue_float__PhysicalPixel__Interop& value)
	{
		TVector2<TUnitValue<float, PhysicalPixel>> output;
		output.X = value.X;
		output.Y = value.Y;

		return output;
	}

	__TVector2_TUnitValue_float__PhysicalPixel__Interop ScriptTVector2_TUnitValue_float__PhysicalPixel__::ToInterop(const TVector2<TUnitValue<float, PhysicalPixel>>& value)
	{
		__TVector2_TUnitValue_float__PhysicalPixel__Interop output;
		output.X = value.X;
		output.Y = value.Y;

		return output;
	}

}
