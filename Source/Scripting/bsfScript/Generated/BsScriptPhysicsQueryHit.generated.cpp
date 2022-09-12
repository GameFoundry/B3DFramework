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
		:ScriptObject(managedInstance)
	{ }

	void ScriptPhysicsQueryHit::InitRuntimeData()
	{ }

	MonoObject*ScriptPhysicsQueryHit::box(const __PhysicsQueryHitInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__PhysicsQueryHitInterop ScriptPhysicsQueryHit::Unbox(MonoObject* value)
	{
		return *(__PhysicsQueryHitInterop*)MonoUtil::unbox(value);
	}

	PhysicsQueryHit ScriptPhysicsQueryHit::FromInterop(const __PhysicsQueryHitInterop& value)
	{
		PhysicsQueryHit output;
		output.point = value.point;
		output.normal = value.normal;
		output.uv = value.uv;
		output.distance = value.distance;
		output.triangleIdx = value.triangleIdx;
		output.unmappedTriangleIdx = value.unmappedTriangleIdx;
		GameObjectHandle<CCollider> tmpcollider;
		ScriptCColliderBase* scriptcollider;
		scriptcollider = (ScriptCColliderBase*)ScriptCCollider::toNative(value.collider);
		if(scriptcollider != nullptr)
			tmpcollider = static_object_cast<CCollider>(scriptcollider->GetComponent());
		output.collider = tmpcollider;

		return output;
	}

	__PhysicsQueryHitInterop ScriptPhysicsQueryHit::ToInterop(const PhysicsQueryHit& value)
	{
		__PhysicsQueryHitInterop output;
		output.point = value.point;
		output.normal = value.normal;
		output.uv = value.uv;
		output.distance = value.distance;
		output.triangleIdx = value.triangleIdx;
		output.unmappedTriangleIdx = value.unmappedTriangleIdx;
		ScriptComponentBase* scriptcollider = nullptr;
		if(value.collider)
			scriptcollider = ScriptGameObjectManager::instance().GetBuiltinScriptComponent(static_object_cast<Component>(value.collider));
		MonoObject* tmpcollider;
		if(scriptcollider != nullptr)
			tmpcollider = scriptcollider->GetManagedInstance();
		else
			tmpcollider = nullptr;
		output.collider = tmpcollider;

		return output;
	}

}
