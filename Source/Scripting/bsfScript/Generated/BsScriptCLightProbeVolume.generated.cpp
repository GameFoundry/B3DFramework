//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCLightProbeVolume.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCLightProbeVolume.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptLightProbeInfo.generated.h"
#include "BsScriptVector3I.generated.h"

namespace bs
{
	ScriptCLightProbeVolume::ScriptCLightProbeVolume(MonoObject* managedInstance, const GameObjectHandle<CLightProbeVolume>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCLightProbeVolume::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_AddProbe", (void*)&ScriptCLightProbeVolume::InternalAddProbe);
		metaData.ScriptClass->AddInternalCall("Internal_SetProbePosition", (void*)&ScriptCLightProbeVolume::InternalSetProbePosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetProbePosition", (void*)&ScriptCLightProbeVolume::InternalGetProbePosition);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveProbe", (void*)&ScriptCLightProbeVolume::InternalRemoveProbe);
		metaData.ScriptClass->AddInternalCall("Internal_GetProbes", (void*)&ScriptCLightProbeVolume::InternalGetProbes);
		metaData.ScriptClass->AddInternalCall("Internal_RenderProbe", (void*)&ScriptCLightProbeVolume::InternalRenderProbe);
		metaData.ScriptClass->AddInternalCall("Internal_RenderProbes", (void*)&ScriptCLightProbeVolume::InternalRenderProbes);
		metaData.ScriptClass->AddInternalCall("Internal_Resize", (void*)&ScriptCLightProbeVolume::InternalResize);
		metaData.ScriptClass->AddInternalCall("Internal_Clip", (void*)&ScriptCLightProbeVolume::InternalClip);
		metaData.ScriptClass->AddInternalCall("Internal_Reset", (void*)&ScriptCLightProbeVolume::InternalReset);
		metaData.ScriptClass->AddInternalCall("Internal_GetGridVolume", (void*)&ScriptCLightProbeVolume::InternalGetGridVolume);
		metaData.ScriptClass->AddInternalCall("Internal_GetCellCount", (void*)&ScriptCLightProbeVolume::InternalGetCellCount);

	}

	uint32_t ScriptCLightProbeVolume::InternalAddProbe(ScriptCLightProbeVolume* thisPtr, Vector3* position)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->AddProbe(*position);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLightProbeVolume::InternalSetProbePosition(ScriptCLightProbeVolume* thisPtr, uint32_t handle, Vector3* position)
	{
		thisPtr->GetHandle()->SetProbePosition(handle, *position);
	}

	void ScriptCLightProbeVolume::InternalGetProbePosition(ScriptCLightProbeVolume* thisPtr, uint32_t handle, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetProbePosition(handle);

		*__output = tmp__output;
	}

	void ScriptCLightProbeVolume::InternalRemoveProbe(ScriptCLightProbeVolume* thisPtr, uint32_t handle)
	{
		thisPtr->GetHandle()->RemoveProbe(handle);
	}

	MonoArray* ScriptCLightProbeVolume::InternalGetProbes(ScriptCLightProbeVolume* thisPtr)
	{
		Vector<LightProbeInfo> vec__output;
		vec__output = thisPtr->GetHandle()->GetProbes();

		MonoArray* __output;
		int arraySize__output = (int)vec__output.size();
		ScriptArray array__output = ScriptArray::Create<ScriptLightProbeInfo>(arraySize__output);
		for(int i = 0; i < arraySize__output; i++)
		{
			array__output.Set(i, ScriptLightProbeInfo::ToInterop(vec__output[i]));
		}
		__output = array__output.GetInternal();

		return __output;
	}

	void ScriptCLightProbeVolume::InternalRenderProbe(ScriptCLightProbeVolume* thisPtr, uint32_t handle)
	{
		thisPtr->GetHandle()->RenderProbe(handle);
	}

	void ScriptCLightProbeVolume::InternalRenderProbes(ScriptCLightProbeVolume* thisPtr)
	{
		thisPtr->GetHandle()->RenderProbes();
	}

	void ScriptCLightProbeVolume::InternalResize(ScriptCLightProbeVolume* thisPtr, AABox* volume, Vector3I* cellCount)
	{
		thisPtr->GetHandle()->Resize(*volume, *cellCount);
	}

	void ScriptCLightProbeVolume::InternalClip(ScriptCLightProbeVolume* thisPtr)
	{
		thisPtr->GetHandle()->Clip();
	}

	void ScriptCLightProbeVolume::InternalReset(ScriptCLightProbeVolume* thisPtr)
	{
		thisPtr->GetHandle()->Reset();
	}

	void ScriptCLightProbeVolume::InternalGetGridVolume(ScriptCLightProbeVolume* thisPtr, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = thisPtr->GetHandle()->GetGridVolume();

		*__output = tmp__output;
	}

	void ScriptCLightProbeVolume::InternalGetCellCount(ScriptCLightProbeVolume* thisPtr, Vector3I* __output)
	{
		Vector3I tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCellCount();

		*__output = tmp__output;
	}

}
