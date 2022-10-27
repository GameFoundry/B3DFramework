//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
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

	class BS_SCR_BE_EXPORT ScriptCollisionData : public ScriptObject<ScriptCollisionData>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "CollisionData")

		static MonoObject* Box(const __CollisionDataInterop& value);
		static __CollisionDataInterop Unbox(MonoObject* value);
		static CollisionData FromInterop(const __CollisionDataInterop& value);
		static __CollisionDataInterop ToInterop(const CollisionData& value);

	private:
		ScriptCollisionData(MonoObject* managedInstance);
	};
} // namespace bs
