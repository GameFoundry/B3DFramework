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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCHingeJoint : public TScriptComponent<ScriptCHingeJoint, CHingeJoint, ScriptCJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "HingeJoint")

		ScriptCHingeJoint(MonoObject* managedInstance, const GameObjectHandle<CHingeJoint>& value);

	private:
		static void InternalGetAngle(ScriptCHingeJoint* thisPtr, Radian* __output);
		static float InternalGetSpeed(ScriptCHingeJoint* thisPtr);
		static void InternalGetLimit(ScriptCHingeJoint* thisPtr, __LimitAngularRangeInterop* __output);
		static void InternalSetLimit(ScriptCHingeJoint* thisPtr, __LimitAngularRangeInterop* limit);
		static void InternalGetDrive(ScriptCHingeJoint* thisPtr, HingeJointDrive* __output);
		static void InternalSetDrive(ScriptCHingeJoint* thisPtr, HingeJointDrive* drive);
		static void InternalSetFlag(ScriptCHingeJoint* thisPtr, HingeJointFlag flag, bool enabled);
		static bool InternalHasFlag(ScriptCHingeJoint* thisPtr, HingeJointFlag flag);
	};
}
