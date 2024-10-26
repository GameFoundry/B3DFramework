//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRect3.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"

namespace bs
{
	ScriptRect3::ScriptRect3()
	{ }

	MonoObject* ScriptRect3::Box(const __Rect3Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__Rect3Interop ScriptRect3::Unbox(MonoObject* value)
	{
		return *(__Rect3Interop*)MonoUtil::Unbox(value);
	}

	Rect3 ScriptRect3::FromInterop(const __Rect3Interop& value)
	{
		Rect3 output;
		output.Center = value.Center;
		output.HorizontalAxis = value.HorizontalAxis;
		output.VerticalAxis = value.VerticalAxis;
		output.HorizontalExtent = value.HorizontalExtent;
		output.VerticalExtent = value.VerticalExtent;

		return output;
	}

	__Rect3Interop ScriptRect3::ToInterop(const Rect3& value)
	{
		__Rect3Interop output;
		output.Center = value.Center;
		output.HorizontalAxis = value.HorizontalAxis;
		output.VerticalAxis = value.VerticalAxis;
		output.HorizontalExtent = value.HorizontalExtent;
		output.VerticalExtent = value.VerticalExtent;

		return output;
	}

}
