//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	ScriptMeshCollider::ScriptMeshCollider(MonoObject* managedInstance, const GameObjectHandle<CMeshCollider>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptMeshCollider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetMesh", (void*)&ScriptMeshCollider::InternalSetMesh);
		metaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptMeshCollider::InternalGetMesh);

	}

	void ScriptMeshCollider::InternalSetMesh(ScriptMeshCollider* thisPtr, MonoObject* mesh)
	{
		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::ToNative(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetHandle());
		thisPtr->GetHandle()->SetMesh(tmpmesh);
	}

	MonoObject* ScriptMeshCollider::InternalGetMesh(ScriptMeshCollider* thisPtr)
	{
		TResourceHandle<PhysicsMesh> tmp__output;
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
}
