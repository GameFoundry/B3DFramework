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
		void OnJointBreak();

		typedef void(B3D_THUNKCALL *OnJointBreakThunkDef) (MonoObject*, MonoException**);
		static OnJointBreakThunkDef OnJointBreakThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptJoint : public TScriptComponent<ScriptJoint, CJoint, ScriptJointBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Joint")

		ScriptJoint(MonoObject* managedInstance, const GameObjectHandle<CJoint>& value);

	private:
		static MonoObject* InternalGetBody(ScriptJointBase* self, JointBody body);
		static void InternalSetBody(ScriptJointBase* self, JointBody body, MonoObject* value);
		static void InternalGetPosition(ScriptJointBase* self, JointBody body, TVector3<float>* __output);
		static void InternalGetRotation(ScriptJointBase* self, JointBody body, Quaternion* __output);
		static void InternalSetTransform(ScriptJointBase* self, JointBody body, TVector3<float>* position, Quaternion* rotation);
		static float InternalGetBreakForce(ScriptJointBase* self);
		static void InternalSetBreakForce(ScriptJointBase* self, float force);
		static float InternalGetBreakTorque(ScriptJointBase* self);
		static void InternalSetBreakTorque(ScriptJointBase* self, float torque);
		static bool InternalGetEnableCollision(ScriptJointBase* self);
		static void InternalSetEnableCollision(ScriptJointBase* self, bool value);
	};
}
