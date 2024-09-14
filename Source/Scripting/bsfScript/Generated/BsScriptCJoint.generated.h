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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptJointWrapperBase : public ScriptGameObjectWrapper
	{
	public:
		using ScriptGameObjectWrapper::ScriptGameObjectWrapper;

		virtual void RegisterEvents();
		void OnJointBreak();

		typedef void(B3D_THUNKCALL *OnJointBreakThunkDefinition) (MonoObject*, MonoException**);
		static OnJointBreakThunkDefinition OnJointBreakThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptJoint : public TScriptGameObjectWrapper<CJoint, ScriptJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Joint")

		ScriptJoint(const GameObjectHandle<CJoint>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetBody(ScriptJointWrapperBase* self, JointBody body);
		static void InternalSetBody(ScriptJointWrapperBase* self, JointBody body, MonoObject* value);
		static void InternalGetPosition(ScriptJointWrapperBase* self, JointBody body, TVector3<float>* __output);
		static void InternalGetRotation(ScriptJointWrapperBase* self, JointBody body, Quaternion* __output);
		static void InternalSetTransform(ScriptJointWrapperBase* self, JointBody body, TVector3<float>* position, Quaternion* rotation);
		static float InternalGetBreakForce(ScriptJointWrapperBase* self);
		static void InternalSetBreakForce(ScriptJointWrapperBase* self, float force);
		static float InternalGetBreakTorque(ScriptJointWrapperBase* self);
		static void InternalSetBreakTorque(ScriptJointWrapperBase* self, float torque);
		static bool InternalGetEnableCollision(ScriptJointWrapperBase* self);
		static void InternalSetEnableCollision(ScriptJointWrapperBase* self, bool value);
	};
}
