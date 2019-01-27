//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCLightProbeVolume.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCLightProbeVolume.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptLightProbeInfo.generated.h"
#include "BsScriptVector3I.generated.h"

namespace bs
{
	ScriptCLightProbeVolume::ScriptCLightProbeVolume(MonoObject* managedInstance, const GameObjectHandle<CLightProbeVolume>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCLightProbeVolume::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_addProbe", (void*)&ScriptCLightProbeVolume::Internal_addProbe);
		metaData.scriptClass->addInternalCall("Internal_setProbePosition", (void*)&ScriptCLightProbeVolume::Internal_setProbePosition);
		metaData.scriptClass->addInternalCall("Internal_getProbePosition", (void*)&ScriptCLightProbeVolume::Internal_getProbePosition);
		metaData.scriptClass->addInternalCall("Internal_removeProbe", (void*)&ScriptCLightProbeVolume::Internal_removeProbe);
		metaData.scriptClass->addInternalCall("Internal_getProbes", (void*)&ScriptCLightProbeVolume::Internal_getProbes);
		metaData.scriptClass->addInternalCall("Internal_renderProbe", (void*)&ScriptCLightProbeVolume::Internal_renderProbe);
		metaData.scriptClass->addInternalCall("Internal_renderProbes", (void*)&ScriptCLightProbeVolume::Internal_renderProbes);
		metaData.scriptClass->addInternalCall("Internal_resize", (void*)&ScriptCLightProbeVolume::Internal_resize);
		metaData.scriptClass->addInternalCall("Internal_clip", (void*)&ScriptCLightProbeVolume::Internal_clip);
		metaData.scriptClass->addInternalCall("Internal_reset", (void*)&ScriptCLightProbeVolume::Internal_reset);
		metaData.scriptClass->addInternalCall("Internal_getGridVolume", (void*)&ScriptCLightProbeVolume::Internal_getGridVolume);
		metaData.scriptClass->addInternalCall("Internal_getCellCount", (void*)&ScriptCLightProbeVolume::Internal_getCellCount);

	}

	uint32_t ScriptCLightProbeVolume::Internal_addProbe(ScriptCLightProbeVolume* thisPtr, Vector3* position)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->addProbe(*position);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLightProbeVolume::Internal_setProbePosition(ScriptCLightProbeVolume* thisPtr, uint32_t handle, Vector3* position)
	{
		thisPtr->getHandle()->setProbePosition(handle, *position);
	}

	void ScriptCLightProbeVolume::Internal_getProbePosition(ScriptCLightProbeVolume* thisPtr, uint32_t handle, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->getProbePosition(handle);

		*__output = tmp__output;
	}

	void ScriptCLightProbeVolume::Internal_removeProbe(ScriptCLightProbeVolume* thisPtr, uint32_t handle)
	{
		thisPtr->getHandle()->removeProbe(handle);
	}

	MonoArray* ScriptCLightProbeVolume::Internal_getProbes(ScriptCLightProbeVolume* thisPtr)
	{
		Vector<LightProbeInfo> vec__output;
		vec__output = thisPtr->getHandle()->getProbes();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::create<ScriptLightProbeInfo>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.set(i, ScriptLightProbeInfo::toInterop(vec__output[i]));
		}
		__output = array__output.getInternal();

		return __output;
	}

	void ScriptCLightProbeVolume::Internal_renderProbe(ScriptCLightProbeVolume* thisPtr, uint32_t handle)
	{
		thisPtr->getHandle()->renderProbe(handle);
	}

	void ScriptCLightProbeVolume::Internal_renderProbes(ScriptCLightProbeVolume* thisPtr)
	{
		thisPtr->getHandle()->renderProbes();
	}

	void ScriptCLightProbeVolume::Internal_resize(ScriptCLightProbeVolume* thisPtr, AABox* volume, Vector3I* cellCount)
	{
		thisPtr->getHandle()->resize(*volume, *cellCount);
	}

	void ScriptCLightProbeVolume::Internal_clip(ScriptCLightProbeVolume* thisPtr)
	{
		thisPtr->getHandle()->clip();
	}

	void ScriptCLightProbeVolume::Internal_reset(ScriptCLightProbeVolume* thisPtr)
	{
		thisPtr->getHandle()->reset();
	}

	void ScriptCLightProbeVolume::Internal_getGridVolume(ScriptCLightProbeVolume* thisPtr, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = thisPtr->getHandle()->getGridVolume();

		*__output = tmp__output;
	}

	void ScriptCLightProbeVolume::Internal_getCellCount(ScriptCLightProbeVolume* thisPtr, Vector3I* __output)
	{
		Vector3I tmp__output;
		tmp__output = thisPtr->getHandle()->getCellCount();

		*__output = tmp__output;
	}

}
