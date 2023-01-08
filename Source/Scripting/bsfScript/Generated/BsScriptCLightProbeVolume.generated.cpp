//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCLightProbeVolume.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCLightProbeVolume.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptTVector3I.generated.h"
#include "BsScriptLightProbeInfo.generated.h"

namespace bs
{
	ScriptLightProbeVolume::ScriptLightProbeVolume(MonoObject* managedInstance, const GameObjectHandle<CLightProbeVolume>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptLightProbeVolume::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_AddProbe", (void*)&ScriptLightProbeVolume::InternalAddProbe);
		metaData.ScriptClass->AddInternalCall("Internal_SetProbePosition", (void*)&ScriptLightProbeVolume::InternalSetProbePosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetProbePosition", (void*)&ScriptLightProbeVolume::InternalGetProbePosition);
		metaData.ScriptClass->AddInternalCall("Internal_RemoveProbe", (void*)&ScriptLightProbeVolume::InternalRemoveProbe);
		metaData.ScriptClass->AddInternalCall("Internal_GetProbes", (void*)&ScriptLightProbeVolume::InternalGetProbes);
		metaData.ScriptClass->AddInternalCall("Internal_RenderProbe", (void*)&ScriptLightProbeVolume::InternalRenderProbe);
		metaData.ScriptClass->AddInternalCall("Internal_RenderProbes", (void*)&ScriptLightProbeVolume::InternalRenderProbes);
		metaData.ScriptClass->AddInternalCall("Internal_Resize", (void*)&ScriptLightProbeVolume::InternalResize);
		metaData.ScriptClass->AddInternalCall("Internal_Clip", (void*)&ScriptLightProbeVolume::InternalClip);
		metaData.ScriptClass->AddInternalCall("Internal_Reset", (void*)&ScriptLightProbeVolume::InternalReset);
		metaData.ScriptClass->AddInternalCall("Internal_GetGridVolume", (void*)&ScriptLightProbeVolume::InternalGetGridVolume);
		metaData.ScriptClass->AddInternalCall("Internal_GetCellCount", (void*)&ScriptLightProbeVolume::InternalGetCellCount);

	}

	uint32_t ScriptLightProbeVolume::InternalAddProbe(ScriptLightProbeVolume* thisPtr, Vector3* position)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->AddProbe(*position);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLightProbeVolume::InternalSetProbePosition(ScriptLightProbeVolume* thisPtr, uint32_t handle, Vector3* position)
	{
		thisPtr->GetHandle()->SetProbePosition(handle, *position);
	}

	void ScriptLightProbeVolume::InternalGetProbePosition(ScriptLightProbeVolume* thisPtr, uint32_t handle, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetProbePosition(handle);

		*__output = tmp__output;
	}

	void ScriptLightProbeVolume::InternalRemoveProbe(ScriptLightProbeVolume* thisPtr, uint32_t handle)
	{
		thisPtr->GetHandle()->RemoveProbe(handle);
	}

	MonoArray* ScriptLightProbeVolume::InternalGetProbes(ScriptLightProbeVolume* thisPtr)
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

	void ScriptLightProbeVolume::InternalRenderProbe(ScriptLightProbeVolume* thisPtr, uint32_t handle)
	{
		thisPtr->GetHandle()->RenderProbe(handle);
	}

	void ScriptLightProbeVolume::InternalRenderProbes(ScriptLightProbeVolume* thisPtr)
	{
		thisPtr->GetHandle()->RenderProbes();
	}

	void ScriptLightProbeVolume::InternalResize(ScriptLightProbeVolume* thisPtr, AABox* volume, TVector3I<int32_t>* cellCount)
	{
		thisPtr->GetHandle()->Resize(*volume, *cellCount);
	}

	void ScriptLightProbeVolume::InternalClip(ScriptLightProbeVolume* thisPtr)
	{
		thisPtr->GetHandle()->Clip();
	}

	void ScriptLightProbeVolume::InternalReset(ScriptLightProbeVolume* thisPtr)
	{
		thisPtr->GetHandle()->Reset();
	}

	void ScriptLightProbeVolume::InternalGetGridVolume(ScriptLightProbeVolume* thisPtr, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = thisPtr->GetHandle()->GetGridVolume();

		*__output = tmp__output;
	}

	void ScriptLightProbeVolume::InternalGetCellCount(ScriptLightProbeVolume* thisPtr, TVector3I<int32_t>* __output)
	{
		TVector3I<int32_t> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCellCount();

		*__output = tmp__output;
	}

}
