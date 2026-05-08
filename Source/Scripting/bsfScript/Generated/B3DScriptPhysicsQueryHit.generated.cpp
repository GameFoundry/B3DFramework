//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptPhysicsQueryHit.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Components/B3DCollider.h"
#include "B3DScriptCollider.generated.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"
#include "../../../Engine/Core/Physics/B3DColliderShape.h"
#include "B3DScriptColliderShape.generated.h"
#include "../../../Engine/Utility/Math/B3DVector2.h"
#include "B3DScriptTVector2.generated.h"

namespace b3d
{
	ScriptPhysicsQueryHit::ScriptPhysicsQueryHit()
	{ }

	MonoObject* ScriptPhysicsQueryHit::Box(const __PhysicsQueryHitInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__PhysicsQueryHitInterop ScriptPhysicsQueryHit::Unbox(MonoObject* value)
	{
		return *(__PhysicsQueryHitInterop*)MonoUtil::Unbox(value);
	}

	PhysicsQueryHit ScriptPhysicsQueryHit::FromInterop(const __PhysicsQueryHitInterop& value)
	{
		PhysicsQueryHit output;
		output.Point = value.Point;
		output.Normal = value.Normal;
		output.Uv = value.Uv;
		output.Distance = value.Distance;
		output.TriangleIdx = value.TriangleIdx;
		output.UnmappedTriangleIdx = value.UnmappedTriangleIdx;
		TGameObjectHandle<Collider> tmpCollider;
		ScriptColliderWrapperBase* scriptObjectWrapperCollider;
		scriptObjectWrapperCollider = (ScriptColliderWrapperBase*)ScriptCollider::GetScriptObjectWrapper(value.Collider);
		if(scriptObjectWrapperCollider != nullptr)
			tmpCollider = B3DStaticGameObjectCast<Collider>(scriptObjectWrapperCollider->GetBaseNativeObjectAsHandle());
		output.Collider = tmpCollider;
		TShared<ColliderShape> tmpColliderShape;
		ScriptColliderShape* scriptObjectWrapperColliderShape;
		scriptObjectWrapperColliderShape = ScriptColliderShape::GetScriptObjectWrapper(value.ColliderShape);
		if(scriptObjectWrapperColliderShape != nullptr)
			tmpColliderShape = std::static_pointer_cast<ColliderShape>(scriptObjectWrapperColliderShape->GetBaseNativeObjectAsShared());
		output.ColliderShape = tmpColliderShape;

		return output;
	}

	__PhysicsQueryHitInterop ScriptPhysicsQueryHit::ToInterop(const PhysicsQueryHit& value)
	{
		__PhysicsQueryHitInterop output;
		output.Point = value.Point;
		output.Normal = value.Normal;
		output.Uv = value.Uv;
		output.Distance = value.Distance;
		output.TriangleIdx = value.TriangleIdx;
		output.UnmappedTriangleIdx = value.UnmappedTriangleIdx;
		MonoObject* tmpCollider;
		MonoObject* temptmpCollider = nullptr;
		if(value.Collider)
			temptmpCollider = ScriptComponent::GetOrCreateScriptObject(value.Collider);
		tmpCollider = temptmpCollider;
		output.Collider = tmpCollider;
		MonoObject* tmpColliderShape;
		tmpColliderShape = ScriptColliderShape::GetOrCreateScriptObject(value.ColliderShape);
		output.ColliderShape = tmpColliderShape;

		return output;
	}

}
