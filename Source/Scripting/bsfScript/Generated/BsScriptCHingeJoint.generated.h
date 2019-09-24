//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	class BS_SCR_BE_EXPORT ScriptCHingeJoint : public TScriptComponent<ScriptCHingeJoint, CHingeJoint, ScriptCJointBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "HingeJoint")

		ScriptCHingeJoint(MonoObject* managedInstance, const GameObjectHandle<CHingeJoint>& value);

	private:
		static void Internal_getAngle(ScriptCHingeJoint* thisPtr, Radian* __output);
		static float Internal_getSpeed(ScriptCHingeJoint* thisPtr);
		static void Internal_getLimit(ScriptCHingeJoint* thisPtr, __LimitAngularRangeInterop* __output);
		static void Internal_setLimit(ScriptCHingeJoint* thisPtr, __LimitAngularRangeInterop* limit);
		static void Internal_getDrive(ScriptCHingeJoint* thisPtr, HingeJointDrive* __output);
		static void Internal_setDrive(ScriptCHingeJoint* thisPtr, HingeJointDrive* drive);
		static void Internal_setFlag(ScriptCHingeJoint* thisPtr, HingeJointFlag flag, bool enabled);
		static bool Internal_hasFlag(ScriptCHingeJoint* thisPtr, HingeJointFlag flag);
	};
}
