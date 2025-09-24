//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"

namespace b3d
{
	struct __PhysicsQueryHitInterop
	{
		TVector3<float> Point;
		TVector3<float> Normal;
		TVector2<float> Uv;
		float Distance;
		uint32_t TriangleIdx;
		uint32_t UnmappedTriangleIdx;
		MonoObject* Collider;
		MonoObject* ColliderShape;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysicsQueryHit : public TScriptTypeDefinition<ScriptPhysicsQueryHit>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PhysicsQueryHit")

		static MonoObject* Box(const __PhysicsQueryHitInterop& value);
		static __PhysicsQueryHitInterop Unbox(MonoObject* value);
		static PhysicsQueryHit FromInterop(const __PhysicsQueryHitInterop& value);
		static __PhysicsQueryHitInterop ToInterop(const PhysicsQueryHit& value);

	private:
		ScriptPhysicsQueryHit();

	};
}
