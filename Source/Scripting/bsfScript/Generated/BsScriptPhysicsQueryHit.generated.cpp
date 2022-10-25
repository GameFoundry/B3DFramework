//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPhysicsQueryHit.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"
#include "Math/BsVector2.h"
#include "Wrappers/BsScriptVector.h"
#include "../../../Foundation/bsfCore/Components/BsCCollider.h"
#include "BsScriptCCollider.generated.h"

namespace bs
{
	ScriptPhysicsQueryHit::ScriptPhysicsQueryHit(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptPhysicsQueryHit::InitRuntimeData()
	{}

	MonoObject* ScriptPhysicsQueryHit::Box(const __PhysicsQueryHitInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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
		ScriptCColliderBase* scriptCollider;
		scriptCollider = (ScriptCColliderBase*)ScriptCCollider::ToNative(value.Collider);
		if(scriptCollider != nullptr)
			tmpCollider = static_object_cast<CCollider>(scriptCollider->GetComponent());
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
		ScriptComponentBase* scriptCollider = nullptr;
		if(value.Collider)
			scriptCollider = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(value.Collider));
		if(scriptCollider != nullptr)
			tmpCollider = scriptCollider->GetManagedInstance();
		else
			tmpCollider = nullptr;
		output.Collider = tmpCollider;

		return output;
	}

} // namespace bs
