//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Physics/BsCharacterController.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct __ControllerColliderCollisionInterop
	{
		MonoObject* collider;
		uint32_t triangleIndex;
		Vector3 position;
		Vector3 normal;
		Vector3 motionDir;
		float motionAmount;
	};

	class BS_SCR_BE_EXPORT ScriptControllerColliderCollision : public ScriptObject<ScriptControllerColliderCollision>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ControllerColliderCollision")

		static MonoObject* box(const __ControllerColliderCollisionInterop& value);
		static __ControllerColliderCollisionInterop unbox(MonoObject* value);
		static ControllerColliderCollision fromInterop(const __ControllerColliderCollisionInterop& value);
		static __ControllerColliderCollisionInterop toInterop(const ControllerColliderCollision& value);

	private:
		ScriptControllerColliderCollision(MonoObject* managedInstance);

	};
}
