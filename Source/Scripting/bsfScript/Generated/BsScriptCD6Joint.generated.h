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

namespace bs { struct __LimitConeRangeInterop; }
namespace bs { struct __LimitAngularRangeInterop; }
namespace bs { class CD6Joint; }
namespace bs { struct __LimitLinearInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCD6Joint : public TScriptComponent<ScriptCD6Joint, CD6Joint, ScriptCJointBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "D6Joint")

		ScriptCD6Joint(MonoObject* managedInstance, const GameObjectHandle<CD6Joint>& value);

	private:
		static D6JointMotion Internal_getMotion(ScriptCD6Joint* thisPtr, D6JointAxis axis);
		static void Internal_setMotion(ScriptCD6Joint* thisPtr, D6JointAxis axis, D6JointMotion motion);
		static void Internal_getTwist(ScriptCD6Joint* thisPtr, Radian* __output);
		static void Internal_getSwingY(ScriptCD6Joint* thisPtr, Radian* __output);
		static void Internal_getSwingZ(ScriptCD6Joint* thisPtr, Radian* __output);
		static void Internal_getLimitLinear(ScriptCD6Joint* thisPtr, __LimitLinearInterop* __output);
		static void Internal_setLimitLinear(ScriptCD6Joint* thisPtr, __LimitLinearInterop* limit);
		static void Internal_getLimitTwist(ScriptCD6Joint* thisPtr, __LimitAngularRangeInterop* __output);
		static void Internal_setLimitTwist(ScriptCD6Joint* thisPtr, __LimitAngularRangeInterop* limit);
		static void Internal_getLimitSwing(ScriptCD6Joint* thisPtr, __LimitConeRangeInterop* __output);
		static void Internal_setLimitSwing(ScriptCD6Joint* thisPtr, __LimitConeRangeInterop* limit);
		static void Internal_getDrive(ScriptCD6Joint* thisPtr, D6JointDriveType type, D6JointDrive* __output);
		static void Internal_setDrive(ScriptCD6Joint* thisPtr, D6JointDriveType type, D6JointDrive* drive);
		static void Internal_getDrivePosition(ScriptCD6Joint* thisPtr, Vector3* __output);
		static void Internal_getDriveRotation(ScriptCD6Joint* thisPtr, Quaternion* __output);
		static void Internal_setDriveTransform(ScriptCD6Joint* thisPtr, Vector3* position, Quaternion* rotation);
		static void Internal_getDriveLinearVelocity(ScriptCD6Joint* thisPtr, Vector3* __output);
		static void Internal_getDriveAngularVelocity(ScriptCD6Joint* thisPtr, Vector3* __output);
		static void Internal_setDriveVelocity(ScriptCD6Joint* thisPtr, Vector3* linear, Vector3* angular);
	};
}
