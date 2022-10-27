//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsDistanceJoint.h"

namespace bs
{
	class CDistanceJoint;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCDistanceJoint : public TScriptComponent<ScriptCDistanceJoint, CDistanceJoint, ScriptCJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "DistanceJoint")

		ScriptCDistanceJoint(MonoObject* managedInstance, const GameObjectHandle<CDistanceJoint>& value);

	private:
		static float InternalGetDistance(ScriptCDistanceJoint* thisPtr);
		static float InternalGetMinDistance(ScriptCDistanceJoint* thisPtr);
		static void InternalSetMinDistance(ScriptCDistanceJoint* thisPtr, float value);
		static float InternalGetMaxDistance(ScriptCDistanceJoint* thisPtr);
		static void InternalSetMaxDistance(ScriptCDistanceJoint* thisPtr, float value);
		static float InternalGetTolerance(ScriptCDistanceJoint* thisPtr);
		static void InternalSetTolerance(ScriptCDistanceJoint* thisPtr, float value);
		static void InternalGetSpring(ScriptCDistanceJoint* thisPtr, Spring* __output);
		static void InternalSetSpring(ScriptCDistanceJoint* thisPtr, Spring* value);
		static void InternalSetFlag(ScriptCDistanceJoint* thisPtr, DistanceJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptCDistanceJoint* thisPtr, DistanceJointFlag flag);
	};
} // namespace bs
