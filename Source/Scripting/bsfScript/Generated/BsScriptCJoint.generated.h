//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Physics/BsFJoint.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs
{
	class CJoint;
}

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
		void OnJointBreak();

		typedef void(BS_THUNKCALL* OnJointBreakThunkDef)(MonoObject*, MonoException**);
		static OnJointBreakThunkDef OnJointBreakThunk;

		static MonoObject* InternalGetBody(ScriptCJointBase* thisPtr, JointBody body);
		static void InternalSetBody(ScriptCJointBase* thisPtr, JointBody body, MonoObject* value);
		static void InternalGetPosition(ScriptCJointBase* thisPtr, JointBody body, Vector3* __output);
		static void InternalGetRotation(ScriptCJointBase* thisPtr, JointBody body, Quaternion* __output);
		static void InternalSetTransform(ScriptCJointBase* thisPtr, JointBody body, Vector3* position, Quaternion* rotation);
		static float InternalGetBreakForce(ScriptCJointBase* thisPtr);
		static void InternalSetBreakForce(ScriptCJointBase* thisPtr, float force);
		static float InternalGetBreakTorque(ScriptCJointBase* thisPtr);
		static void InternalSetBreakTorque(ScriptCJointBase* thisPtr, float torque);
		static bool InternalGetEnableCollision(ScriptCJointBase* thisPtr);
		static void InternalSetEnableCollision(ScriptCJointBase* thisPtr, bool value);
	};
} // namespace bs
