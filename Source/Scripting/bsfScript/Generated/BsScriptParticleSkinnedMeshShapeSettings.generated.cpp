//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleSkinnedMeshShapeSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsRenderable.h"
#include "BsScriptRenderable.generated.h"

namespace b3d
{
	ScriptParticleSkinnedMeshShapeSettings::ScriptParticleSkinnedMeshShapeSettings()
	{ }

	MonoObject* ScriptParticleSkinnedMeshShapeSettings::Box(const __ParticleSkinnedMeshShapeSettingsInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ParticleSkinnedMeshShapeSettingsInterop ScriptParticleSkinnedMeshShapeSettings::Unbox(MonoObject* value)
	{
		return *(__ParticleSkinnedMeshShapeSettingsInterop*)MonoUtil::Unbox(value);
	}

	ParticleSkinnedMeshShapeSettings ScriptParticleSkinnedMeshShapeSettings::FromInterop(const __ParticleSkinnedMeshShapeSettingsInterop& value)
	{
		ParticleSkinnedMeshShapeSettings output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		GameObjectHandle<Renderable> tmpRenderable;
		ScriptRenderable* scriptObjectWrapperRenderable;
		scriptObjectWrapperRenderable = ScriptRenderable::GetScriptObjectWrapper(value.Renderable);
		if(scriptObjectWrapperRenderable != nullptr)
			tmpRenderable = B3DStaticGameObjectCast<Renderable>(scriptObjectWrapperRenderable->GetBaseNativeObjectAsHandle());
		output.Renderable = tmpRenderable;

		return output;
	}

	__ParticleSkinnedMeshShapeSettingsInterop ScriptParticleSkinnedMeshShapeSettings::ToInterop(const ParticleSkinnedMeshShapeSettings& value)
	{
		__ParticleSkinnedMeshShapeSettingsInterop output;
		output.Type = value.Type;
		output.Sequential = value.Sequential;
		MonoObject* tmpRenderable;
		MonoObject* temptmpRenderable = nullptr;
		if(value.Renderable)
			temptmpRenderable = ScriptComponent::GetOrCreateScriptObject(value.Renderable);
		tmpRenderable = temptmpRenderable;
		output.Renderable = tmpRenderable;

		return output;
	}

}
