//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptLightProbeVolume.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DLightProbeVolume.h"
#include "B3DScriptTVector3I.generated.h"
#include "B3DScriptLightProbeInfo.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTAABox.generated.h"

namespace b3d
{
	ScriptLightProbeVolume::ScriptLightProbeVolume(const TGameObjectHandle<LightProbeVolume>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptLightProbeVolume::~ScriptLightProbeVolume()
	{
		UnregisterEvents();
	}

	void ScriptLightProbeVolume::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddProbe", (void*)&ScriptLightProbeVolume::InternalAddProbe);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveProbe", (void*)&ScriptLightProbeVolume::InternalRemoveProbe);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetProbePosition", (void*)&ScriptLightProbeVolume::InternalSetProbePosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetProbePosition", (void*)&ScriptLightProbeVolume::InternalGetProbePosition);
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
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<LightProbeVolume*>(self->GetNativeObject())->AddProbe(*position);

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLightProbeVolume::InternalRemoveProbe(ScriptLightProbeVolume* self, uint32_t handle)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<LightProbeVolume*>(self->GetNativeObject())->RemoveProbe(handle);
	}

	void ScriptLightProbeVolume::InternalSetProbePosition(ScriptLightProbeVolume* self, uint32_t handle, TVector3<float>* position)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<LightProbeVolume*>(self->GetNativeObject())->SetProbePosition(handle, *position);
	}

	void ScriptLightProbeVolume::InternalGetProbePosition(ScriptLightProbeVolume* self, uint32_t handle, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<LightProbeVolume*>(self->GetNativeObject())->GetProbePosition(handle);

		*__output = tmp__output;
	}

	MonoArray* ScriptLightProbeVolume::InternalGetProbes(ScriptLightProbeVolume* self)
	{
		Vector<LightProbeInfo> nativeArray__output;
		if(!self->IsNativeObjectValid())
			return {};

		nativeArray__output = static_cast<LightProbeVolume*>(self->GetNativeObject())->GetProbes();

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
		if(!self->IsNativeObjectValid())
			return;

		static_cast<LightProbeVolume*>(self->GetNativeObject())->RenderProbe(handle);
	}

	void ScriptLightProbeVolume::InternalRenderProbes(ScriptLightProbeVolume* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<LightProbeVolume*>(self->GetNativeObject())->RenderProbes();
	}

	void ScriptLightProbeVolume::InternalResize(ScriptLightProbeVolume* self, __TAABox_float_Interop* volume, TVector3I<int32_t>* cellCount)
	{
		if(!self->IsNativeObjectValid())
			return;

		TAABox<float> tmpvolume;
		tmpvolume = ScriptAABox::FromInterop(*volume);
		static_cast<LightProbeVolume*>(self->GetNativeObject())->Resize(tmpvolume, *cellCount);
	}

	void ScriptLightProbeVolume::InternalClip(ScriptLightProbeVolume* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<LightProbeVolume*>(self->GetNativeObject())->Clip();
	}

	void ScriptLightProbeVolume::InternalReset(ScriptLightProbeVolume* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<LightProbeVolume*>(self->GetNativeObject())->Reset();
	}

	void ScriptLightProbeVolume::InternalGetGridVolume(ScriptLightProbeVolume* self, __TAABox_float_Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TAABox<float> tmp__output;
		tmp__output = static_cast<LightProbeVolume*>(self->GetNativeObject())->GetGridVolume();

		__TAABox_float_Interop interop__output;
		interop__output = ScriptAABox::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptAABox::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptLightProbeVolume::InternalGetCellCount(ScriptLightProbeVolume* self, TVector3I<int32_t>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3I<int32_t> tmp__output;
		tmp__output = static_cast<LightProbeVolume*>(self->GetNativeObject())->GetCellCount();

		*__output = tmp__output;
	}

}
