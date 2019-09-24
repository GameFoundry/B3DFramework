//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Physics/BsFJoint.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs { class CJoint; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCJointBase : public ScriptComponentBase
	{
	public:
		ScriptCJointBase(MonoObject* instance);
		virtual ~ScriptCJointBase() {}
	};

	class BS_SCR_BE_EXPORT ScriptCJoint : public TScriptComponent<ScriptCJoint, CJoint, ScriptCJointBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Joint")

		ScriptCJoint(MonoObject* managedInstance, const GameObjectHandle<CJoint>& value);

	private:
		void onJointBreak();

		typedef void(BS_THUNKCALL *onJointBreakThunkDef) (MonoObject*, MonoException**);
		static onJointBreakThunkDef onJointBreakThunk;

		static MonoObject* Internal_getBody(ScriptCJointBase* thisPtr, JointBody body);
		static void Internal_setBody(ScriptCJointBase* thisPtr, JointBody body, MonoObject* value);
		static void Internal_getPosition(ScriptCJointBase* thisPtr, JointBody body, Vector3* __output);
		static void Internal_getRotation(ScriptCJointBase* thisPtr, JointBody body, Quaternion* __output);
		static void Internal_setTransform(ScriptCJointBase* thisPtr, JointBody body, Vector3* position, Quaternion* rotation);
		static float Internal_getBreakForce(ScriptCJointBase* thisPtr);
		static void Internal_setBreakForce(ScriptCJointBase* thisPtr, float force);
		static float Internal_getBreakTorque(ScriptCJointBase* thisPtr);
		static void Internal_setBreakTorque(ScriptCJointBase* thisPtr, float torque);
		static bool Internal_getEnableCollision(ScriptCJointBase* thisPtr);
		static void Internal_setEnableCollision(ScriptCJointBase* thisPtr, bool value);
	};
}
