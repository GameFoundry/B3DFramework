//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsCommon.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsCommon.h"
#include "B3DScriptContactPoint.generated.h"

namespace b3d
{
	struct __CollisionDataInterop
	{
		MonoArray* Collider;
		MonoArray* ColliderShapes;
		MonoArray* ContactPoints;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptCollisionData : public TScriptTypeDefinition<ScriptCollisionData>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CollisionData")

		static MonoObject* Box(const __CollisionDataInterop& value);
		static __CollisionDataInterop Unbox(MonoObject* value);
		static CollisionData FromInterop(const __CollisionDataInterop& value);
		static __CollisionDataInterop ToInterop(const CollisionData& value);

	private:
		ScriptCollisionData();

	};
}
