//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DCharacterController.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d
{
	struct __ControllerColliderCollisionInterop
	{
		MonoObject* Collider;
		uint32_t TriangleIndex;
		TVector3<float> Position;
		TVector3<float> Normal;
		TVector3<float> MotionDir;
		float MotionAmount;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptControllerColliderCollision : public TScriptTypeDefinition<ScriptControllerColliderCollision>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ControllerColliderCollision")

		static MonoObject* Box(const __ControllerColliderCollisionInterop& value);
		static __ControllerColliderCollisionInterop Unbox(MonoObject* value);
		static ControllerColliderCollision FromInterop(const __ControllerColliderCollisionInterop& value);
		static __ControllerColliderCollisionInterop ToInterop(const ControllerColliderCollision& value);

	private:
		ScriptControllerColliderCollision();

	};
}
