//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRootMotion.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../Extensions/BsAnimationEx.h"
#include "BsScriptTAnimationCurve.generated.h"

namespace bs
{
	ScriptRootMotion::ScriptRootMotion(MonoObject* managedInstance, const SPtr<RootMotion>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptRootMotion::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetPositionCurves", (void*)&ScriptRootMotion::InternalGetPositionCurves);
		metaData.ScriptClass->AddInternalCall("Internal_GetRotationCurves", (void*)&ScriptRootMotion::InternalGetRotationCurves);

	}

	MonoObject* ScriptRootMotion::Create(const SPtr<RootMotion>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptRootMotion>()) ScriptRootMotion(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptRootMotion::InternalGetPositionCurves(ScriptRootMotion* thisPtr)
	{
		SPtr<TAnimationCurve<Vector3>> tmp__output = B3DMakeShared<TAnimationCurve<Vector3>>();
		*tmp__output = RootMotionEx::GetPositionCurves(thisPtr->GetInternal());

		MonoObject* __output;
		__output = ScriptTAnimationCurveVector3::Create(tmp__output);

		return __output;
	}

	MonoObject* ScriptRootMotion::InternalGetRotationCurves(ScriptRootMotion* thisPtr)
	{
		SPtr<TAnimationCurve<Quaternion>> tmp__output = B3DMakeShared<TAnimationCurve<Quaternion>>();
		*tmp__output = RootMotionEx::GetRotationCurves(thisPtr->GetInternal());

		MonoObject* __output;
		__output = ScriptTAnimationCurveQuaternion::Create(tmp__output);

		return __output;
	}
}
