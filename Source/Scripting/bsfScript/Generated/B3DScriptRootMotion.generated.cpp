//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRootMotion.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Animation/B3DAnimationClip.h"
#include "B3DScriptTAnimationCurve.generated.h"
#include "../Extensions/B3DAnimationEx.h"
#include "B3DScriptTAnimationCurve.generated.h"

namespace b3d
{
	ScriptRootMotion::ScriptRootMotion(const TShared<RootMotion>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptRootMotion::~ScriptRootMotion()
	{
		UnregisterEvents();
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
		TShared<TAnimationCurve<TVector3<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TVector3<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = RootMotionEx::GetPositionCurves(std::static_pointer_cast<RootMotion>(self->GetBaseNativeObjectAsShared()));

		MonoObject* __output;
		__output = ScriptVector3Curve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptRootMotion::InternalGetRotationCurves(ScriptRootMotion* self)
	{
		TShared<TAnimationCurve<TQuaternion<float>>> tmp__output = B3DMakeShared<TAnimationCurve<TQuaternion<float>>>();
		if(!self->IsNativeObjectValid())
			return {};

		*tmp__output = RootMotionEx::GetRotationCurves(std::static_pointer_cast<RootMotion>(self->GetBaseNativeObjectAsShared()));

		MonoObject* __output;
		__output = ScriptQuaternionCurve::GetOrCreateScriptObject(tmp__output);

		return __output;
	}
}
