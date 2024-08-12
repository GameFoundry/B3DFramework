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

	void ScriptCamera::InternalSetFlags(ScriptCamera* thisPtr, CameraFlag flags)
	{
		thisPtr->GetHandle()->SetFlags(flags);
	}

	CameraFlag ScriptCamera::InternalGetFlags(ScriptCamera* thisPtr)
	{
		Flags<CameraFlag> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetFlags();

		CameraFlag __output;
		__output = (CameraFlag)(uint32_t)tmp__output;

		return __output;
	}

	MonoObject* ScriptCamera::InternalGetViewport(ScriptCamera* thisPtr)
	{
		SPtr<Viewport> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetViewport();

		MonoObject* __output;
		__output = ScriptViewport::Create(tmp__output);

		return __output;
	}

	void ScriptCamera::InternalSetHorzFov(ScriptCamera* thisPtr, TRadian<float>* fovy)
	{
		thisPtr->GetHandle()->SetHorzFov(*fovy);
	}

	void ScriptCamera::InternalGetHorzFov(ScriptCamera* thisPtr, TRadian<float>* __output)
	{
		TRadian<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetHorzFov();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetNearClipDistance(ScriptCamera* thisPtr, float nearDist)
	{
		thisPtr->GetHandle()->SetNearClipDistance(nearDist);
	}

	float ScriptCamera::InternalGetNearClipDistance(ScriptCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetNearClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetFarClipDistance(ScriptCamera* thisPtr, float farDist)
	{
		thisPtr->GetHandle()->SetFarClipDistance(farDist);
	}

	float ScriptCamera::InternalGetFarClipDistance(ScriptCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetFarClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetAspectRatio(ScriptCamera* thisPtr, float ratio)
	{
		thisPtr->GetHandle()->SetAspectRatio(ratio);
	}

	float ScriptCamera::InternalGetAspectRatio(ScriptCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAspectRatio();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalGetProjectionMatrixRs(ScriptCamera* thisPtr, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetProjectionMatrixRs();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalGetViewMatrix(ScriptCamera* thisPtr, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetViewMatrix();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetProjectionType(ScriptCamera* thisPtr, ProjectionType pt)
	{
		thisPtr->GetHandle()->SetProjectionType(pt);
	}

	ProjectionType ScriptCamera::InternalGetProjectionType(ScriptCamera* thisPtr)
	{
		ProjectionType tmp__output;
		tmp__output = thisPtr->GetHandle()->GetProjectionType();

		ProjectionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetOrthoWindowHeight(ScriptCamera* thisPtr, float h)
	{
		thisPtr->GetHandle()->SetOrthoWindowHeight(h);
	}

	float ScriptCamera::InternalGetOrthoWindowHeight(ScriptCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetOrthoWindowHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetOrthoWindowWidth(ScriptCamera* thisPtr, float w)
	{
		thisPtr->GetHandle()->SetOrthoWindowWidth(w);
	}

	float ScriptCamera::InternalGetOrthoWindowWidth(ScriptCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetOrthoWindowWidth();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetPriority(ScriptCamera* thisPtr, int32_t priority)
	{
		thisPtr->GetHandle()->SetPriority(priority);
	}

	int32_t ScriptCamera::InternalGetPriority(ScriptCamera* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPriority();

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetLayers(ScriptCamera* thisPtr, uint64_t layers)
	{
		thisPtr->GetHandle()->SetLayers(layers);
	}

	uint64_t ScriptCamera::InternalGetLayers(ScriptCamera* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayers();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetMsaaCount(ScriptCamera* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->SetMsaaCount(count);
	}

	uint32_t ScriptCamera::InternalGetMsaaCount(ScriptCamera* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMsaaCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetRenderSettings(ScriptCamera* thisPtr, MonoObject* settings)
	{
		SPtr<RenderSettings> tmpsettings;
		ScriptRenderSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptRenderSettings::ToNative(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = scriptObjectWrappersettings->GetInternal();
		thisPtr->GetHandle()->SetRenderSettings(tmpsettings);
	}

	MonoObject* ScriptCamera::InternalGetRenderSettings(ScriptCamera* thisPtr)
	{
		SPtr<RenderSettings> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRenderSettings();

		MonoObject* __output;
		__output = ScriptRenderSettings::Create(tmp__output);

		return __output;
	}

	void ScriptCamera::InternalNotifyNeedsRedraw(ScriptCamera* thisPtr)
	{
		thisPtr->GetHandle()->NotifyNeedsRedraw();
	}

	void ScriptCamera::InternalWorldToScreenPoint(ScriptCamera* thisPtr, TVector3<float>* worldPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = thisPtr->GetHandle()->WorldToScreenPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalWorldToNdcPoint(ScriptCamera* thisPtr, TVector3<float>* worldPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->WorldToNdcPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalWorldToViewPoint(ScriptCamera* thisPtr, TVector3<float>* worldPoint, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->WorldToViewPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToWorldPoint(ScriptCamera* thisPtr, TVector2I<int32_t>* screenPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->ScreenToWorldPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToViewPoint(ScriptCamera* thisPtr, TVector2I<int32_t>* screenPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->ScreenToViewPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToNdcPoint(ScriptCamera* thisPtr, TVector2I<int32_t>* screenPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->ScreenToNdcPoint(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToWorldPoint(ScriptCamera* thisPtr, TVector3<float>* viewPoint, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->ViewToWorldPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToScreenPoint(ScriptCamera* thisPtr, TVector3<float>* viewPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = thisPtr->GetHandle()->ViewToScreenPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToNdcPoint(ScriptCamera* thisPtr, TVector3<float>* viewPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->ViewToNdcPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToWorldPoint(ScriptCamera* thisPtr, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->NdcToWorldPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToViewPoint(ScriptCamera* thisPtr, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->NdcToViewPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToScreenPoint(ScriptCamera* thisPtr, TVector2<float>* ndcPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = thisPtr->GetHandle()->NdcToScreenPoint(*ndcPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenPointToRay(ScriptCamera* thisPtr, TVector2I<int32_t>* screenPoint, Ray* __output)
	{
		Ray tmp__output;
		tmp__output = thisPtr->GetHandle()->ScreenPointToRay(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalProjectPoint(ScriptCamera* thisPtr, TVector3<float>* point, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->ProjectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalUnprojectPoint(ScriptCamera* thisPtr, TVector3<float>* point, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = thisPtr->GetHandle()->UnprojectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetMain(ScriptCamera* thisPtr, bool main)
	{
		thisPtr->GetHandle()->SetMain(main);
	}

	bool ScriptCamera::InternalIsMain(ScriptCamera* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->IsMain();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
