//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTangentRef.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"
#include "BsScriptKeyframeRef.generated.h"

namespace bs
{
	ScriptTangentRef::ScriptTangentRef(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptTangentRef::initRuntimeData()
	{ }

	MonoObject*ScriptTangentRef::box(const __TangentRefInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__TangentRefInterop ScriptTangentRef::unbox(MonoObject* value)
	{
		return *(__TangentRefInterop*)MonoUtil::unbox(value);
	}

	TangentRef ScriptTangentRef::fromInterop(const __TangentRefInterop& value)
	{
		TangentRef output;
		output.keyframeRef = value.keyframeRef;
		output.type = value.type;

		return output;
	}

	__TangentRefInterop ScriptTangentRef::toInterop(const TangentRef& value)
	{
		__TangentRefInterop output;
		output.keyframeRef = value.keyframeRef;
		output.type = value.type;

		return output;
	}

}
