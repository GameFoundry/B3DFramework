//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "../../../Foundation/Core/Components/B3DJoint.h"
#include "../../../Foundation/Utility/Math/B3DQuaternion.h"

namespace b3d { class Joint; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptJointWrapperBase : public ScriptGameObjectWrapper
	{
	public:
		using ScriptGameObjectWrapper::ScriptGameObjectWrapper;

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		void OnJointBreak();

		typedef void(B3D_THUNKCALL *OnJointBreakThunkDefinition) (MonoObject*, MonoException**);
		static OnJointBreakThunkDefinition OnJointBreakThunk;

		HEvent OnJointBreakConnection;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptJoint : public TScriptGameObjectWrapper<Joint, ScriptJoint, ScriptJointWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Joint")

		ScriptJoint(const TGameObjectHandle<Joint>& nativeObject);
		~ScriptJoint();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetBody(ScriptJointWrapperBase* self, JointBody body, MonoObject* value);
		static MonoObject* InternalGetBody(ScriptJointWrapperBase* self, JointBody body);
		static void InternalGetRelativeBodyPosition(ScriptJointWrapperBase* self, JointBody body, TVector3<float>* __output);
		static void InternalGetRelativeBodyRotation(ScriptJointWrapperBase* self, JointBody body, TQuaternion<float>* __output);
		static void InternalSetRelativeBodyTransform(ScriptJointWrapperBase* self, JointBody body, TVector3<float>* position, TQuaternion<float>* rotation);
		static void InternalSetBreakForce(ScriptJointWrapperBase* self, float force);
		static float InternalGetBreakForce(ScriptJointWrapperBase* self);
		static void InternalSetBreakTorque(ScriptJointWrapperBase* self, float torque);
		static float InternalGetBreakTorque(ScriptJointWrapperBase* self);
		static void InternalSetEnableCollision(ScriptJointWrapperBase* self, bool value);
		static bool InternalGetEnableCollision(ScriptJointWrapperBase* self);
	};
}
