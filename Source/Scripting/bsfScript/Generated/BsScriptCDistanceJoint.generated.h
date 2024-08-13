//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsDistanceJoint.h"

namespace bs { class CDistanceJoint; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDistanceJoint : public TScriptComponent<ScriptDistanceJoint, CDistanceJoint, ScriptJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "DistanceJoint")

		ScriptDistanceJoint(MonoObject* managedInstance, const GameObjectHandle<CDistanceJoint>& value);

	private:
		static float InternalGetDistance(ScriptDistanceJoint* self);
		static float InternalGetMinDistance(ScriptDistanceJoint* self);
		static void InternalSetMinDistance(ScriptDistanceJoint* self, float value);
		static float InternalGetMaxDistance(ScriptDistanceJoint* self);
		static void InternalSetMaxDistance(ScriptDistanceJoint* self, float value);
		static float InternalGetTolerance(ScriptDistanceJoint* self);
		static void InternalSetTolerance(ScriptDistanceJoint* self, float value);
		static void InternalGetSpring(ScriptDistanceJoint* self, Spring* __output);
		static void InternalSetSpring(ScriptDistanceJoint* self, Spring* value);
		static void InternalSetFlag(ScriptDistanceJoint* self, DistanceJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptDistanceJoint* self, DistanceJointFlag flag);
	};
}
