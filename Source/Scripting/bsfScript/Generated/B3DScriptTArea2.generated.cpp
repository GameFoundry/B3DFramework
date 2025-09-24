//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTArea2.generated.h"
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
	ScriptTArea2_int32_t__int32_t_::ScriptTArea2_int32_t__int32_t_()
	{ }

	MonoObject* ScriptTArea2_int32_t__int32_t_::Box(const TArea2<int32_t, int32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TArea2<int32_t, int32_t> ScriptTArea2_int32_t__int32_t_::Unbox(MonoObject* value)
	{
		return *(TArea2<int32_t, int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptTArea2_int32_t__uint32_t_::ScriptTArea2_int32_t__uint32_t_()
	{ }

	MonoObject* ScriptTArea2_int32_t__uint32_t_::Box(const TArea2<int32_t, uint32_t>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TArea2<int32_t, uint32_t> ScriptTArea2_int32_t__uint32_t_::Unbox(MonoObject* value)
	{
		return *(TArea2<int32_t, uint32_t>*)MonoUtil::Unbox(value);
	}


	ScriptTArea2_float__float_::ScriptTArea2_float__float_()
	{ }

	MonoObject* ScriptTArea2_float__float_::Box(const TArea2<float, float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TArea2<float, float> ScriptTArea2_float__float_::Unbox(MonoObject* value)
	{
		return *(TArea2<float, float>*)MonoUtil::Unbox(value);
	}


	ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__()
	{ }

	MonoObject* ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::Box(const __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TArea2<TUnitValue<int32_t, LogicalPixel>, TUnitValue<int32_t, LogicalPixel>> ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::FromInterop(const __TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop& value)
	{
		TArea2<TUnitValue<int32_t, LogicalPixel>, TUnitValue<int32_t, LogicalPixel>> output;
		output.X = value.X;
		output.Y = value.Y;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

	__TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop ScriptTArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__::ToInterop(const TArea2<TUnitValue<int32_t, LogicalPixel>, TUnitValue<int32_t, LogicalPixel>>& value)
	{
		__TArea2_TUnitValue_int32_t__LogicalPixel___TUnitValue_int32_t__LogicalPixel__Interop output;
		output.X = value.X;
		output.Y = value.Y;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}


	ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__::ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__()
	{ }

	MonoObject* ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__::Box(const __TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TArea2<TUnitValue<float, LogicalPixel>, TUnitValue<float, LogicalPixel>> ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__::FromInterop(const __TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop& value)
	{
		TArea2<TUnitValue<float, LogicalPixel>, TUnitValue<float, LogicalPixel>> output;
		output.X = value.X;
		output.Y = value.Y;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

	__TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop ScriptTArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__::ToInterop(const TArea2<TUnitValue<float, LogicalPixel>, TUnitValue<float, LogicalPixel>>& value)
	{
		__TArea2_TUnitValue_float__LogicalPixel___TUnitValue_float__LogicalPixel__Interop output;
		output.X = value.X;
		output.Y = value.Y;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}


	ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__()
	{ }

	MonoObject* ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::Box(const __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::FromInterop(const __TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop& value)
	{
		TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>> output;
		output.X = value.X;
		output.Y = value.Y;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

	__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop ScriptTArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__::ToInterop(const TArea2<TUnitValue<int32_t, PhysicalPixel>, TUnitValue<int32_t, PhysicalPixel>>& value)
	{
		__TArea2_TUnitValue_int32_t__PhysicalPixel___TUnitValue_int32_t__PhysicalPixel__Interop output;
		output.X = value.X;
		output.Y = value.Y;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}


	ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__::ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__()
	{ }

	MonoObject* ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__::Box(const __TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__::Unbox(MonoObject* value)
	{
		return *(__TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop*)MonoUtil::Unbox(value);
	}

	TArea2<TUnitValue<float, PhysicalPixel>, TUnitValue<float, PhysicalPixel>> ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__::FromInterop(const __TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop& value)
	{
		TArea2<TUnitValue<float, PhysicalPixel>, TUnitValue<float, PhysicalPixel>> output;
		output.X = value.X;
		output.Y = value.Y;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

	__TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop ScriptTArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__::ToInterop(const TArea2<TUnitValue<float, PhysicalPixel>, TUnitValue<float, PhysicalPixel>>& value)
	{
		__TArea2_TUnitValue_float__PhysicalPixel___TUnitValue_float__PhysicalPixel__Interop output;
		output.X = value.X;
		output.Y = value.Y;
		output.Width = value.Width;
		output.Height = value.Height;

		return output;
	}

}
