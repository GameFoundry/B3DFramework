//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsCommon.h"
#include "../../../Foundation/Core/Components/B3DRigidbody.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "../../../Foundation/Utility/Math/B3DQuaternion.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsCommon.h"
#include "../../../Foundation/Core/Components/B3DRigidbody.h"
#include "../../../Foundation/Core/Components/B3DRigidbody.h"

namespace b3d { struct __CollisionDataInterop; }
namespace b3d { class Rigidbody; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRigidbody : public TScriptGameObjectWrapper<Rigidbody, ScriptRigidbody>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Rigidbody")

		ScriptRigidbody(const TGameObjectHandle<Rigidbody>& nativeObject);
		~ScriptRigidbody();

		static void SetupScriptBindings();

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		void OnCollisionBegin(const CollisionData& p0);
		void OnCollisionStay(const CollisionData& p0);
		void OnCollisionEnd(const CollisionData& p0);

		typedef void(B3D_THUNKCALL *OnCollisionBeginThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionBeginThunkDefinition OnCollisionBeginThunk;
		typedef void(B3D_THUNKCALL *OnCollisionStayThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionStayThunkDefinition OnCollisionStayThunk;
		typedef void(B3D_THUNKCALL *OnCollisionEndThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionEndThunkDefinition OnCollisionEndThunk;

		HEvent OnCollisionBeginConnection;
		HEvent OnCollisionStayConnection;
		HEvent OnCollisionEndConnection;
		static void InternalMove(ScriptRigidbody* self, TVector3<float>* position);
		static void InternalRotate(ScriptRigidbody* self, TQuaternion<float>* rotation);
		static void InternalSetMass(ScriptRigidbody* self, float mass);
		static float InternalGetMass(ScriptRigidbody* self);
		static void InternalSetIsKinematic(ScriptRigidbody* self, bool kinematic);
		static bool InternalGetIsKinematic(ScriptRigidbody* self);
		static bool InternalIsSleeping(ScriptRigidbody* self);
		static void InternalSleep(ScriptRigidbody* self);
		static void InternalWakeUp(ScriptRigidbody* self);
		static void InternalSetSleepThreshold(ScriptRigidbody* self, float threshold);
		static float InternalGetSleepThreshold(ScriptRigidbody* self);
		static void InternalSetUseGravity(ScriptRigidbody* self, bool gravity);
		static bool InternalGetUseGravity(ScriptRigidbody* self);
		static void InternalSetVelocity(ScriptRigidbody* self, TVector3<float>* velocity);
		static void InternalGetVelocity(ScriptRigidbody* self, TVector3<float>* __output);
		static void InternalSetAngularVelocity(ScriptRigidbody* self, TVector3<float>* velocity);
		static void InternalGetAngularVelocity(ScriptRigidbody* self, TVector3<float>* __output);
		static void InternalSetDrag(ScriptRigidbody* self, float drag);
		static float InternalGetDrag(ScriptRigidbody* self);
		static void InternalSetAngularDrag(ScriptRigidbody* self, float drag);
		static float InternalGetAngularDrag(ScriptRigidbody* self);
		static void InternalSetInertiaTensor(ScriptRigidbody* self, TVector3<float>* tensor);
		static void InternalGetInertiaTensor(ScriptRigidbody* self, TVector3<float>* __output);
		static void InternalSetMaxAngularVelocity(ScriptRigidbody* self, float velocity);
		static float InternalGetMaxAngularVelocity(ScriptRigidbody* self);
		static void InternalSetCenterOfMassPosition(ScriptRigidbody* self, TVector3<float>* position);
		static void InternalGetCenterOfMassPosition(ScriptRigidbody* self, TVector3<float>* __output);
		static void InternalSetCenterOfMassRotation(ScriptRigidbody* self, TQuaternion<float>* rotation);
		static void InternalGetCenterOfMassRotation(ScriptRigidbody* self, TQuaternion<float>* __output);
		static void InternalSetPositionSolverCount(ScriptRigidbody* self, uint32_t count);
		static uint32_t InternalGetPositionSolverCount(ScriptRigidbody* self);
		static void InternalSetVelocitySolverCount(ScriptRigidbody* self, uint32_t count);
		static uint32_t InternalGetVelocitySolverCount(ScriptRigidbody* self);
		static void InternalSetCollisionReportMode(ScriptRigidbody* self, CollisionReportMode mode);
		static CollisionReportMode InternalGetCollisionReportMode(ScriptRigidbody* self);
		static void InternalSetFlags(ScriptRigidbody* self, RigidbodyFlag flags);
		static RigidbodyFlag InternalGetFlags(ScriptRigidbody* self);
		static void InternalAddForce(ScriptRigidbody* self, TVector3<float>* force, ForceMode mode);
		static void InternalAddTorque(ScriptRigidbody* self, TVector3<float>* torque, ForceMode mode);
		static void InternalAddForceAtPoint(ScriptRigidbody* self, TVector3<float>* force, TVector3<float>* position, PointForceMode mode);
		static void InternalGetVelocityAtPoint(ScriptRigidbody* self, TVector3<float>* point, TVector3<float>* __output);
	};
}
