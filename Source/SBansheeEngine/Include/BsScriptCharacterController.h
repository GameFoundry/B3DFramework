//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsCharacterController.h"
#include "BsScriptControllerCollision.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for CharacterController. */
	class BS_SCR_BE_EXPORT ScriptCharacterController : public ScriptObject<ScriptCharacterController>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeCharacterController")

		/** Returns the native CharacterController object. */
		CharacterController* getCharController() const { return mCharacterController.get(); }

	private:
		friend class ScriptColliderBase;

		ScriptCharacterController(MonoObject* instance, const SPtr<CharacterController>& controller);

		/** Triggered when the controller hits a collider. */
		static void onColliderHit(MonoObject* instance, const ControllerColliderCollision& collisionData);

		/** Triggered when the controller hits another controller. */
		static void onControllerHit(MonoObject* instance, const ControllerControllerCollision& collisionData);

		SPtr<CharacterController> mCharacterController;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, CHAR_CONTROLLER_DESC* initData);
		static void internal_Destroy(ScriptCharacterController* thisPtr);

		static UINT32 internal_Move(ScriptCharacterController* thisPtr, Vector3* displacement);

		static void internal_GetPosition(ScriptCharacterController* thisPtr, Vector3* position);
		static void internal_SetPosition(ScriptCharacterController* thisPtr, Vector3* position);

		static void internal_GetFootPosition(ScriptCharacterController* thisPtr, Vector3* position);
		static void internal_SetFootPosition(ScriptCharacterController* thisPtr, Vector3* position);

		static void internal_SetRadius(ScriptCharacterController* thisPtr, float radius);
		static void internal_SetHeight(ScriptCharacterController* thisPtr, float height);
		static void internal_SetUp(ScriptCharacterController* thisPtr, Vector3* up);
		static void internal_SetClimbingMode(ScriptCharacterController* thisPtr, CharacterClimbingMode mode);
		static void internal_SetNonWalkableMode(ScriptCharacterController* thisPtr, CharacterNonWalkableMode mode);
		static void internal_SetMinMoveDistance(ScriptCharacterController* thisPtr, float value);
		static void internal_SetContactOffset(ScriptCharacterController* thisPtr, float value);
		static void internal_SetStepOffset(ScriptCharacterController* thisPtr, float value);
		static void internal_SetSlopeLimit(ScriptCharacterController* thisPtr, float value);
		static void internal_SetLayer(ScriptCharacterController* thisPtr, UINT64 layer);

		typedef void(__stdcall *OnHitThunkDef) (MonoObject*, MonoObject*, MonoException**);

		static OnHitThunkDef onColliderHitThunk;
		static OnHitThunkDef onControllerHitThunk;
	};

	/** @} */
}