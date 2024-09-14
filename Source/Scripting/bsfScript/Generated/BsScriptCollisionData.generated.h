//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "BsScriptContactPoint.generated.h"

namespace bs
{
	struct __CollisionDataInterop
	{
		MonoArray* Collider;
		MonoArray* ContactPoints;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptCollisionData : public TScriptStructWrapper<ScriptCollisionData>
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
