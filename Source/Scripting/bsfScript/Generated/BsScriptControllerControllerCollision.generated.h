//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsCharacterController.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct __ControllerControllerCollisionInterop
	{
		MonoObject* Controller;
		TVector3<float> Position;
		TVector3<float> Normal;
		TVector3<float> MotionDir;
		float MotionAmount;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptControllerControllerCollision : public ScriptObject<ScriptControllerControllerCollision>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ControllerControllerCollision")

		static MonoObject* Box(const __ControllerControllerCollisionInterop& value);
		static __ControllerControllerCollisionInterop Unbox(MonoObject* value);
		static ControllerControllerCollision FromInterop(const __ControllerControllerCollisionInterop& value);
		static __ControllerControllerCollisionInterop ToInterop(const ControllerControllerCollision& value);

	private:
		ScriptControllerControllerCollision(MonoObject* managedInstance);

	};
}
