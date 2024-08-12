//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCLightProbeVolume.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCLightProbeVolume.h"
#include "BsScriptTVector3I.generated.h"
#include "BsScriptLightProbeInfo.generated.h"
#include "Wrappers/BsScriptVector.h"

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

	uint32_t ScriptLightProbeVolume::InternalAddProbe(ScriptLightProbeVolume* thisPtr, TVector3<float>* position)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->AddProbe(*position);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLightProbeVolume::InternalSetProbePosition(ScriptLightProbeVolume* thisPtr, uint32_t handle, TVector3<float>* position)
	{
		thisPtr->GetHandle()->SetProbePosition(handle, *position);
	}

	void ScriptLightProbeVolume::InternalGetProbePosition(ScriptLightProbeVolume* thisPtr, uint32_t handle, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetProbePosition(handle);

		*__output = tmp__output;
	}

	void ScriptLightProbeVolume::InternalRemoveProbe(ScriptLightProbeVolume* thisPtr, uint32_t handle)
	{
		thisPtr->GetHandle()->RemoveProbe(handle);
	}

	MonoArray* ScriptLightProbeVolume::InternalGetProbes(ScriptLightProbeVolume* thisPtr)
	{
		Vector<LightProbeInfo> nativeArray__output;
		nativeArray__output = thisPtr->GetHandle()->GetProbes();

		MonoArray* __output;
		int elementCount__output = (int)nativeArray__output.size();
		ScriptArray scriptArray__output = ScriptArray::Create<ScriptLightProbeInfo>(elementCount__output);
		for(int elementIndex = 0; elementIndex < elementCount__output; elementIndex++)
		{
			scriptArray__output.Set(elementIndex, ScriptLightProbeInfo::ToInterop(nativeArray__output[elementIndex]));
		}
		__output = scriptArray__output.GetInternal();

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
