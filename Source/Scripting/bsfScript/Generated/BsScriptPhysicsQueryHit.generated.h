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
		Vector3 Point;
		Vector3 Normal;
		Vector2 Uv;
		float Distance;
		uint32_t TriangleIdx;
		uint32_t UnmappedTriangleIdx;
		MonoObject* Collider;
	};

	class BS_SCR_BE_EXPORT ScriptPhysicsQueryHit : public ScriptObject<ScriptPhysicsQueryHit>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "PhysicsQueryHit")

		static MonoObject* Box(const __PhysicsQueryHitInterop& value);
		static __PhysicsQueryHitInterop Unbox(MonoObject* value);
		static PhysicsQueryHit FromInterop(const __PhysicsQueryHitInterop& value);
		static __PhysicsQueryHitInterop ToInterop(const PhysicsQueryHit& value);

	private:
		ScriptPhysicsQueryHit(MonoObject* managedInstance);
	};
} // namespace bs
