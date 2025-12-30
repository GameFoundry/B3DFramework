//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptCamera.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DCamera.h"
#include "B3DScriptRenderSettings.generated.h"
#include "B3DScriptViewport.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptTVector3.generated.h"
#include "B3DScriptTVector2.generated.h"
#include "B3DScriptTRay.generated.h"

namespace b3d
{
	ScriptCamera::ScriptCamera(const TGameObjectHandle<Camera>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptCamera::~ScriptCamera()
	{
		UnregisterEvents();
	}

	void ScriptCamera::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMain", (void*)&ScriptCamera::InternalSetMain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsMain", (void*)&ScriptCamera::InternalIsMain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFlags", (void*)&ScriptCamera::InternalSetFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFlags", (void*)&ScriptCamera::InternalGetFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHorizontalFOV", (void*)&ScriptCamera::InternalSetHorizontalFOV);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHorizontalFOV", (void*)&ScriptCamera::InternalGetHorizontalFOV);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNearClipDistance", (void*)&ScriptCamera::InternalSetNearClipDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNearClipDistance", (void*)&ScriptCamera::InternalGetNearClipDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFarClipDistance", (void*)&ScriptCamera::InternalSetFarClipDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFarClipDistance", (void*)&ScriptCamera::InternalGetFarClipDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAspectRatio", (void*)&ScriptCamera::InternalSetAspectRatio);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAspectRatio", (void*)&ScriptCamera::InternalGetAspectRatio);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetProjectionMatrix", (void*)&ScriptCamera::InternalGetProjectionMatrix);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetViewMatrix", (void*)&ScriptCamera::InternalGetViewMatrix);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetProjectionType", (void*)&ScriptCamera::InternalSetProjectionType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetProjectionType", (void*)&ScriptCamera::InternalGetProjectionType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOrthographicHeight", (void*)&ScriptCamera::InternalSetOrthographicHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrthographicHeight", (void*)&ScriptCamera::InternalGetOrthographicHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOrthographicWidth", (void*)&ScriptCamera::InternalSetOrthographicWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOrthographicWidth", (void*)&ScriptCamera::InternalGetOrthographicWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptCamera::InternalSetPriority);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptCamera::InternalGetPriority);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetLayers", (void*)&ScriptCamera::InternalSetLayers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetLayers", (void*)&ScriptCamera::InternalGetLayers);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSampleCount", (void*)&ScriptCamera::InternalSetSampleCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSampleCount", (void*)&ScriptCamera::InternalGetSampleCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetViewport", (void*)&ScriptCamera::InternalGetViewport);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRenderSettings", (void*)&ScriptCamera::InternalSetRenderSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRenderSettings", (void*)&ScriptCamera::InternalGetRenderSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_NotifyNeedsRedraw", (void*)&ScriptCamera::InternalNotifyNeedsRedraw);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WorldToScreenPoint", (void*)&ScriptCamera::InternalWorldToScreenPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WorldToNDCPoint", (void*)&ScriptCamera::InternalWorldToNDCPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_WorldToViewPoint", (void*)&ScriptCamera::InternalWorldToViewPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenToWorldPoint", (void*)&ScriptCamera::InternalScreenToWorldPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenToViewPoint", (void*)&ScriptCamera::InternalScreenToViewPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenToNDCPoint", (void*)&ScriptCamera::InternalScreenToNDCPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ViewToWorldPoint", (void*)&ScriptCamera::InternalViewToWorldPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ViewToScreenPoint", (void*)&ScriptCamera::InternalViewToScreenPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ViewToNDCPoint", (void*)&ScriptCamera::InternalViewToNDCPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_NDCToWorldPoint", (void*)&ScriptCamera::InternalNDCToWorldPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_NDCToViewPoint", (void*)&ScriptCamera::InternalNDCToViewPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_NDCToScreenPoint", (void*)&ScriptCamera::InternalNDCToScreenPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScreenPointToRay", (void*)&ScriptCamera::InternalScreenPointToRay);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ProjectPoint", (void*)&ScriptCamera::InternalProjectPoint);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_UnprojectPoint", (void*)&ScriptCamera::InternalUnprojectPoint);

	}

	MonoObject* ScriptCamera::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptCamera::InternalSetMain(ScriptCamera* self, bool main)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetMain(main);
	}

	bool ScriptCamera::InternalIsMain(ScriptCamera* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->IsMain();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetFlags(ScriptCamera* self, CameraFlag flags)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetFlags(flags);
	}

	CameraFlag ScriptCamera::InternalGetFlags(ScriptCamera* self)
	{
		Flags<CameraFlag> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetFlags();

		CameraFlag __output;
		__output = (CameraFlag)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetHorizontalFOV(ScriptCamera* self, TRadian<float>* fovy)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetHorizontalFOV(*fovy);
	}

	void ScriptCamera::InternalGetHorizontalFOV(ScriptCamera* self, TRadian<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TRadian<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetHorizontalFOV();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetNearClipDistance(ScriptCamera* self, float nearDist)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetNearClipDistance(nearDist);
	}

	float ScriptCamera::InternalGetNearClipDistance(ScriptCamera* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetNearClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetFarClipDistance(ScriptCamera* self, float farDist)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetFarClipDistance(farDist);
	}

	float ScriptCamera::InternalGetFarClipDistance(ScriptCamera* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetFarClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetAspectRatio(ScriptCamera* self, float ratio)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetAspectRatio(ratio);
	}

	float ScriptCamera::InternalGetAspectRatio(ScriptCamera* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetAspectRatio();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalGetProjectionMatrix(ScriptCamera* self, TMatrix4<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TMatrix4<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetProjectionMatrix();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalGetViewMatrix(ScriptCamera* self, TMatrix4<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TMatrix4<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetViewMatrix();

		*__output = tmp__output;
	}

	void ScriptCamera::InternalSetProjectionType(ScriptCamera* self, ProjectionType pt)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetProjectionType(pt);
	}

	ProjectionType ScriptCamera::InternalGetProjectionType(ScriptCamera* self)
	{
		ProjectionType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetProjectionType();

		ProjectionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetOrthographicHeight(ScriptCamera* self, float height)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetOrthographicHeight(height);
	}

	float ScriptCamera::InternalGetOrthographicHeight(ScriptCamera* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetOrthographicHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetOrthographicWidth(ScriptCamera* self, float width)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetOrthographicWidth(width);
	}

	float ScriptCamera::InternalGetOrthographicWidth(ScriptCamera* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetOrthographicWidth();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetPriority(ScriptCamera* self, int32_t priority)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetPriority(priority);
	}

	int32_t ScriptCamera::InternalGetPriority(ScriptCamera* self)
	{
		int32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetPriority();

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetLayers(ScriptCamera* self, uint64_t layers)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetLayers(layers);
	}

	uint64_t ScriptCamera::InternalGetLayers(ScriptCamera* self)
	{
		uint64_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetLayers();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCamera::InternalSetSampleCount(ScriptCamera* self, uint32_t count)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->SetSampleCount(count);
	}

	uint32_t ScriptCamera::InternalGetSampleCount(ScriptCamera* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetSampleCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptCamera::InternalGetViewport(ScriptCamera* self)
	{
		SPtr<Viewport> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetViewport();

		MonoObject* __output;
		__output = ScriptViewport::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptCamera::InternalSetRenderSettings(ScriptCamera* self, MonoObject* settings)
	{
		if(!self->IsNativeObjectValid())
			return;

		SPtr<RenderSettings> tmpsettings;
		ScriptRenderSettings* scriptObjectWrappersettings;
		scriptObjectWrappersettings = ScriptRenderSettings::GetScriptObjectWrapper(settings);
		if(scriptObjectWrappersettings != nullptr)
			tmpsettings = std::static_pointer_cast<RenderSettings>(scriptObjectWrappersettings->GetBaseNativeObjectAsShared());
		static_cast<Camera*>(self->GetNativeObject())->SetRenderSettings(tmpsettings);
	}

	MonoObject* ScriptCamera::InternalGetRenderSettings(ScriptCamera* self)
	{
		SPtr<RenderSettings> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Camera*>(self->GetNativeObject())->GetRenderSettings();

		MonoObject* __output;
		__output = ScriptRenderSettings::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptCamera::InternalNotifyNeedsRedraw(ScriptCamera* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Camera*>(self->GetNativeObject())->NotifyNeedsRedraw();
	}

	void ScriptCamera::InternalWorldToScreenPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2<int32_t>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<int32_t> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->WorldToScreenPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalWorldToNDCPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->WorldToNDCPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalWorldToViewPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->WorldToViewPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToWorldPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, float depth, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->ScreenToWorldPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToViewPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, float depth, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->ScreenToViewPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenToNDCPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->ScreenToNDCPoint(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToWorldPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->ViewToWorldPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToScreenPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2<int32_t>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<int32_t> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->ViewToScreenPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalViewToNDCPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->ViewToNDCPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNDCToWorldPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->NDCToWorldPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNDCToViewPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->NDCToViewPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalNDCToScreenPoint(ScriptCamera* self, TVector2<float>* ndcPoint, TVector2<int32_t>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector2<int32_t> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->NDCToScreenPoint(*ndcPoint);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalScreenPointToRay(ScriptCamera* self, TVector2<int32_t>* screenPoint, __TRay_float_Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TRay<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->ScreenPointToRay(*screenPoint);

		__TRay_float_Interop interop__output;
		interop__output = ScriptRay::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptRay::GetMetaData()->ScriptClass->GetInternalClass());
	}

	void ScriptCamera::InternalProjectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->ProjectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCamera::InternalUnprojectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TVector3<float> tmp__output;
		tmp__output = static_cast<Camera*>(self->GetNativeObject())->UnprojectPoint(*point);

		*__output = tmp__output;
	}
}
