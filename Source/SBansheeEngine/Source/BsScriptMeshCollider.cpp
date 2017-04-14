//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptMeshCollider.h"
#include "BsMeshCollider.h"
#include "BsScriptPhysicsMesh.h"
#include "BsScriptResourceManager.h"

namespace bs
{
	ScriptMeshCollider::ScriptMeshCollider(MonoObject* instance, const SPtr<Collider>& collider)
		:TScriptCollider(instance, collider)
	{

	}

	void ScriptMeshCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptMeshCollider::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_SetMesh", &ScriptMeshCollider::internal_SetMesh);
		metaData.scriptClass->addInternalCall("Internal_GetMesh", &ScriptMeshCollider::internal_GetMesh);
	}

	MeshCollider* ScriptMeshCollider::getMeshCollider() const
	{
		return static_cast<MeshCollider*>(mCollider.get());
	}

	void ScriptMeshCollider::internal_CreateInstance(MonoObject* instance)
	{
		SPtr<MeshCollider> collider = MeshCollider::create();
		collider->_setOwner(PhysicsOwnerType::Script, instance);

		new (bs_alloc<ScriptMeshCollider>()) ScriptMeshCollider(instance, collider);
	}

	void ScriptMeshCollider::internal_SetMesh(ScriptMeshCollider* thisPtr, ScriptPhysicsMesh* scriptMesh)
	{
		HPhysicsMesh mesh;
		if (scriptMesh != nullptr)
			mesh = scriptMesh->getHandle();

		thisPtr->getMeshCollider()->setMesh(mesh);
	}

	MonoObject* ScriptMeshCollider::internal_GetMesh(ScriptMeshCollider* thisPtr)
	{
		HPhysicsMesh mesh = thisPtr->getMeshCollider()->getMesh();
		if (mesh == nullptr)
			return nullptr;

		ScriptResourceBase* scriptMesh = ScriptResourceManager::instance().getScriptResource(mesh, true);
		return scriptMesh->getManagedInstance();
	}
}