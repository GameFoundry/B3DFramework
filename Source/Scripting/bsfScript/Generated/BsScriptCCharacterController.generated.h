//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

namespace bs { class CCharacterController; }
namespace bs { struct __ControllerColliderCollisionInterop; }
namespace bs { struct __ControllerControllerCollisionInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCharacterController : public TScriptComponent<ScriptCharacterController, CCharacterController>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "CharacterController")

		ScriptCharacterController(MonoObject* managedInstance, const GameObjectHandle<CCharacterController>& value);

	private:
		void OnColliderHit(const ControllerColliderCollision& p0);
		void OnControllerHit(const ControllerControllerCollision& p0);

		typedef void(B3D_THUNKCALL *OnColliderHitThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static OnColliderHitThunkDef OnColliderHitThunk;
		typedef void(B3D_THUNKCALL *OnControllerHitThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static OnControllerHitThunkDef OnControllerHitThunk;

		static CharacterCollisionFlag InternalMove(ScriptCharacterController* thisPtr, TVector3<float>* displacement);
		static void InternalGetFootPosition(ScriptCharacterController* thisPtr, TVector3<float>* __output);
		static void InternalSetFootPosition(ScriptCharacterController* thisPtr, TVector3<float>* position);
		static float InternalGetRadius(ScriptCharacterController* thisPtr);
		static void InternalSetRadius(ScriptCharacterController* thisPtr, float radius);
		static float InternalGetHeight(ScriptCharacterController* thisPtr);
		static void InternalSetHeight(ScriptCharacterController* thisPtr, float height);
		static void InternalGetUp(ScriptCharacterController* thisPtr, TVector3<float>* __output);
		static void InternalSetUp(ScriptCharacterController* thisPtr, TVector3<float>* up);
		static CharacterClimbingMode InternalGetClimbingMode(ScriptCharacterController* thisPtr);
		static void InternalSetClimbingMode(ScriptCharacterController* thisPtr, CharacterClimbingMode mode);
		static CharacterNonWalkableMode InternalGetNonWalkableMode(ScriptCharacterController* thisPtr);
		static void InternalSetNonWalkableMode(ScriptCharacterController* thisPtr, CharacterNonWalkableMode mode);
		static float InternalGetMinMoveDistance(ScriptCharacterController* thisPtr);
		static void InternalSetMinMoveDistance(ScriptCharacterController* thisPtr, float value);
		static float InternalGetContactOffset(ScriptCharacterController* thisPtr);
		static void InternalSetContactOffset(ScriptCharacterController* thisPtr, float value);
		static float InternalGetStepOffset(ScriptCharacterController* thisPtr);
		static void InternalSetStepOffset(ScriptCharacterController* thisPtr, float value);
		static void InternalGetSlopeLimit(ScriptCharacterController* thisPtr, TRadian<float>* __output);
		static void InternalSetSlopeLimit(ScriptCharacterController* thisPtr, TRadian<float>* value);
		static uint64_t InternalGetLayer(ScriptCharacterController* thisPtr);
		static void InternalSetLayer(ScriptCharacterController* thisPtr, uint64_t layer);
	};
}
