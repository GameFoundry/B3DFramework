//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptPointerEvent.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"
#include "B3DScriptTVector2.generated.h"

namespace b3d
{
	ScriptPointerEvent::ScriptPointerEvent()
	{ }

	MonoObject* ScriptPointerEvent::Box(const __PointerEventInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__PointerEventInterop ScriptPointerEvent::Unbox(MonoObject* value)
	{
		return *(__PointerEventInterop*)MonoUtil::Unbox(value);
	}

	PointerEvent ScriptPointerEvent::FromInterop(const __PointerEventInterop& value)
	{
		PointerEvent output;
		output.ScreenPos = value.ScreenPos;
		output.Delta = value.Delta;
		output.Button = value.Button;
		output.Type = value.Type;
		output.Shift = value.Shift;
		output.Control = value.Control;
		output.Alt = value.Alt;
		output.MouseWheelScrollAmount = value.MouseWheelScrollAmount;
		output.IsUsed = value.IsUsed;

		return output;
	}

	__PointerEventInterop ScriptPointerEvent::ToInterop(const PointerEvent& value)
	{
		__PointerEventInterop output;
		output.ScreenPos = value.ScreenPos;
		output.Delta = value.Delta;
		output.Button = value.Button;
		output.Type = value.Type;
		output.Shift = value.Shift;
		output.Control = value.Control;
		output.Alt = value.Alt;
		output.MouseWheelScrollAmount = value.MouseWheelScrollAmount;
		output.IsUsed = value.IsUsed;

		return output;
	}

}
