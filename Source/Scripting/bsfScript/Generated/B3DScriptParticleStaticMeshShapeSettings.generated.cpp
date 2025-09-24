//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleStaticMeshShapeSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Mesh/BsMesh.h"
#include "BsScriptMesh.generated.h"

namespace b3d
{
	ScriptParticleStaticMeshShapeSettings::ScriptParticleStaticMeshShapeSettings()
	{ }

	MonoObject* ScriptParticleStaticMeshShapeSettings::Box(const __ParticleStaticMeshShapeSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleStaticMeshShapeSettingsInterop ScriptParticleStaticMeshShapeSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleStaticMeshShapeSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleStaticMeshShapeSettings ScriptParticleStaticMeshShapeSettings::FromInterop(const __ParticleStaticMeshShapeSettingsInterop& value)
	{
		ParticleStaticMeshShapeSettings output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		TResourceHandle<Mesh> tmpMesh;
		ScriptRRefBase* scriptObjectWrapperMesh;
		scriptObjectWrapperMesh = ScriptRRefBase::GetScriptObjectWrapper(value.Mesh);
		if(scriptObjectWrapperMesh != nullptr)
			tmpMesh = B3DStaticResourceCast<Mesh>(scriptObjectWrapperMesh->GetNativeObject());
		output.Mesh = tmpMesh;

		return output;
	}

	__ParticleStaticMeshShapeSettingsInterop ScriptParticleStaticMeshShapeSettings::ToInterop(const ParticleStaticMeshShapeSettings& value)
	{
		__ParticleStaticMeshShapeSettingsInterop output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
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
