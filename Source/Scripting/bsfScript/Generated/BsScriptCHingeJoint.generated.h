//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsHingeJoint.h"
#include "Math/BsRadian.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsHingeJoint.h"

namespace bs { class CHingeJoint; }
namespace bs { struct __LimitAngularRangeInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptHingeJoint : public TScriptComponent<ScriptHingeJoint, CHingeJoint, ScriptJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "HingeJoint")

		ScriptHingeJoint(MonoObject* managedInstance, const GameObjectHandle<CHingeJoint>& value);

	private:
		static void InternalGetAngle(ScriptHingeJoint* thisPtr, Radian* __output);
		static float InternalGetSpeed(ScriptHingeJoint* thisPtr);
		static void InternalGetLimit(ScriptHingeJoint* thisPtr, __LimitAngularRangeInterop* __output);
		static void InternalSetLimit(ScriptHingeJoint* thisPtr, __LimitAngularRangeInterop* limit);
		static void InternalGetDrive(ScriptHingeJoint* thisPtr, HingeJointDrive* __output);
		static void InternalSetDrive(ScriptHingeJoint* thisPtr, HingeJointDrive* drive);
		static void InternalSetFlag(ScriptHingeJoint* thisPtr, HingeJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptHingeJoint* thisPtr, HingeJointFlag flag);
	};
}
