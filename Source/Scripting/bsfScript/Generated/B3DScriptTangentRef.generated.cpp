//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTangentRef.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Animation/B3DAnimationUtility.h"
#include "B3DScriptKeyframeRef.generated.h"

namespace b3d
{
	ScriptTangentRef::ScriptTangentRef()
	{ }

	MonoObject* ScriptTangentRef::Box(const __TangentRefInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TangentRefInterop ScriptTangentRef::Unbox(MonoObject* value)
	{
		return *(__TangentRefInterop*)MonoUtil::Unbox(value);
	}

	TangentRef ScriptTangentRef::FromInterop(const __TangentRefInterop& value)
	{
		TangentRef output;
		output.KeyframeRef = value.KeyframeRef;
		output.Type = value.Type;

		return output;
	}

	__TangentRefInterop ScriptTangentRef::ToInterop(const TangentRef& value)
	{
		__TangentRefInterop output;
		output.KeyframeRef = value.KeyframeRef;
		output.Type = value.Type;

		return output;
	}

}
