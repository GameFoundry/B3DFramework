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
	ScriptRootMotion::ScriptRootMotion(const SPtr<RootMotion>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptRootMotion::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPositionCurves", (void*)&ScriptRootMotion::InternalGetPositionCurves);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRotationCurves", (void*)&ScriptRootMotion::InternalGetRotationCurves);

	}

	MonoObject* ScriptRootMotion::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptRootMotion::InternalGetPositionCurves(ScriptRootMotion* self)
	{
		SPtr<TAnimationCurve<TVector3<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector3<float>>>();
		*tmp__output = RootMotionEx::GetPositionCurves(std::static_pointer_cast<RootMotion>(self->GetBaseNativeObjectAsShared()));

		MonoObject* __output;
		__output = ScriptVector3Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptRootMotion::InternalGetRotationCurves(ScriptRootMotion* self)
	{
		SPtr<TAnimationCurve<Quaternion>> tmp__output = B3DMakeShared<TAnimationCurve<Quaternion>>();
		*tmp__output = RootMotionEx::GetRotationCurves(std::static_pointer_cast<RootMotion>(self->GetBaseNativeObjectAsShared()));

		MonoObject* __output;
		__output = ScriptQuaternionCurve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}
}
