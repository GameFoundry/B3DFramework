//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

namespace bs
{
	struct __LimitConeRangeInterop;
}

namespace bs
{
	struct __LimitAngularRangeInterop;
}

namespace bs
{
	class CD6Joint;
}

namespace bs
{
	struct __LimitLinearInterop;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCD6Joint : public TScriptComponent<ScriptCD6Joint, CD6Joint, ScriptCJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "D6Joint")

		ScriptCD6Joint(MonoObject* managedInstance, const GameObjectHandle<CD6Joint>& value);

	private:
		static D6JointMotion InternalGetMotion(ScriptCD6Joint* thisPtr, D6JointAxis axis);
		static void InternalSetMotion(ScriptCD6Joint* thisPtr, D6JointAxis axis, D6JointMotion motion);
		static void InternalGetTwist(ScriptCD6Joint* thisPtr, Radian* __output);
		static void InternalGetSwingY(ScriptCD6Joint* thisPtr, Radian* __output);
		static void InternalGetSwingZ(ScriptCD6Joint* thisPtr, Radian* __output);
		static void InternalGetLimitLinear(ScriptCD6Joint* thisPtr, __LimitLinearInterop* __output);
		static void InternalSetLimitLinear(ScriptCD6Joint* thisPtr, __LimitLinearInterop* limit);
		static void InternalGetLimitTwist(ScriptCD6Joint* thisPtr, __LimitAngularRangeInterop* __output);
		static void InternalSetLimitTwist(ScriptCD6Joint* thisPtr, __LimitAngularRangeInterop* limit);
		static void InternalGetLimitSwing(ScriptCD6Joint* thisPtr, __LimitConeRangeInterop* __output);
		static void InternalSetLimitSwing(ScriptCD6Joint* thisPtr, __LimitConeRangeInterop* limit);
		static void InternalGetDrive(ScriptCD6Joint* thisPtr, D6JointDriveType type, D6JointDrive* __output);
		static void InternalSetDrive(ScriptCD6Joint* thisPtr, D6JointDriveType type, D6JointDrive* drive);
		static void InternalGetDrivePosition(ScriptCD6Joint* thisPtr, Vector3* __output);
		static void InternalGetDriveRotation(ScriptCD6Joint* thisPtr, Quaternion* __output);
		static void InternalSetDriveTransform(ScriptCD6Joint* thisPtr, Vector3* position, Quaternion* rotation);
		static void InternalGetDriveLinearVelocity(ScriptCD6Joint* thisPtr, Vector3* __output);
		static void InternalGetDriveAngularVelocity(ScriptCD6Joint* thisPtr, Vector3* __output);
		static void InternalSetDriveVelocity(ScriptCD6Joint* thisPtr, Vector3* linear, Vector3* angular);
	};
} // namespace bs
