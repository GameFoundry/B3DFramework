//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRootMotion.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Animation/BsAnimationClip.h"
#include "BsScriptTAnimationCurve.generated.h"
#include "../../../Extensions/BsAnimationEx.h"
#include "BsScriptTAnimationCurve.generated.h"

namespace bs
{
	ScriptRootMotion::ScriptRootMotion(MonoObject* managedInstance, const SPtr<RootMotion>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptRootMotion::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getPositionCurves", (void*)&ScriptRootMotion::Internal_getPositionCurves);
		metaData.scriptClass->addInternalCall("Internal_getRotationCurves", (void*)&ScriptRootMotion::Internal_getRotationCurves);

	}

	MonoObject* ScriptRootMotion::create(const SPtr<RootMotion>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptRootMotion>()) ScriptRootMotion(managedInstance, value);
		return managedInstance;
	}
	MonoObject* ScriptRootMotion::Internal_getPositionCurves(ScriptRootMotion* thisPtr)
	{
		SPtr<TAnimationCurve<Vector3>> tmp__output = bs_shared_ptr_new<TAnimationCurve<Vector3>>();
		*tmp__output = RootMotionEx::getPositionCurves(thisPtr->getInternal());

		MonoObject* __output;
		__output = ScriptTAnimationCurveVector3::create(tmp__output);

		return __output;
	}

	MonoObject* ScriptRootMotion::Internal_getRotationCurves(ScriptRootMotion* thisPtr)
	{
		SPtr<TAnimationCurve<Quaternion>> tmp__output = bs_shared_ptr_new<TAnimationCurve<Quaternion>>();
		*tmp__output = RootMotionEx::getRotationCurves(thisPtr->getInternal());

		MonoObject* __output;
		__output = ScriptTAnimationCurveQuaternion::create(tmp__output);

		return __output;
	}
}
