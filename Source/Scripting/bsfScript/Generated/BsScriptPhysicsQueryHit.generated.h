//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"

namespace bs
{
	struct __PhysicsQueryHitInterop
	{
		Vector3 point;
		Vector3 normal;
		Vector2 uv;
		float distance;
		uint32_t triangleIdx;
		uint32_t unmappedTriangleIdx;
		MonoObject* collider;
	};

	class BS_SCR_BE_EXPORT ScriptPhysicsQueryHit : public ScriptObject<ScriptPhysicsQueryHit>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "PhysicsQueryHit")

		static MonoObject* box(const __PhysicsQueryHitInterop& value);
		static __PhysicsQueryHitInterop unbox(MonoObject* value);
		static PhysicsQueryHit fromInterop(const __PhysicsQueryHitInterop& value);
		static __PhysicsQueryHitInterop toInterop(const PhysicsQueryHit& value);

	private:
		ScriptPhysicsQueryHit(MonoObject* managedInstance);

	};
}
