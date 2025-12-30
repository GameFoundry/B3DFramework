//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"
#include "Math/B3DRadian.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"

namespace b3d { class CharacterController; }
namespace b3d { struct __ControllerColliderCollisionInterop; }
namespace b3d { struct __ControllerControllerCollisionInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCharacterController : public TScriptGameObjectWrapper<CharacterController, ScriptCharacterController>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CharacterController")

		ScriptCharacterController(const TGameObjectHandle<CharacterController>& nativeObject);
		~ScriptCharacterController();

		static void SetupScriptBindings();

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		void OnColliderHit(const ControllerColliderCollision& p0);
		void OnControllerHit(const ControllerControllerCollision& p0);

		typedef void(B3D_THUNKCALL *OnColliderHitThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnColliderHitThunkDefinition OnColliderHitThunk;
		typedef void(B3D_THUNKCALL *OnControllerHitThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnControllerHitThunkDefinition OnControllerHitThunk;

		HEvent OnColliderHitConnection;
		HEvent OnControllerHitConnection;
		static CharacterCollisionFlag InternalMove(ScriptCharacterController* self, TVector3<float>* displacement);
		static void InternalSetFootPosition(ScriptCharacterController* self, TVector3<float>* position);
		static void InternalGetFootPosition(ScriptCharacterController* self, TVector3<float>* __output);
		static void InternalSetRadius(ScriptCharacterController* self, float radius);
		static float InternalGetRadius(ScriptCharacterController* self);
		static void InternalSetHeight(ScriptCharacterController* self, float height);
		static float InternalGetHeight(ScriptCharacterController* self);
		static void InternalSetUp(ScriptCharacterController* self, TVector3<float>* up);
		static void InternalGetUp(ScriptCharacterController* self, TVector3<float>* __output);
		static void InternalSetClimbingMode(ScriptCharacterController* self, CharacterClimbingMode mode);
		static CharacterClimbingMode InternalGetClimbingMode(ScriptCharacterController* self);
		static void InternalSetNonWalkableMode(ScriptCharacterController* self, CharacterNonWalkableMode mode);
		static CharacterNonWalkableMode InternalGetNonWalkableMode(ScriptCharacterController* self);
		static void InternalSetMinMoveDistance(ScriptCharacterController* self, float value);
		static float InternalGetMinMoveDistance(ScriptCharacterController* self);
		static void InternalSetContactOffset(ScriptCharacterController* self, float value);
		static float InternalGetContactOffset(ScriptCharacterController* self);
		static void InternalSetStepOffset(ScriptCharacterController* self, float value);
		static float InternalGetStepOffset(ScriptCharacterController* self);
		static void InternalSetSlopeLimit(ScriptCharacterController* self, TRadian<float>* value);
		static void InternalGetSlopeLimit(ScriptCharacterController* self, TRadian<float>* __output);
		static void InternalSetLayer(ScriptCharacterController* self, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptCharacterController* self);
	};
}
