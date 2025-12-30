//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptJoint.generated.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "../../../Foundation/Core/Components/B3DDistanceJoint.h"

namespace b3d { class DistanceJoint; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDistanceJoint : public TScriptGameObjectWrapper<DistanceJoint, ScriptDistanceJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "DistanceJoint")

		ScriptDistanceJoint(const TGameObjectHandle<DistanceJoint>& nativeObject);
		~ScriptDistanceJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static float InternalGetDistance(ScriptDistanceJoint* self);
		static void InternalSetMinDistance(ScriptDistanceJoint* self, float value);
		static float InternalGetMinDistance(ScriptDistanceJoint* self);
		static void InternalSetMaxDistance(ScriptDistanceJoint* self, float value);
		static float InternalGetMaxDistance(ScriptDistanceJoint* self);
		static void InternalSetTolerance(ScriptDistanceJoint* self, float value);
		static float InternalGetTolerance(ScriptDistanceJoint* self);
		static void InternalSetSpring(ScriptDistanceJoint* self, Spring* value);
		static void InternalGetSpring(ScriptDistanceJoint* self, Spring* __output);
		static void InternalSetFlag(ScriptDistanceJoint* self, DistanceJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptDistanceJoint* self, DistanceJointFlag flag);
	};
}
