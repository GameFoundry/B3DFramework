//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCMeshCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCMeshCollider.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"

namespace bs
{
	ScriptCMeshCollider::ScriptCMeshCollider(MonoObject* managedInstance, const GameObjectHandle<CMeshCollider>& value)
		: TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCMeshCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetMesh", (void*)&ScriptCMeshCollider::InternalSetMesh);
		metaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptCMeshCollider::InternalGetMesh);
	}

	void ScriptCMeshCollider::InternalSetMesh(ScriptCMeshCollider* thisPtr, MonoObject* mesh)
	{
		ResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptmesh;
		scriptmesh = ScriptRRefBase::ToNative(mesh);
		if(scriptmesh != nullptr)
			tmpmesh = static_resource_cast<PhysicsMesh>(scriptmesh->GetHandle());
		thisPtr->GetHandle()->SetMesh(tmpmesh);
	}

	MonoObject* ScriptCMeshCollider::InternalGetMesh(ScriptCMeshCollider* thisPtr)
	{
		ResourceHandle<PhysicsMesh> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMesh();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}
} // namespace bs
