//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCMeshCollider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCMeshCollider.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"

namespace b3d
{
	ScriptMeshCollider::ScriptMeshCollider(const GameObjectHandle<MeshCollider>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptMeshCollider::~ScriptMeshCollider()
	{
		UnregisterEvents();
	}

	void ScriptMeshCollider::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMesh", (void*)&ScriptMeshCollider::InternalSetMesh);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMesh", (void*)&ScriptMeshCollider::InternalGetMesh);

	}

	MonoObject* ScriptMeshCollider::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptMeshCollider::InternalSetMesh(ScriptMeshCollider* self, MonoObject* mesh)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<PhysicsMesh> tmpmesh;
		ScriptRRefBase* scriptObjectWrappermesh;
		scriptObjectWrappermesh = ScriptRRefBase::GetScriptObjectWrapper(mesh);
		if(scriptObjectWrappermesh != nullptr)
			tmpmesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrappermesh->GetNativeObject());
		static_cast<MeshCollider*>(self->GetNativeObject())->SetMesh(tmpmesh);
	}

	MonoObject* ScriptMeshCollider::InternalGetMesh(ScriptMeshCollider* self)
	{
		TResourceHandle<PhysicsMesh> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MeshCollider*>(self->GetNativeObject())->GetMesh();

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}
}
