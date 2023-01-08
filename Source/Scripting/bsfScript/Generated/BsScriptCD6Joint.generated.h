//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCJoint.generated.h"
#include "../../../Foundation/bsfCore/Physics/BsD6Joint.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Physics/BsD6Joint.h"
#include "../../../Foundation/bsfCore/Physics/BsD6Joint.h"
#include "Math/BsRadian.h"
#include "../../../Foundation/bsfCore/Physics/BsD6Joint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "../../../Foundation/bsfCore/Physics/BsJoint.h"
#include "Math/BsQuaternion.h"

namespace bs { struct __LimitConeRangeInterop; }
namespace bs { struct __LimitAngularRangeInterop; }
namespace bs { class CD6Joint; }
namespace bs { struct __LimitLinearInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptD6Joint : public TScriptComponent<ScriptD6Joint, CD6Joint, ScriptJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "D6Joint")

		ScriptD6Joint(MonoObject* managedInstance, const GameObjectHandle<CD6Joint>& value);

	private:
		static D6JointMotion InternalGetMotion(ScriptD6Joint* thisPtr, D6JointAxis axis);
		static void InternalSetMotion(ScriptD6Joint* thisPtr, D6JointAxis axis, D6JointMotion motion);
		static void InternalGetTwist(ScriptD6Joint* thisPtr, Radian* __output);
		static void InternalGetSwingY(ScriptD6Joint* thisPtr, Radian* __output);
		static void InternalGetSwingZ(ScriptD6Joint* thisPtr, Radian* __output);
		static void InternalGetLimitLinear(ScriptD6Joint* thisPtr, __LimitLinearInterop* __output);
		static void InternalSetLimitLinear(ScriptD6Joint* thisPtr, __LimitLinearInterop* limit);
		static void InternalGetLimitTwist(ScriptD6Joint* thisPtr, __LimitAngularRangeInterop* __output);
		static void InternalSetLimitTwist(ScriptD6Joint* thisPtr, __LimitAngularRangeInterop* limit);
		static void InternalGetLimitSwing(ScriptD6Joint* thisPtr, __LimitConeRangeInterop* __output);
		static void InternalSetLimitSwing(ScriptD6Joint* thisPtr, __LimitConeRangeInterop* limit);
		static void InternalGetDrive(ScriptD6Joint* thisPtr, D6JointDriveType type, D6JointDrive* __output);
		static void InternalSetDrive(ScriptD6Joint* thisPtr, D6JointDriveType type, D6JointDrive* drive);
		static void InternalGetDrivePosition(ScriptD6Joint* thisPtr, Vector3* __output);
		static void InternalGetDriveRotation(ScriptD6Joint* thisPtr, Quaternion* __output);
		static void InternalSetDriveTransform(ScriptD6Joint* thisPtr, Vector3* position, Quaternion* rotation);
		static void InternalGetDriveLinearVelocity(ScriptD6Joint* thisPtr, Vector3* __output);
		static void InternalGetDriveAngularVelocity(ScriptD6Joint* thisPtr, Vector3* __output);
		static void InternalSetDriveVelocity(ScriptD6Joint* thisPtr, Vector3* linear, Vector3* angular);
	};
}
