//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsCharacterController.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct __ControllerColliderCollisionInterop
	{
		MonoObject* Collider;
		uint32_t TriangleIndex;
		Vector3 Position;
		Vector3 Normal;
		Vector3 MotionDir;
		float MotionAmount;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptControllerColliderCollision : public ScriptObject<ScriptControllerColliderCollision>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ControllerColliderCollision")

		static MonoObject* Box(const __ControllerColliderCollisionInterop& value);
		static __ControllerColliderCollisionInterop Unbox(MonoObject* value);
		static ControllerColliderCollision FromInterop(const __ControllerColliderCollisionInterop& value);
		static __ControllerColliderCollisionInterop ToInterop(const ControllerColliderCollision& value);

	private:
		ScriptControllerColliderCollision(MonoObject* managedInstance);

	};
}
