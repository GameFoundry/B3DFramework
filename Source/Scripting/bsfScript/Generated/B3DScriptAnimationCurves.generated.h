//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/Animation/B3DAnimationClip.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"

namespace b3d { class AnimationCurvesEx; }
namespace b3d { struct __TNamedAnimationCurve_float_Interop; }
namespace b3d { struct __TNamedAnimationCurve_TVector3_float__Interop; }
namespace b3d { struct __TNamedAnimationCurve_TQuaternion_float__Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationCurves : public TScriptNonReflectableWrapper<AnimationCurves, ScriptAnimationCurves>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AnimationCurves")

		ScriptAnimationCurves(const TShared<AnimationCurves>& nativeObject);
		~ScriptAnimationCurves();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalAnimationCurves(MonoObject* scriptObject);
		static void InternalAddPositionCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve);
		static void InternalAddRotationCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve);
		static void InternalAddScaleCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve);
		static void InternalAddGenericCurve(ScriptAnimationCurves* self, MonoString* name, MonoObject* curve);
		static void InternalRemovePositionCurve(ScriptAnimationCurves* self, MonoString* name);
		static void InternalRemoveRotationCurve(ScriptAnimationCurves* self, MonoString* name);
		static void InternalRemoveScaleCurve(ScriptAnimationCurves* self, MonoString* name);
		static void InternalRemoveGenericCurve(ScriptAnimationCurves* self, MonoString* name);
		static MonoArray* InternalGetPositionCurves(ScriptAnimationCurves* self);
		static void InternalSetPositionCurves(ScriptAnimationCurves* self, MonoArray* value);
		static MonoArray* InternalGetRotationCurves(ScriptAnimationCurves* self);
		static void InternalSetRotationCurves(ScriptAnimationCurves* self, MonoArray* value);
		static MonoArray* InternalGetScaleCurves(ScriptAnimationCurves* self);
		static void InternalSetScaleCurves(ScriptAnimationCurves* self, MonoArray* value);
		static MonoArray* InternalGetGenericCurves(ScriptAnimationCurves* self);
		static void InternalSetGenericCurves(ScriptAnimationCurves* self, MonoArray* value);
	};
}
