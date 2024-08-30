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
	ScriptCamera::ScriptCamera(const GameObjectHandle<CCamera>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptCamera::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlags", (void*)&ScriptCamera::InternalSetFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFlags", (void*)&ScriptCamera::InternalGetFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetViewport", (void*)&ScriptCamera::InternalGetViewport);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHorzFov", (void*)&ScriptCamera::InternalSetHorzFov);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHorzFov", (void*)&ScriptCamera::InternalGetHorzFov);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNearClipDistance", (void*)&ScriptCamera::InternalSetNearClipDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNearClipDistance", (void*)&ScriptCamera::InternalGetNearClipDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFarClipDistance", (void*)&ScriptCamera::InternalSetFarClipDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFarClipDistance", (void*)&ScriptCamera::InternalGetFarClipDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAspectRatio", (void*)&ScriptCamera::InternalSetAspectRatio);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAspectRatio", (void*)&ScriptCamera::InternalGetAspectRatio);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetProjectionMatrixRs", (void*)&ScriptCamera::InternalGetProjectionMatrixRs);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetViewMatrix", (void*)&ScriptCamera::InternalGetViewMatrix);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetProjectionType", (void*)&ScriptCamera::InternalSetProjectionType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetProjectionType", (void*)&ScriptCamera::InternalGetProjectionType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOrthoWindowHeight", (void*)&ScriptCamera::InternalSetOrthoWindowHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrthoWindowHeight", (void*)&ScriptCamera::InternalGetOrthoWindowHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOrthoWindowWidth", (void*)&ScriptCamera::InternalSetOrthoWindowWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrthoWindowWidth", (void*)&ScriptCamera::InternalGetOrthoWindowWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptCamera::InternalSetPriority);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptCamera::InternalGetPriority);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayers", (void*)&ScriptCamera::InternalSetLayers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayers", (void*)&ScriptCamera::InternalGetLayers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMsaaCount", (void*)&ScriptCamera::InternalSetMsaaCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMsaaCount", (void*)&ScriptCamera::InternalGetMsaaCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRenderSettings", (void*)&ScriptCamera::InternalSetRenderSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRenderSettings", (void*)&ScriptCamera::InternalGetRenderSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_NotifyNeedsRedraw", (void*)&ScriptCamera::InternalNotifyNeedsRedraw);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WorldToScreenPoint", (void*)&ScriptCamera::InternalWorldToScreenPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WorldToNdcPoint", (void*)&ScriptCamera::InternalWorldToNdcPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WorldToViewPoint", (void*)&ScriptCamera::InternalWorldToViewPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenToWorldPoint", (void*)&ScriptCamera::InternalScreenToWorldPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenToViewPoint", (void*)&ScriptCamera::InternalScreenToViewPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenToNdcPoint", (void*)&ScriptCamera::InternalScreenToNdcPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ViewToWorldPoint", (void*)&ScriptCamera::InternalViewToWorldPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ViewToScreenPoint", (void*)&ScriptCamera::InternalViewToScreenPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ViewToNdcPoint", (void*)&ScriptCamera::InternalViewToNdcPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_NdcToWorldPoint", (void*)&ScriptCamera::InternalNdcToWorldPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_NdcToViewPoint", (void*)&ScriptCamera::InternalNdcToViewPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_NdcToScreenPoint", (void*)&ScriptCamera::InternalNdcToScreenPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenPointToRay", (void*)&ScriptCamera::InternalScreenPointToRay);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ProjectPoint", (void*)&ScriptCamera::InternalProjectPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_UnprojectPoint", (void*)&ScriptCamera::InternalUnprojectPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMain", (void*)&ScriptCamera::InternalSetMain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsMain", (void*)&ScriptCamera::InternalIsMain);

	}

	MonoObject* ScriptCamera::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptCamera::InternalSetFlags(ScriptCamera* self, CameraFlag flags)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetFlags(flags);
	}

	CameraFlag ScriptCamera::InternalGetFlags(ScriptCamera* self)
	{
		Flags<CameraFlag> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetFlags();

		CameraFlag __output;
		__output = (CameraFlag)(uint32_t)tmp__output;

		return __output;
	}

	MonoObject* ScriptCamera::InternalGetViewport(ScriptCamera* self)
	{
		SPtr<Viewport> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetViewport();

		MonoObject* __output;
		__output = ScriptViewport::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptCamera::InternalSetHorzFov(ScriptCamera* self, TRadian<float>* fovy)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetHorzFov(*fovy);
	}

	void ScriptCamera::InternalGetHorzFov(ScriptCamera* self, TRadian<float>* __output)
	{
		TRadian<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetHorzFov();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetNearClipDistance(ScriptCamera* self, float nearDist)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetNearClipDistance(nearDist);
	}

	float ScriptCamera::InternalGetNearClipDistance(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetNearClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetFarClipDistance(ScriptCamera* self, float farDist)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetFarClipDistance(farDist);
	}

	float ScriptCamera::InternalGetFarClipDistance(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetFarClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetAspectRatio(ScriptCamera* self, float ratio)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetAspectRatio(ratio);
	}

	float ScriptCamera::InternalGetAspectRatio(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetAspectRatio();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalGetProjectionMatrixRs(ScriptCamera* self, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetProjectionMatrixRs();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalGetViewMatrix(ScriptCamera* self, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetViewMatrix();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetProjectionType(ScriptCamera* self, ProjectionType pt)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetProjectionType(pt);
	}

	ProjectionType ScriptCamera::InternalGetProjectionType(ScriptCamera* self)
	{
		ProjectionType tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetProjectionType();

		ProjectionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetOrthoWindowHeight(ScriptCamera* self, float h)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetOrthoWindowHeight(h);
	}

	float ScriptCamera::InternalGetOrthoWindowHeight(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetOrthoWindowHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetOrthoWindowWidth(ScriptCamera* self, float w)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetOrthoWindowWidth(w);
	}

	float ScriptCamera::InternalGetOrthoWindowWidth(ScriptCamera* self)
	{
		float tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetOrthoWindowWidth();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetPriority(ScriptCamera* self, int32_t priority)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetPriority(priority);
	}

	int32_t ScriptCamera::InternalGetPriority(ScriptCamera* self)
	{
		int32_t tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetPriority();

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetLayers(ScriptCamera* self, uint64_t layers)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetLayers(layers);
	}

	uint64_t ScriptCamera::InternalGetLayers(ScriptCamera* self)
	{
		uint64_t tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetLayers();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetMsaaCount(ScriptCamera* self, uint32_t count)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetMsaaCount(count);
	}

	uint32_t ScriptCamera::InternalGetMsaaCount(ScriptCamera* self)
	{
		uint32_t tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetMsaaCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetRenderSettings(ScriptCamera* self, MonoObject* settings)
	{
		SPtr<RenderSettings> tmpsettings;
		ScriptRenderSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptRenderSettings::GetScriptObjectWrapper(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = std::static_pointer_cast<RenderSettings>(scriptObjectWrappersettings->GetBaseNativeObjectAsShared());
		static_cast<CCamera*>(self->GetNativeObject())->SetRenderSettings(tmpsettings);
	}

	MonoObject* ScriptCamera::InternalGetRenderSettings(ScriptCamera* self)
	{
		SPtr<RenderSettings> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->GetRenderSettings();

		MonoObject* __output;
		__output = ScriptRenderSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptCamera::InternalNotifyNeedsRedraw(ScriptCamera* self)
	{
		static_cast<CCamera*>(self->GetNativeObject())->NotifyNeedsRedraw();
	}

	void ScriptCamera::InternalWorldToScreenPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->WorldToScreenPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalWorldToNdcPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->WorldToNdcPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalWorldToViewPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->WorldToViewPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToWorldPoint(ScriptCamera* self, TVector2I<int32_t>* screenPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->ScreenToWorldPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToViewPoint(ScriptCamera* self, TVector2I<int32_t>* screenPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->ScreenToViewPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToNdcPoint(ScriptCamera* self, TVector2I<int32_t>* screenPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->ScreenToNdcPoint(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToWorldPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->ViewToWorldPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToScreenPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->ViewToScreenPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToNdcPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->ViewToNdcPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToWorldPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->NdcToWorldPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToViewPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->NdcToViewPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNdcToScreenPoint(ScriptCamera* self, TVector2<float>* ndcPoint, TVector2I<int32_t>* __output)
	{
		TVector2I<int32_t> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->NdcToScreenPoint(*ndcPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenPointToRay(ScriptCamera* self, TVector2I<int32_t>* screenPoint, Ray* __output)
	{
		Ray tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->ScreenPointToRay(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalProjectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->ProjectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalUnprojectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output)
	{
		TVector3<float> tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->UnprojectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetMain(ScriptCamera* self, bool main)
	{
		static_cast<CCamera*>(self->GetNativeObject())->SetMain(main);
	}

	bool ScriptCamera::InternalIsMain(ScriptCamera* self)
	{
		bool tmp__output;
		tmp__output = static_cast<CCamera*>(self->GetNativeObject())->IsMain();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
