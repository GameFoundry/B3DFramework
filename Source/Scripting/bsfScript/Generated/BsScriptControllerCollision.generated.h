//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsCharacterController.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct __ControllerCollisionInterop
	{
		Vector3 Position;
		Vector3 Normal;
		Vector3 MotionDir;
		float MotionAmount;
	};

	class BS_SCR_BE_EXPORT ScriptControllerCollision : public ScriptObject<ScriptControllerCollision>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ControllerCollision")

		static MonoObject* Box(const __ControllerCollisionInterop& value);
		static __ControllerCollisionInterop Unbox(MonoObject* value);
		static ControllerCollision FromInterop(const __ControllerCollisionInterop& value);
		static __ControllerCollisionInterop ToInterop(const ControllerCollision& value);

	private:
		ScriptControllerCollision(MonoObject* managedInstance);
	};
} // namespace bs
