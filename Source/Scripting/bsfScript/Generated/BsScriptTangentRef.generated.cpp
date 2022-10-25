//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTangentRef.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"
#include "BsScriptKeyframeRef.generated.h"

namespace bs
{
	ScriptTangentRef::ScriptTangentRef(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptTangentRef::InitRuntimeData()
	{}

	MonoObject* ScriptTangentRef::Box(const __TangentRefInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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

} // namespace bs
