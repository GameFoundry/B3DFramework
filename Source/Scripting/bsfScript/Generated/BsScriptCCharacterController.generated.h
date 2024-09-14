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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCharacterController : public TScriptGameObjectWrapper<CCharacterController, ScriptCharacterController>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CharacterController")

		ScriptCharacterController(const GameObjectHandle<CCharacterController>& nativeObject);

		static void SetupScriptBindings();

		virtual void RegisterEvents();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		void OnColliderHit(const ControllerColliderCollision& p0);
		void OnControllerHit(const ControllerControllerCollision& p0);

		typedef void(B3D_THUNKCALL *OnColliderHitThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnColliderHitThunkDefinition OnColliderHitThunk;
		typedef void(B3D_THUNKCALL *OnControllerHitThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnControllerHitThunkDefinition OnControllerHitThunk;

		static CharacterCollisionFlag InternalMove(ScriptCharacterController* self, TVector3<float>* displacement);
		static void InternalGetFootPosition(ScriptCharacterController* self, TVector3<float>* __output);
		static void InternalSetFootPosition(ScriptCharacterController* self, TVector3<float>* position);
		static float InternalGetRadius(ScriptCharacterController* self);
		static void InternalSetRadius(ScriptCharacterController* self, float radius);
		static float InternalGetHeight(ScriptCharacterController* self);
		static void InternalSetHeight(ScriptCharacterController* self, float height);
		static void InternalGetUp(ScriptCharacterController* self, TVector3<float>* __output);
		static void InternalSetUp(ScriptCharacterController* self, TVector3<float>* up);
		static CharacterClimbingMode InternalGetClimbingMode(ScriptCharacterController* self);
		static void InternalSetClimbingMode(ScriptCharacterController* self, CharacterClimbingMode mode);
		static CharacterNonWalkableMode InternalGetNonWalkableMode(ScriptCharacterController* self);
		static void InternalSetNonWalkableMode(ScriptCharacterController* self, CharacterNonWalkableMode mode);
		static float InternalGetMinMoveDistance(ScriptCharacterController* self);
		static void InternalSetMinMoveDistance(ScriptCharacterController* self, float value);
		static float InternalGetContactOffset(ScriptCharacterController* self);
		static void InternalSetContactOffset(ScriptCharacterController* self, float value);
		static float InternalGetStepOffset(ScriptCharacterController* self);
		static void InternalSetStepOffset(ScriptCharacterController* self, float value);
		static void InternalGetSlopeLimit(ScriptCharacterController* self, TRadian<float>* __output);
		static void InternalSetSlopeLimit(ScriptCharacterController* self, TRadian<float>* value);
		static uint64_t InternalGetLayer(ScriptCharacterController* self);
		static void InternalSetLayer(ScriptCharacterController* self, uint64_t layer);
	};
}
