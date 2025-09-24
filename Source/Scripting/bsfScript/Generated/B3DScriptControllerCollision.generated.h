//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d
{
	struct __ControllerCollisionInterop
	{
		TVector3<float> Position;
		TVector3<float> Normal;
		TVector3<float> MotionDir;
		float MotionAmount;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptControllerCollision : public TScriptTypeDefinition<ScriptControllerCollision>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ControllerCollision")

		static MonoObject* Box(const __ControllerCollisionInterop& value);
		static __ControllerCollisionInterop Unbox(MonoObject* value);
		static ControllerCollision FromInterop(const __ControllerCollisionInterop& value);
		static __ControllerCollisionInterop ToInterop(const ControllerCollision& value);

	private:
		ScriptControllerCollision();

	};
}
