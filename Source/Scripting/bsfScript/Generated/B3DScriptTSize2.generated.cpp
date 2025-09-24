//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTSize2.generated.h"
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
	ScriptTSize2_float_::ScriptTSize2_float_()
	{ }

	MonoObject* ScriptTSize2_float_::Box(const TSize2<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TSize2<float> ScriptTSize2_float_::Unbox(MonoObject* value)
	{
		return *(TSize2<float>*)MonoUtil::Unbox(value);
	}


	ScriptTSize2_double_::ScriptTSize2_double_()
	{ }

	MonoObject* ScriptTSize2_double_::Box(const TSize2<double>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TSize2<double> ScriptTSize2_double_::Unbox(MonoObject* value)
	{
		return *(TSize2<double>*)MonoUtil::Unbox(value);
	}


	ScriptTSize2_uint32_t_::ScriptTSize2_uint32_t_()
	{ }

	MonoObject* ScriptTSize2_uint32_t_::Box(const TSize2<uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TSize2<uint32_t> ScriptTSize2_uint32_t_::Unbox(MonoObject* value)
	{
		return *(TSize2<uint32_t>*)MonoUtil::Unbox(value);
	}


	ScriptTSize2_int32_t_::ScriptTSize2_int32_t_()
	{ }

	MonoObject* ScriptTSize2_int32_t_::Box(const TSize2<int32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TSize2<int32_t> ScriptTSize2_int32_t_::Unbox(MonoObject* value)
	{
		return *(TSize2<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::ScriptTSize2_TUnitValue_int32_t__LogicalPixel__()
	{ }

	MonoObject* ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::Box(const __TSize2_TUnitValue_int32_t__LogicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TSize2_TUnitValue_int32_t__LogicalPixel__Interop ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TSize2_TUnitValue_int32_t__LogicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TSize2<TUnitValue<int32_t, LogicalPixel>> ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::FromInterop(const __TSize2_TUnitValue_int32_t__LogicalPixel__Interop& value)
	{
		TSize2<TUnitValue<int32_t, LogicalPixel>> output;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

	__TSize2_TUnitValue_int32_t__LogicalPixel__Interop ScriptTSize2_TUnitValue_int32_t__LogicalPixel__::ToInterop(const TSize2<TUnitValue<int32_t, LogicalPixel>>& value)
	{
		__TSize2_TUnitValue_int32_t__LogicalPixel__Interop output;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}


	ScriptTSize2_TUnitValue_float__LogicalPixel__::ScriptTSize2_TUnitValue_float__LogicalPixel__()
	{ }

	MonoObject* ScriptTSize2_TUnitValue_float__LogicalPixel__::Box(const __TSize2_TUnitValue_float__LogicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TSize2_TUnitValue_float__LogicalPixel__Interop ScriptTSize2_TUnitValue_float__LogicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TSize2_TUnitValue_float__LogicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TSize2<TUnitValue<float, LogicalPixel>> ScriptTSize2_TUnitValue_float__LogicalPixel__::FromInterop(const __TSize2_TUnitValue_float__LogicalPixel__Interop& value)
	{
		TSize2<TUnitValue<float, LogicalPixel>> output;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

	__TSize2_TUnitValue_float__LogicalPixel__Interop ScriptTSize2_TUnitValue_float__LogicalPixel__::ToInterop(const TSize2<TUnitValue<float, LogicalPixel>>& value)
	{
		__TSize2_TUnitValue_float__LogicalPixel__Interop output;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}


	ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__::ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__()
	{ }

	MonoObject* ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__::Box(const __TSize2_TUnitValue_int32_t__PhysicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TSize2_TUnitValue_int32_t__PhysicalPixel__Interop ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TSize2_TUnitValue_int32_t__PhysicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TSize2<TUnitValue<int32_t, PhysicalPixel>> ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__::FromInterop(const __TSize2_TUnitValue_int32_t__PhysicalPixel__Interop& value)
	{
		TSize2<TUnitValue<int32_t, PhysicalPixel>> output;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

	__TSize2_TUnitValue_int32_t__PhysicalPixel__Interop ScriptTSize2_TUnitValue_int32_t__PhysicalPixel__::ToInterop(const TSize2<TUnitValue<int32_t, PhysicalPixel>>& value)
	{
		__TSize2_TUnitValue_int32_t__PhysicalPixel__Interop output;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}


	ScriptTSize2_TUnitValue_float__PhysicalPixel__::ScriptTSize2_TUnitValue_float__PhysicalPixel__()
	{ }

	MonoObject* ScriptTSize2_TUnitValue_float__PhysicalPixel__::Box(const __TSize2_TUnitValue_float__PhysicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TSize2_TUnitValue_float__PhysicalPixel__Interop ScriptTSize2_TUnitValue_float__PhysicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TSize2_TUnitValue_float__PhysicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TSize2<TUnitValue<float, PhysicalPixel>> ScriptTSize2_TUnitValue_float__PhysicalPixel__::FromInterop(const __TSize2_TUnitValue_float__PhysicalPixel__Interop& value)
	{
		TSize2<TUnitValue<float, PhysicalPixel>> output;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

	__TSize2_TUnitValue_float__PhysicalPixel__Interop ScriptTSize2_TUnitValue_float__PhysicalPixel__::ToInterop(const TSize2<TUnitValue<float, PhysicalPixel>>& value)
	{
		__TSize2_TUnitValue_float__PhysicalPixel__Interop output;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

}
