//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		TVector3<float> Position;
		TVector3<float> Normal;
		TVector3<float> MotionDir;
		float MotionAmount;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptControllerCollision : public ScriptObject<ScriptControllerCollision>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ControllerCollision")

		static MonoObject* Box(const __ControllerCollisionInterop& value);
		static __ControllerCollisionInterop Unbox(MonoObject* value);
		static ControllerCollision FromInterop(const __ControllerCollisionInterop& value);
		static __ControllerCollisionInterop ToInterop(const ControllerCollision& value);

	private:
		ScriptControllerCollision(MonoObject* managedInstance);

	};
}
