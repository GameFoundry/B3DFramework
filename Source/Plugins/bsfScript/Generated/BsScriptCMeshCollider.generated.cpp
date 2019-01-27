//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCMeshCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCMeshCollider.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"

namespace bs
{
	ScriptCMeshCollider::ScriptCMeshCollider(MonoObject* managedInstance, const GameObjectHandle<CMeshCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCMeshCollider::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setMesh", (void*)&ScriptCMeshCollider::Internal_setMesh);
		metaData.scriptClass->addInternalCall("Internal_getMesh", (void*)&ScriptCMeshCollider::Internal_getMesh);

	}

	void ScriptCMeshCollider::Internal_setMesh(ScriptCMeshCollider* thisPtr, MonoObject* mesh)
	{
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::toNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->getHandle());
		thisPtr->getHandle()->setMesh(tmpmesh);
	}

	MonoObject* ScriptCMeshCollider::Internal_getMesh(ScriptCMeshCollider* thisPtr)
	{
		ResourceHandle<PhysicsMesh> tmp__output;
		tmp__output = thisPtr->getHandle()->getMesh();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().getScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->getManagedInstance();
		else
			__output = nullptr;

		return __output;
	}
}
