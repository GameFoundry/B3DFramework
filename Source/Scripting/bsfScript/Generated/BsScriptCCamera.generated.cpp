//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCamera.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCamera.h"
#include "BsScriptTVector2I.generated.h"
#include "BsScriptRenderSettings.generated.h"
#include "BsScriptViewport.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptCamera::ScriptCamera(MonoObject* managedInstance, const GameObjectHandle<CCamera>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCamera::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetFlags", (void*)&ScriptCamera::InternalSetFlags);
		metaData.ScriptClass->AddInternalCall("Internal_GetFlags", (void*)&ScriptCamera::InternalGetFlags);
		metaData.ScriptClass->AddInternalCall("Internal_GetViewport", (void*)&ScriptCamera::InternalGetViewport);
		metaData.ScriptClass->AddInternalCall("Internal_SetHorzFov", (void*)&ScriptCamera::InternalSetHorzFov);
		metaData.ScriptClass->AddInternalCall("Internal_GetHorzFov", (void*)&ScriptCamera::InternalGetHorzFov);
		metaData.ScriptClass->AddInternalCall("Internal_SetNearClipDistance", (void*)&ScriptCamera::InternalSetNearClipDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetNearClipDistance", (void*)&ScriptCamera::InternalGetNearClipDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetFarClipDistance", (void*)&ScriptCamera::InternalSetFarClipDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetFarClipDistance", (void*)&ScriptCamera::InternalGetFarClipDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetAspectRatio", (void*)&ScriptCamera::InternalSetAspectRatio);
		metaData.ScriptClass->AddInternalCall("Internal_GetAspectRatio", (void*)&ScriptCamera::InternalGetAspectRatio);
		metaData.ScriptClass->AddInternalCall("Internal_GetProjectionMatrixRs", (void*)&ScriptCamera::InternalGetProjectionMatrixRs);
		metaData.ScriptClass->AddInternalCall("Internal_GetViewMatrix", (void*)&ScriptCamera::InternalGetViewMatrix);
		metaData.ScriptClass->AddInternalCall("Internal_SetProjectionType", (void*)&ScriptCamera::InternalSetProjectionType);
		metaData.ScriptClass->AddInternalCall("Internal_GetProjectionType", (void*)&ScriptCamera::InternalGetProjectionType);
		metaData.ScriptClass->AddInternalCall("Internal_SetOrthoWindowHeight", (void*)&ScriptCamera::InternalSetOrthoWindowHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetOrthoWindowHeight", (void*)&ScriptCamera::InternalGetOrthoWindowHeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetOrthoWindowWidth", (void*)&ScriptCamera::InternalSetOrthoWindowWidth);
		metaData.ScriptClass->AddInternalCall("Internal_GetOrthoWindowWidth", (void*)&ScriptCamera::InternalGetOrthoWindowWidth);
		metaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptCamera::InternalSetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptCamera::InternalGetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayers", (void*)&ScriptCamera::InternalSetLayers);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayers", (void*)&ScriptCamera::InternalGetLayers);
		metaData.ScriptClass->AddInternalCall("Internal_SetMsaaCount", (void*)&ScriptCamera::InternalSetMsaaCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetMsaaCount", (void*)&ScriptCamera::InternalGetMsaaCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetRenderSettings", (void*)&ScriptCamera::InternalSetRenderSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetRenderSettings", (void*)&ScriptCamera::InternalGetRenderSettings);
		metaData.ScriptClass->AddInternalCall("Internal_NotifyNeedsRedraw", (void*)&ScriptCamera::InternalNotifyNeedsRedraw);
		metaData.ScriptClass->AddInternalCall("Internal_WorldToScreenPoint", (void*)&ScriptCamera::InternalWorldToScreenPoint);
		metaData.ScriptClass->AddInternalCall("Internal_WorldToNdcPoint", (void*)&ScriptCamera::InternalWorldToNdcPoint);
		metaData.ScriptClass->AddInternalCall("Internal_WorldToViewPoint", (void*)&ScriptCamera::InternalWorldToViewPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ScreenToWorldPoint", (void*)&ScriptCamera::InternalScreenToWorldPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ScreenToViewPoint", (void*)&ScriptCamera::InternalScreenToViewPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ScreenToNdcPoint", (void*)&ScriptCamera::InternalScreenToNdcPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ViewToWorldPoint", (void*)&ScriptCamera::InternalViewToWorldPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ViewToScreenPoint", (void*)&ScriptCamera::InternalViewToScreenPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ViewToNdcPoint", (void*)&ScriptCamera::InternalViewToNdcPoint);
		metaData.ScriptClass->AddInternalCall("Internal_NdcToWorldPoint", (void*)&ScriptCamera::InternalNdcToWorldPoint);
		metaData.ScriptClass->AddInternalCall("Internal_NdcToViewPoint", (void*)&ScriptCamera::InternalNdcToViewPoint);
		metaData.ScriptClass->AddInternalCall("Internal_NdcToScreenPoint", (void*)&ScriptCamera::InternalNdcToScreenPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ScreenPointToRay", (void*)&ScriptCamera::InternalScreenPointToRay);
		metaData.ScriptClass->AddInternalCall("Internal_ProjectPoint", (void*)&ScriptCamera::InternalProjectPoint);
		metaData.ScriptClass->AddInternalCall("Internal_UnprojectPoint", (void*)&ScriptCamera::InternalUnprojectPoint);
		metaData.ScriptClass->AddInternalCall("Internal_SetMain", (void*)&ScriptCamera::InternalSetMain);
		metaData.ScriptClass->AddInternalCall("Internal_IsMain", (void*)&ScriptCamera::InternalIsMain);

	}

	void ScriptCamera::InternalSetFlags(ScriptCamera* self, CameraFlag flags)
	{
		self->GetHandle()->SetFlags(flags);
	}

	CameraFlag ScriptCamera::InternalGetFlags(ScriptCamera* self)
	{
		Flags<CameraFlag> tmp__output;
		tmp__output = self->GetHandle()->GetFlags();

		CameraFlag __output;
		__output = (CameraFlag)(uint32_t)tmp__output;

		return __output;
	}

	MonoObject* ScriptCamera::InternalGetViewport(ScriptCamera* self)
	{
		SPtr<Viewport> tmp__output;
		tmp__output = self->GetHandle()->GetViewport();

		MonoObject* __output;
		__output = ScriptViewport::Create(tmp__output);

		return __output;
	}

	void ScriptCamera::InternalSetHorzFov(ScriptCamera* self, TRadian<float>* fovy)
	{
		self->GetHandle()->SetHorzFov(*fovy);
	}

	void ScriptCamera::InternalGetHorzFov(ScriptCamera* self, TRadian<float>* __output)
	{
		TRadian<float> tmp__output;
		tmp__output = self->GetHandle()->GetHorzFov();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetNearClipDistance(ScriptCamera* self, float nearDist)
	{
		self->GetHandle()->SetNearClipDistance(nearDist);
	}

	float ScriptCamera::InternalGetNearClipDistance(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetNearClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetFarClipDistance(ScriptCamera* self, float farDist)
	{
		self->GetHandle()->SetFarClipDistance(farDist);
	}

	float ScriptCamera::InternalGetFarClipDistance(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetFarClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetAspectRatio(ScriptCamera* self, float ratio)
	{
		self->GetHandle()->SetAspectRatio(ratio);
	}

	float ScriptCamera::InternalGetAspectRatio(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetAspectRatio();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalGetProjectionMatrixRs(ScriptCamera* self, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = self->GetHandle()->GetProjectionMatrixRs();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalGetViewMatrix(ScriptCamera* self, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = self->GetHandle()->GetViewMatrix();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetProjectionType(ScriptCamera* self, ProjectionType pt)
	{
		self->GetHandle()->SetProjectionType(pt);
	}

	ProjectionType ScriptCamera::InternalGetProjectionType(ScriptCamera* self)
	{
		ProjectionType tmp__output;
		tmp__output = self->GetHandle()->GetProjectionType();

		ProjectionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetOrthoWindowHeight(ScriptCamera* self, float h)
	{
		self->GetHandle()->SetOrthoWindowHeight(h);
	}

	float ScriptCamera::InternalGetOrthoWindowHeight(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetOrthoWindowHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetOrthoWindowWidth(ScriptCamera* self, float w)
	{
		self->GetHandle()->SetOrthoWindowWidth(w);
	}

	float ScriptCamera::InternalGetOrthoWindowWidth(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = self->GetHandle()->GetOrthoWindowWidth();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetPriority(ScriptCamera* self, int32_t priority)
	{
		self->GetHandle()->SetPriority(priority);
	}

	int32_t ScriptCamera::InternalGetPriority(ScriptCamera* self)
	{
		int32_t tmp__output;
		tmp__output = self->GetHandle()->GetPriority();

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetLayers(ScriptCamera* self, uint64_t layers)
	{
		self->GetHandle()->SetLayers(layers);
	}

	uint64_t ScriptCamera::InternalGetLayers(ScriptCamera* self)
	{
		uint64_t tmp__output;
		tmp__output = self->GetHandle()->GetLayers();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetMsaaCount(ScriptCamera* self, uint32_t count)
	{
		self->GetHandle()->SetMsaaCount(count);
	}

	uint32_t ScriptCamera::InternalGetMsaaCount(ScriptCamera* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetHandle()->GetMsaaCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetRenderSettings(ScriptCamera* self, MonoObject* settings)
	{
		SPtr<RenderSettings> tmpsettings;
		ScriptRenderSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptRenderSettings::ToNative(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = scriptObjectWrappersettings->GetInternal();
		self->GetHandle()->SetRenderSettings(tmpsettings);
	}

	MonoObject* ScriptCamera::InternalGetRenderSettings(ScriptCamera* self)
	{
		SPtr<RenderSettings> tmp__output;
		tmp__output = self->GetHandle()->GetRenderSettings();

		MonoObject* __output;
		__output = ScriptRenderSettings::Create(tmp__output);

		return __output;
	}

	void ScriptCamera::InternalNotifyNeedsRedraw(ScriptCamera* self)
	{
		self->GetHandle()->NotifyNeedsRedraw();
	}

	void ScriptCamera::InternalWorldToScreenPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = self->GetHandle()->WorldToScreenPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalWorldToNdcPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetHandle()->WorldToNdcPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalWorldToViewPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->WorldToViewPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToWorldPoint(ScriptCamera* self, TVector2I<int32_t>* screenPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->ScreenToWorldPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToViewPoint(ScriptCamera* self, TVector2I<int32_t>* screenPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->ScreenToViewPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToNdcPoint(ScriptCamera* self, TVector2I<int32_t>* screenPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetHandle()->ScreenToNdcPoint(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToWorldPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->ViewToWorldPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToScreenPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = self->GetHandle()->ViewToScreenPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToNdcPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetHandle()->ViewToNdcPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToWorldPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->NdcToWorldPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToViewPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->NdcToViewPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToScreenPoint(ScriptCamera* self, TVector2<float>* ndcPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = self->GetHandle()->NdcToScreenPoint(*ndcPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenPointToRay(ScriptCamera* self, TVector2I<int32_t>* screenPoint, Ray* __output)
	{
		Ray tmp__output;
		tmp__output = self->GetHandle()->ScreenPointToRay(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalProjectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->ProjectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalUnprojectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = self->GetHandle()->UnprojectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetMain(ScriptCamera* self, bool main)
	{
		self->GetHandle()->SetMain(main);
	}

	bool ScriptCamera::InternalIsMain(ScriptCamera* self)
	{
		bool tmp__output;
		tmp__output = self->GetHandle()->IsMain();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
