//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleSphereShapeSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptParticleSphereShapeSettings::ScriptParticleSphereShapeSettings()
	{ }

	MonoObject* ScriptParticleSphereShapeSettings::Box(const ParticleSphereShapeSettings& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	ParticleSphereShapeSettings ScriptParticleSphereShapeSettings::Unbox(MonoObject* value)
	{
		return *(ParticleSphereShapeSettings*)MonoUtil::Unbox(value);
	}

}
