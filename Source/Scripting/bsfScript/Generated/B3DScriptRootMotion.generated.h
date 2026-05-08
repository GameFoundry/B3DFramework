//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/Animation/B3DAnimationClip.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"

namespace b3d { class RootMotionEx; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRootMotion : public TScriptNonReflectableWrapper<RootMotion, ScriptRootMotion>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RootMotion")

		ScriptRootMotion(const TShared<RootMotion>& nativeObject);
		~ScriptRootMotion();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetPositionCurves(ScriptRootMotion* self);
		static MonoObject* InternalGetRotationCurves(ScriptRootMotion* self);
	};
}
