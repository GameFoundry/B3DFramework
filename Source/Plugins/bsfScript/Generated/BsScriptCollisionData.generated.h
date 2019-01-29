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
		MonoArray* collider;
		MonoArray* contactPoints;
	};

	class BS_SCR_BE_EXPORT ScriptCollisionData : public ScriptObject<ScriptCollisionData>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "CollisionData")

		static MonoObject* box(const __CollisionDataInterop& value);
		static __CollisionDataInterop unbox(MonoObject* value);
		static CollisionData fromInterop(const __CollisionDataInterop& value);
		static __CollisionDataInterop toInterop(const CollisionData& value);

	private:
		ScriptCollisionData(MonoObject* managedInstance);

	};
}
