//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysicsQueryHit.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"
#include "Math/BsVector2.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../Foundation/bsfCore/Components/BsCCollider.h"
#include "BsScriptCCollider.generated.h"

namespace bs
{
	ScriptPhysicsQueryHit::ScriptPhysicsQueryHit(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptPhysicsQueryHit::InitRuntimeData()
	{ }

	MonoObject*ScriptPhysicsQueryHit::Box(const __PhysicsQueryHitInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClass(), (void*)&value);
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
		GameObjectHandle<CCollider> tmpCollider;
		ScriptColliderWrapperBase* scriptWrapperObjectCollider;
		scriptWrapperObjectCollider = (ScriptColliderWrapperBase*)ScriptCollider::GetScriptObjectWrapper(value.Collider);
		if(scriptWrapperObjectCollider != nullptr)
			tmpCollider = B3DStaticGameObjectCast<CCollider>(scriptWrapperObjectCollider->GetBaseNativeObjectAsHandle());
		output.Collider = tmpCollider;

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

		return output;
	}

}
