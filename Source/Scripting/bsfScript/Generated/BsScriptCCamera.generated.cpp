//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCamera.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCamera.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptRenderSettings.generated.h"
#include "BsScriptViewport.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector2I.h"

namespace bs
{
	ScriptCCamera::ScriptCCamera(MonoObject* managedInstance, const GameObjectHandle<CCamera>& value)
		: TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCCamera::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetFlags", (void*)&ScriptCCamera::InternalSetFlags);
		metaData.ScriptClass->AddInternalCall("Internal_GetFlags", (void*)&ScriptCCamera::InternalGetFlags);
		metaData.ScriptClass->AddInternalCall("Internal_GetViewport", (void*)&ScriptCCamera::InternalGetViewport);
		metaData.ScriptClass->AddInternalCall("Internal_SetHorzFov", (void*)&ScriptCCamera::InternalSetHorzFov);
		metaData.ScriptClass->AddInternalCall("Internal_GetHorzFov", (void*)&ScriptCCamera::InternalGetHorzFov);
		metaData.ScriptClass->AddInternalCall("Internal_SetNearClipDistance", (void*)&ScriptCCamera::InternalSetNearClipDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetNearClipDistance", (void*)&ScriptCCamera::InternalGetNearClipDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetFarClipDistance", (void*)&ScriptCCamera::InternalSetFarClipDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetFarClipDistance", (void*)&ScriptCCamera::InternalGetFarClipDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetAspectRatio", (void*)&ScriptCCamera::InternalSetAspectRatio);
		metaData.ScriptClass->AddInternalCall("Internal_GetAspectRatio", (void*)&ScriptCCamera::InternalGetAspectRatio);
		metaData.ScriptClass->AddInternalCall("Internal_GetProjectionMatrixRs", (void*)&ScriptCCamera::InternalGetProjectionMatrixRs);
		metaData.ScriptClass->AddInternalCall("Internal_GetViewMatrix", (void*)&ScriptCCamera::InternalGetViewMatrix);
		metaData.ScriptClass->AddInternalCall("Internal_SetProjectionType", (void*)&ScriptCCamera::InternalSetProjectionType);
		metaData.ScriptClass->AddInternalCall("Internal_GetProjectionType", (void*)&ScriptCCamera::InternalGetProjectionType);
		metaData.ScriptClass->AddInternalCall("Internal_SetOrthoWindowHeight", (void*)&ScriptCCamera::InternalSetOrthoWindowHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetOrthoWindowHeight", (void*)&ScriptCCamera::InternalGetOrthoWindowHeight);
		metaData.ScriptClass->AddInternalCall("Internal_SetOrthoWindowWidth", (void*)&ScriptCCamera::InternalSetOrthoWindowWidth);
		metaData.ScriptClass->AddInternalCall("Internal_GetOrthoWindowWidth", (void*)&ScriptCCamera::InternalGetOrthoWindowWidth);
		metaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptCCamera::InternalSetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptCCamera::InternalGetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_SetLayers", (void*)&ScriptCCamera::InternalSetLayers);
		metaData.ScriptClass->AddInternalCall("Internal_GetLayers", (void*)&ScriptCCamera::InternalGetLayers);
		metaData.ScriptClass->AddInternalCall("Internal_SetMsaaCount", (void*)&ScriptCCamera::InternalSetMsaaCount);
		metaData.ScriptClass->AddInternalCall("Internal_GetMsaaCount", (void*)&ScriptCCamera::InternalGetMsaaCount);
		metaData.ScriptClass->AddInternalCall("Internal_SetRenderSettings", (void*)&ScriptCCamera::InternalSetRenderSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetRenderSettings", (void*)&ScriptCCamera::InternalGetRenderSettings);
		metaData.ScriptClass->AddInternalCall("Internal_NotifyNeedsRedraw", (void*)&ScriptCCamera::InternalNotifyNeedsRedraw);
		metaData.ScriptClass->AddInternalCall("Internal_WorldToScreenPoint", (void*)&ScriptCCamera::InternalWorldToScreenPoint);
		metaData.ScriptClass->AddInternalCall("Internal_WorldToNdcPoint", (void*)&ScriptCCamera::InternalWorldToNdcPoint);
		metaData.ScriptClass->AddInternalCall("Internal_WorldToViewPoint", (void*)&ScriptCCamera::InternalWorldToViewPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ScreenToWorldPoint", (void*)&ScriptCCamera::InternalScreenToWorldPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ScreenToViewPoint", (void*)&ScriptCCamera::InternalScreenToViewPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ScreenToNdcPoint", (void*)&ScriptCCamera::InternalScreenToNdcPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ViewToWorldPoint", (void*)&ScriptCCamera::InternalViewToWorldPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ViewToScreenPoint", (void*)&ScriptCCamera::InternalViewToScreenPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ViewToNdcPoint", (void*)&ScriptCCamera::InternalViewToNdcPoint);
		metaData.ScriptClass->AddInternalCall("Internal_NdcToWorldPoint", (void*)&ScriptCCamera::InternalNdcToWorldPoint);
		metaData.ScriptClass->AddInternalCall("Internal_NdcToViewPoint", (void*)&ScriptCCamera::InternalNdcToViewPoint);
		metaData.ScriptClass->AddInternalCall("Internal_NdcToScreenPoint", (void*)&ScriptCCamera::InternalNdcToScreenPoint);
		metaData.ScriptClass->AddInternalCall("Internal_ScreenPointToRay", (void*)&ScriptCCamera::InternalScreenPointToRay);
		metaData.ScriptClass->AddInternalCall("Internal_ProjectPoint", (void*)&ScriptCCamera::InternalProjectPoint);
		metaData.ScriptClass->AddInternalCall("Internal_UnprojectPoint", (void*)&ScriptCCamera::InternalUnprojectPoint);
		metaData.ScriptClass->AddInternalCall("Internal_SetMain", (void*)&ScriptCCamera::InternalSetMain);
		metaData.ScriptClass->AddInternalCall("Internal_IsMain", (void*)&ScriptCCamera::InternalIsMain);
	}

	void ScriptCCamera::InternalSetFlags(ScriptCCamera* thisPtr, CameraFlag flags)
	{
		thisPtr->GetHandle()->SetFlags(flags);
	}

	CameraFlag ScriptCCamera::InternalGetFlags(ScriptCCamera* thisPtr)
	{
		Flags<CameraFlag> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetFlags();

		CameraFlag __output;
		__output = (CameraFlag)(uint32_t)tmp__output;

		return __output;
	}

	MonoObject* ScriptCCamera::InternalGetViewport(ScriptCCamera* thisPtr)
	{
		SPtr<Viewport> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetViewport();

		MonoObject* __output;
		__output = ScriptViewport::Create(tmp__output);

		return __output;
	}

	void ScriptCCamera::InternalSetHorzFov(ScriptCCamera* thisPtr, Radian* fovy)
	{
		thisPtr->GetHandle()->SetHorzFov(*fovy);
	}

	void ScriptCCamera::InternalGetHorzFov(ScriptCCamera* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->GetHandle()->GetHorzFov();

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalSetNearClipDistance(ScriptCCamera* thisPtr, float nearDist)
	{
		thisPtr->GetHandle()->SetNearClipDistance(nearDist);
	}

	float ScriptCCamera::InternalGetNearClipDistance(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetNearClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalSetFarClipDistance(ScriptCCamera* thisPtr, float farDist)
	{
		thisPtr->GetHandle()->SetFarClipDistance(farDist);
	}

	float ScriptCCamera::InternalGetFarClipDistance(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetFarClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalSetAspectRatio(ScriptCCamera* thisPtr, float ratio)
	{
		thisPtr->GetHandle()->SetAspectRatio(ratio);
	}

	float ScriptCCamera::InternalGetAspectRatio(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAspectRatio();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalGetProjectionMatrixRs(ScriptCCamera* thisPtr, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetProjectionMatrixRs();

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalGetViewMatrix(ScriptCCamera* thisPtr, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = thisPtr->GetHandle()->GetViewMatrix();

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalSetProjectionType(ScriptCCamera* thisPtr, ProjectionType pt)
	{
		thisPtr->GetHandle()->SetProjectionType(pt);
	}

	ProjectionType ScriptCCamera::InternalGetProjectionType(ScriptCCamera* thisPtr)
	{
		ProjectionType tmp__output;
		tmp__output = thisPtr->GetHandle()->GetProjectionType();

		ProjectionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalSetOrthoWindowHeight(ScriptCCamera* thisPtr, float h)
	{
		thisPtr->GetHandle()->SetOrthoWindowHeight(h);
	}

	float ScriptCCamera::InternalGetOrthoWindowHeight(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetOrthoWindowHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalSetOrthoWindowWidth(ScriptCCamera* thisPtr, float w)
	{
		thisPtr->GetHandle()->SetOrthoWindowWidth(w);
	}

	float ScriptCCamera::InternalGetOrthoWindowWidth(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetOrthoWindowWidth();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalSetPriority(ScriptCCamera* thisPtr, int32_t priority)
	{
		thisPtr->GetHandle()->SetPriority(priority);
	}

	int32_t ScriptCCamera::InternalGetPriority(ScriptCCamera* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetPriority();

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalSetLayers(ScriptCCamera* thisPtr, uint64_t layers)
	{
		thisPtr->GetHandle()->SetLayers(layers);
	}

	uint64_t ScriptCCamera::InternalGetLayers(ScriptCCamera* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetLayers();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalSetMsaaCount(ScriptCCamera* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->SetMsaaCount(count);
	}

	uint32_t ScriptCCamera::InternalGetMsaaCount(ScriptCCamera* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->GetMsaaCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::InternalSetRenderSettings(ScriptCCamera* thisPtr, MonoObject* settings)
	{
		SPtr<RenderSettings> tmpsettings;
		ScriptRenderSettings* scriptsettings;
		scriptsettings = ScriptRenderSettings::ToNative(settings);
		if(scriptsettings != nullptr)
			tmpsettings = scriptsettings->GetInternal();
		thisPtr->GetHandle()->SetRenderSettings(tmpsettings);
	}

	MonoObject* ScriptCCamera::InternalGetRenderSettings(ScriptCCamera* thisPtr)
	{
		SPtr<RenderSettings> tmp__output;
		tmp__output = thisPtr->GetHandle()->GetRenderSettings();

		MonoObject* __output;
		__output = ScriptRenderSettings::Create(tmp__output);

		return __output;
	}

	void ScriptCCamera::InternalNotifyNeedsRedraw(ScriptCCamera* thisPtr)
	{
		thisPtr->GetHandle()->NotifyNeedsRedraw();
	}

	void ScriptCCamera::InternalWorldToScreenPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->GetHandle()->WorldToScreenPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalWorldToNdcPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->WorldToNdcPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalWorldToViewPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->WorldToViewPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalScreenToWorldPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->ScreenToWorldPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalScreenToViewPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->ScreenToViewPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalScreenToNdcPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->ScreenToNdcPoint(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalViewToWorldPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->ViewToWorldPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalViewToScreenPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->GetHandle()->ViewToScreenPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalViewToNdcPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->ViewToNdcPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalNdcToWorldPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->NdcToWorldPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalNdcToViewPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->NdcToViewPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalNdcToScreenPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->GetHandle()->NdcToScreenPoint(*ndcPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalScreenPointToRay(ScriptCCamera* thisPtr, Vector2I* screenPoint, Ray* __output)
	{
		Ray tmp__output;
		tmp__output = thisPtr->GetHandle()->ScreenPointToRay(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalProjectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->ProjectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalUnprojectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->UnprojectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCCamera::InternalSetMain(ScriptCCamera* thisPtr, bool main)
	{
		thisPtr->GetHandle()->SetMain(main);
	}

	bool ScriptCCamera::InternalIsMain(ScriptCCamera* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->IsMain();

		bool __output;
		__output = tmp__output;

		return __output;
	}
} // namespace bs
