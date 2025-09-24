//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d
{
	struct __ControllerControllerCollisionInterop
	{
		MonoObject* Controller;
		TVector3<float> Position;
		TVector3<float> Normal;
		TVector3<float> MotionDir;
		float MotionAmount;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptControllerControllerCollision : public TScriptTypeDefinition<ScriptControllerControllerCollision>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ControllerControllerCollision")

		static MonoObject* Box(const __ControllerControllerCollisionInterop& value);
		static __ControllerControllerCollisionInterop Unbox(MonoObject* value);
		static ControllerControllerCollision FromInterop(const __ControllerControllerCollisionInterop& value);
		static __ControllerControllerCollisionInterop ToInterop(const ControllerControllerCollision& value);

	private:
		ScriptControllerControllerCollision();

	};
}
