//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Physics/BsCharacterController.h"
#include "../../../Foundation/bsfCore/Physics/BsCharacterController.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Physics/BsCharacterController.h"
#include "../../../Foundation/bsfCore/Physics/BsCharacterController.h"
#include "Math/BsRadian.h"
#include "../../../Foundation/bsfCore/Physics/BsCharacterController.h"

namespace bs
{
	class CCharacterController;
}

namespace bs
{
	struct __ControllerColliderCollisionInterop;
}

namespace bs
{
	struct __ControllerControllerCollisionInterop;
}

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCCharacterController : public TScriptComponent<ScriptCCharacterController, CCharacterController>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "CharacterController")

		ScriptCCharacterController(MonoObject* managedInstance, const GameObjectHandle<CCharacterController>& value);

	private:
		void OnColliderHit(const ControllerColliderCollision& p0);
		void OnControllerHit(const ControllerControllerCollision& p0);

		typedef void(B3D_THUNKCALL* OnColliderHitThunkDef)(MonoObject*, MonoObject* p0, MonoException**);
		static OnColliderHitThunkDef OnColliderHitThunk;
		typedef void(B3D_THUNKCALL* OnControllerHitThunkDef)(MonoObject*, MonoObject* p0, MonoException**);
		static OnControllerHitThunkDef OnControllerHitThunk;

		static CharacterCollisionFlag InternalMove(ScriptCCharacterController* thisPtr, Vector3* displacement);
		static void InternalGetFootPosition(ScriptCCharacterController* thisPtr, Vector3* __output);
		static void InternalSetFootPosition(ScriptCCharacterController* thisPtr, Vector3* position);
		static float InternalGetRadius(ScriptCCharacterController* thisPtr);
		static void InternalSetRadius(ScriptCCharacterController* thisPtr, float radius);
		static float InternalGetHeight(ScriptCCharacterController* thisPtr);
		static void InternalSetHeight(ScriptCCharacterController* thisPtr, float height);
		static void InternalGetUp(ScriptCCharacterController* thisPtr, Vector3* __output);
		static void InternalSetUp(ScriptCCharacterController* thisPtr, Vector3* up);
		static CharacterClimbingMode InternalGetClimbingMode(ScriptCCharacterController* thisPtr);
		static void InternalSetClimbingMode(ScriptCCharacterController* thisPtr, CharacterClimbingMode mode);
		static CharacterNonWalkableMode InternalGetNonWalkableMode(ScriptCCharacterController* thisPtr);
		static void InternalSetNonWalkableMode(ScriptCCharacterController* thisPtr, CharacterNonWalkableMode mode);
		static float InternalGetMinMoveDistance(ScriptCCharacterController* thisPtr);
		static void InternalSetMinMoveDistance(ScriptCCharacterController* thisPtr, float value);
		static float InternalGetContactOffset(ScriptCCharacterController* thisPtr);
		static void InternalSetContactOffset(ScriptCCharacterController* thisPtr, float value);
		static float InternalGetStepOffset(ScriptCCharacterController* thisPtr);
		static void InternalSetStepOffset(ScriptCCharacterController* thisPtr, float value);
		static void InternalGetSlopeLimit(ScriptCCharacterController* thisPtr, Radian* __output);
		static void InternalSetSlopeLimit(ScriptCCharacterController* thisPtr, Radian* value);
		static uint64_t InternalGetLayer(ScriptCCharacterController* thisPtr);
		static void InternalSetLayer(ScriptCCharacterController* thisPtr, uint64_t layer);
	};
} // namespace bs
