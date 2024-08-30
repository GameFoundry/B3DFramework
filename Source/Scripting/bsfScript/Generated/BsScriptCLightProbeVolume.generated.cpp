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
	ScriptLightProbeVolume::ScriptLightProbeVolume(const GameObjectHandle<CLightProbeVolume>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptLightProbeVolume::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddProbe", (void*)&ScriptLightProbeVolume::InternalAddProbe);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetProbePosition", (void*)&ScriptLightProbeVolume::InternalSetProbePosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetProbePosition", (void*)&ScriptLightProbeVolume::InternalGetProbePosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveProbe", (void*)&ScriptLightProbeVolume::InternalRemoveProbe);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetProbes", (void*)&ScriptLightProbeVolume::InternalGetProbes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RenderProbe", (void*)&ScriptLightProbeVolume::InternalRenderProbe);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RenderProbes", (void*)&ScriptLightProbeVolume::InternalRenderProbes);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Resize", (void*)&ScriptLightProbeVolume::InternalResize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Clip", (void*)&ScriptLightProbeVolume::InternalClip);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Reset", (void*)&ScriptLightProbeVolume::InternalReset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGridVolume", (void*)&ScriptLightProbeVolume::InternalGetGridVolume);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCellCount", (void*)&ScriptLightProbeVolume::InternalGetCellCount);

	}

	MonoObject* ScriptLightProbeVolume::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	uint32_t ScriptLightProbeVolume::InternalAddProbe(ScriptLightProbeVolume* self, TVector3<float>* position)
	{
		uint32_t tmp__output;
		tmp__output = static_cast<CLightProbeVolume*>(self->GetNativeObject())->AddProbe(*position);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLightProbeVolume::InternalSetProbePosition(ScriptLightProbeVolume* self, uint32_t handle, TVector3<float>* position)
	{
		static_cast<CLightProbeVolume*>(self->GetNativeObject())->SetProbePosition(handle, *position);
	}

	void ScriptLightProbeVolume::InternalGetProbePosition(ScriptLightProbeVolume* self, uint32_t handle, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CLightProbeVolume*>(self->GetNativeObject())->GetProbePosition(handle);

		*__output = tmp__output;
	}

	void ScriptLightProbeVolume::InternalRemoveProbe(ScriptLightProbeVolume* self, uint32_t handle)
	{
		static_cast<CLightProbeVolume*>(self->GetNativeObject())->RemoveProbe(handle);
	}

	MonoArray* ScriptLightProbeVolume::InternalGetProbes(ScriptLightProbeVolume* self)
	{
		Vector<LightProbeInfo> nativeArray__output;
		nativeArray__output = static_cast<CLightProbeVolume*>(self->GetNativeObject())->GetProbes();

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

	void ScriptLightProbeVolume::InternalRenderProbe(ScriptLightProbeVolume* self, uint32_t handle)
	{
		static_cast<CLightProbeVolume*>(self->GetNativeObject())->RenderProbe(handle);
	}

	void ScriptLightProbeVolume::InternalRenderProbes(ScriptLightProbeVolume* self)
	{
		static_cast<CLightProbeVolume*>(self->GetNativeObject())->RenderProbes();
	}

	void ScriptLightProbeVolume::InternalResize(ScriptLightProbeVolume* self, AABox* volume, TVector3I<int32_t>* cellCount)
	{
		static_cast<CLightProbeVolume*>(self->GetNativeObject())->Resize(*volume, *cellCount);
	}

	void ScriptLightProbeVolume::InternalClip(ScriptLightProbeVolume* self)
	{
		static_cast<CLightProbeVolume*>(self->GetNativeObject())->Clip();
	}

	void ScriptLightProbeVolume::InternalReset(ScriptLightProbeVolume* self)
	{
		static_cast<CLightProbeVolume*>(self->GetNativeObject())->Reset();
	}

	void ScriptLightProbeVolume::InternalGetGridVolume(ScriptLightProbeVolume* self, AABox* __output)
	{
		AABox tmp__output;
		tmp__output = static_cast<CLightProbeVolume*>(self->GetNativeObject())->GetGridVolume();

		*__output = tmp__output;
	}

	void ScriptLightProbeVolume::InternalGetCellCount(ScriptLightProbeVolume* self, TVector3I<int32_t>* __output)
	{
		TVector3I<int32_t> tmp__output;
		tmp__output = static_cast<CLightProbeVolume*>(self->GetNativeObject())->GetCellCount();

		*__output = tmp__output;
	}

}
