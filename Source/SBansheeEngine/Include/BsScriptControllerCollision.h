//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsCharacterController.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop struct between C++ & CLR for ControllerCollision. */
	struct ScriptControllerCollision // Note: Must match C++ struct ScriptControllerCollision
	{
		Vector3 position;
		Vector3 normal;
		Vector3 motionDir;
		float motionAmount;
		MonoObject* collider;
		int triangleIndex;
		MonoObject* controller;
	};

	/** Helper class for dealing with ControllerCollision structure. */
	class BS_SCR_BE_EXPORT ScriptControllerCollisionHelper : public ScriptObject<ScriptControllerCollisionHelper>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "ScriptControllerCollision")

		/** Converts native collision data to its managed counterpart. */
		static ScriptControllerCollision create(const ControllerColliderCollision& data);

		/** Converts native collision data to its managed counterpart. */
		static ScriptControllerCollision create(const ControllerControllerCollision& data);

		/** Unboxes a boxed managed ScriptControllerCollision struct and returns the native version of the structure. */
		static ScriptControllerCollision unbox(MonoObject* obj);

		/** Boxes a native ScriptControllerCollision struct and returns a managed object containing it. */
		static MonoObject* box(const ScriptControllerCollision& value);

	private:
		ScriptControllerCollisionHelper(MonoObject* instance);
	};

	/** @} */
}