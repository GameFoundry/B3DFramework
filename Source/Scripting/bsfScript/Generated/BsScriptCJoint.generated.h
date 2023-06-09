//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Physics/BsFJoint.h"
#include "Math/BsQuaternion.h"

namespace bs { class CJoint; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptJointBase : public ScriptComponentBase
	{
	public:
		ScriptJointBase(MonoObject* instance);
		virtual ~ScriptJointBase() {}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptJoint : public TScriptComponent<ScriptJoint, CJoint, ScriptJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Joint")

		ScriptJoint(MonoObject* managedInstance, const GameObjectHandle<CJoint>& value);

	private:
		void OnJointBreak();

		typedef void(B3D_THUNKCALL *OnJointBreakThunkDef) (MonoObject*, MonoException**);
		static OnJointBreakThunkDef OnJointBreakThunk;

		static MonoObject* InternalGetBody(ScriptJointBase* thisPtr, JointBody body);
		static void InternalSetBody(ScriptJointBase* thisPtr, JointBody body, MonoObject* value);
		static void InternalGetPosition(ScriptJointBase* thisPtr, JointBody body, TVector3<float>* __output);
		static void InternalGetRotation(ScriptJointBase* thisPtr, JointBody body, Quaternion* __output);
		static void InternalSetTransform(ScriptJointBase* thisPtr, JointBody body, TVector3<float>* position, Quaternion* rotation);
		static float InternalGetBreakForce(ScriptJointBase* thisPtr);
		static void InternalSetBreakForce(ScriptJointBase* thisPtr, float force);
		static float InternalGetBreakTorque(ScriptJointBase* thisPtr);
		static void InternalSetBreakTorque(ScriptJointBase* thisPtr, float torque);
		static bool InternalGetEnableCollision(ScriptJointBase* thisPtr);
		static void InternalSetEnableCollision(ScriptJointBase* thisPtr, bool value);
	};
}
