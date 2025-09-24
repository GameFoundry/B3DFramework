//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMeshColliderShapeInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsMesh.h"
#include "BsScriptPhysicsMesh.generated.h"

namespace b3d
{
	ScriptMeshColliderShapeInformation::ScriptMeshColliderShapeInformation()
	{ }

	MonoObject* ScriptMeshColliderShapeInformation::Box(const __MeshColliderShapeInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__MeshColliderShapeInformationInterop ScriptMeshColliderShapeInformation::Unbox(MonoObject* value)
	{
		return *(__MeshColliderShapeInformationInterop*)MonoUtil::Unbox(value);
	}

	MeshColliderShapeInformation ScriptMeshColliderShapeInformation::FromInterop(const __MeshColliderShapeInformationInterop& value)
	{
		MeshColliderShapeInformation output;
		TResourceHandle<PhysicsMesh> tmpMesh;
		ScriptRRefBase* scriptObjectWrapperMesh;
		scriptObjectWrapperMesh = ScriptRRefBase::GetScriptObjectWrapper(value.Mesh);
		if(scriptObjectWrapperMesh != nullptr)
			tmpMesh = B3DStaticResourceCast<PhysicsMesh>(scriptObjectWrapperMesh->GetNativeObject());
		output.Mesh = tmpMesh;

		return output;
	}

	__MeshColliderShapeInformationInterop ScriptMeshColliderShapeInformation::ToInterop(const MeshColliderShapeInformation& value)
	{
		__MeshColliderShapeInformationInterop output;
		MonoObject* tmpMesh;
		ScriptRRefBase* scriptWrapperObjectMesh;
		scriptWrapperObjectMesh = ScriptResourceManager::Instance().GetScriptRRef(value.Mesh);
		if(scriptWrapperObjectMesh != nullptr)
			tmpMesh = scriptWrapperObjectMesh->GetScriptObject();
		else
			tmpMesh = nullptr;
		output.Mesh = tmpMesh;

		return output;
	}

}
