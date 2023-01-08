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
		static float InternalGetDistance(ScriptDistanceJoint* thisPtr);
		static float InternalGetMinDistance(ScriptDistanceJoint* thisPtr);
		static void InternalSetMinDistance(ScriptDistanceJoint* thisPtr, float value);
		static float InternalGetMaxDistance(ScriptDistanceJoint* thisPtr);
		static void InternalSetMaxDistance(ScriptDistanceJoint* thisPtr, float value);
		static float InternalGetTolerance(ScriptDistanceJoint* thisPtr);
		static void InternalSetTolerance(ScriptDistanceJoint* thisPtr, float value);
		static void InternalGetSpring(ScriptDistanceJoint* thisPtr, Spring* __output);
		static void InternalSetSpring(ScriptDistanceJoint* thisPtr, Spring* value);
		static void InternalSetFlag(ScriptDistanceJoint* thisPtr, DistanceJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptDistanceJoint* thisPtr, DistanceJointFlag flag);
	};
}
