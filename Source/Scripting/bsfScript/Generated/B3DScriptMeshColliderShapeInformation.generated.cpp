//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMeshColliderShapeInformation.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsMesh.h"
#include "B3DScriptPhysicsMesh.generated.h"

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
