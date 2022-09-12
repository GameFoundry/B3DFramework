//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCCamera.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCCamera.h"
#include "BsScriptRenderSettings.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "BsScriptViewport.generated.h"
#include "Wrappers/BsScriptVector.h"
#include "Wrappers/BsScriptVector2I.h"

namespace bs
{
	ScriptCCamera::ScriptCCamera(MonoObject* managedInstance, const GameObjectHandle<CCamera>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCCamera::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setFlags", (void*)&ScriptCCamera::Internal_setFlags);
		metaData.scriptClass->AddInternalCall("Internal_getFlags", (void*)&ScriptCCamera::Internal_getFlags);
		metaData.scriptClass->AddInternalCall("Internal_getViewport", (void*)&ScriptCCamera::Internal_getViewport);
		metaData.scriptClass->AddInternalCall("Internal_setHorzFOV", (void*)&ScriptCCamera::Internal_setHorzFOV);
		metaData.scriptClass->AddInternalCall("Internal_getHorzFOV", (void*)&ScriptCCamera::Internal_getHorzFOV);
		metaData.scriptClass->AddInternalCall("Internal_setNearClipDistance", (void*)&ScriptCCamera::Internal_setNearClipDistance);
		metaData.scriptClass->AddInternalCall("Internal_getNearClipDistance", (void*)&ScriptCCamera::Internal_getNearClipDistance);
		metaData.scriptClass->AddInternalCall("Internal_setFarClipDistance", (void*)&ScriptCCamera::Internal_setFarClipDistance);
		metaData.scriptClass->AddInternalCall("Internal_getFarClipDistance", (void*)&ScriptCCamera::Internal_getFarClipDistance);
		metaData.scriptClass->AddInternalCall("Internal_setAspectRatio", (void*)&ScriptCCamera::Internal_setAspectRatio);
		metaData.scriptClass->AddInternalCall("Internal_getAspectRatio", (void*)&ScriptCCamera::Internal_getAspectRatio);
		metaData.scriptClass->AddInternalCall("Internal_getProjectionMatrixRS", (void*)&ScriptCCamera::Internal_getProjectionMatrixRS);
		metaData.scriptClass->AddInternalCall("Internal_getViewMatrix", (void*)&ScriptCCamera::Internal_getViewMatrix);
		metaData.scriptClass->AddInternalCall("Internal_setProjectionType", (void*)&ScriptCCamera::Internal_setProjectionType);
		metaData.scriptClass->AddInternalCall("Internal_getProjectionType", (void*)&ScriptCCamera::Internal_getProjectionType);
		metaData.scriptClass->AddInternalCall("Internal_setOrthoWindowHeight", (void*)&ScriptCCamera::Internal_setOrthoWindowHeight);
		metaData.scriptClass->AddInternalCall("Internal_getOrthoWindowHeight", (void*)&ScriptCCamera::Internal_getOrthoWindowHeight);
		metaData.scriptClass->AddInternalCall("Internal_setOrthoWindowWidth", (void*)&ScriptCCamera::Internal_setOrthoWindowWidth);
		metaData.scriptClass->AddInternalCall("Internal_getOrthoWindowWidth", (void*)&ScriptCCamera::Internal_getOrthoWindowWidth);
		metaData.scriptClass->AddInternalCall("Internal_setPriority", (void*)&ScriptCCamera::Internal_setPriority);
		metaData.scriptClass->AddInternalCall("Internal_getPriority", (void*)&ScriptCCamera::Internal_getPriority);
		metaData.scriptClass->AddInternalCall("Internal_setLayers", (void*)&ScriptCCamera::Internal_setLayers);
		metaData.scriptClass->AddInternalCall("Internal_getLayers", (void*)&ScriptCCamera::Internal_getLayers);
		metaData.scriptClass->AddInternalCall("Internal_setMSAACount", (void*)&ScriptCCamera::Internal_setMSAACount);
		metaData.scriptClass->AddInternalCall("Internal_getMSAACount", (void*)&ScriptCCamera::Internal_getMSAACount);
		metaData.scriptClass->AddInternalCall("Internal_setRenderSettings", (void*)&ScriptCCamera::Internal_setRenderSettings);
		metaData.scriptClass->AddInternalCall("Internal_getRenderSettings", (void*)&ScriptCCamera::Internal_getRenderSettings);
		metaData.scriptClass->AddInternalCall("Internal_notifyNeedsRedraw", (void*)&ScriptCCamera::Internal_notifyNeedsRedraw);
		metaData.scriptClass->AddInternalCall("Internal_worldToScreenPoint", (void*)&ScriptCCamera::Internal_worldToScreenPoint);
		metaData.scriptClass->AddInternalCall("Internal_worldToNdcPoint", (void*)&ScriptCCamera::Internal_worldToNdcPoint);
		metaData.scriptClass->AddInternalCall("Internal_worldToViewPoint", (void*)&ScriptCCamera::Internal_worldToViewPoint);
		metaData.scriptClass->AddInternalCall("Internal_screenToWorldPoint", (void*)&ScriptCCamera::Internal_screenToWorldPoint);
		metaData.scriptClass->AddInternalCall("Internal_screenToViewPoint", (void*)&ScriptCCamera::Internal_screenToViewPoint);
		metaData.scriptClass->AddInternalCall("Internal_screenToNdcPoint", (void*)&ScriptCCamera::Internal_screenToNdcPoint);
		metaData.scriptClass->AddInternalCall("Internal_viewToWorldPoint", (void*)&ScriptCCamera::Internal_viewToWorldPoint);
		metaData.scriptClass->AddInternalCall("Internal_viewToScreenPoint", (void*)&ScriptCCamera::Internal_viewToScreenPoint);
		metaData.scriptClass->AddInternalCall("Internal_viewToNdcPoint", (void*)&ScriptCCamera::Internal_viewToNdcPoint);
		metaData.scriptClass->AddInternalCall("Internal_ndcToWorldPoint", (void*)&ScriptCCamera::Internal_ndcToWorldPoint);
		metaData.scriptClass->AddInternalCall("Internal_ndcToViewPoint", (void*)&ScriptCCamera::Internal_ndcToViewPoint);
		metaData.scriptClass->AddInternalCall("Internal_ndcToScreenPoint", (void*)&ScriptCCamera::Internal_ndcToScreenPoint);
		metaData.scriptClass->AddInternalCall("Internal_screenPointToRay", (void*)&ScriptCCamera::Internal_screenPointToRay);
		metaData.scriptClass->AddInternalCall("Internal_projectPoint", (void*)&ScriptCCamera::Internal_projectPoint);
		metaData.scriptClass->AddInternalCall("Internal_unprojectPoint", (void*)&ScriptCCamera::Internal_unprojectPoint);
		metaData.scriptClass->AddInternalCall("Internal_setMain", (void*)&ScriptCCamera::Internal_setMain);
		metaData.scriptClass->AddInternalCall("Internal_isMain", (void*)&ScriptCCamera::Internal_isMain);

	}

	void ScriptCCamera::Internal_setFlags(ScriptCCamera* thisPtr, CameraFlag flags)
	{
		thisPtr->GetHandle()->setFlags(flags);
	}

	CameraFlag ScriptCCamera::Internal_getFlags(ScriptCCamera* thisPtr)
	{
		Flags<CameraFlag> tmp__output;
		tmp__output = thisPtr->GetHandle()->getFlags();

		CameraFlag __output;
		__output = (CameraFlag)(uint32_t)tmp__output;

		return __output;
	}

	MonoObject* ScriptCCamera::Internal_getViewport(ScriptCCamera* thisPtr)
	{
		SPtr<Viewport> tmp__output;
		tmp__output = thisPtr->GetHandle()->getViewport();

		MonoObject* __output;
		__output = ScriptViewport::create(tmp__output);

		return __output;
	}

	void ScriptCCamera::Internal_setHorzFOV(ScriptCCamera* thisPtr, Radian* fovy)
	{
		thisPtr->GetHandle()->setHorzFOV(*fovy);
	}

	void ScriptCCamera::Internal_getHorzFOV(ScriptCCamera* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->GetHandle()->getHorzFOV();

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_setNearClipDistance(ScriptCCamera* thisPtr, float nearDist)
	{
		thisPtr->GetHandle()->setNearClipDistance(nearDist);
	}

	float ScriptCCamera::Internal_getNearClipDistance(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getNearClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setFarClipDistance(ScriptCCamera* thisPtr, float farDist)
	{
		thisPtr->GetHandle()->setFarClipDistance(farDist);
	}

	float ScriptCCamera::Internal_getFarClipDistance(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getFarClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setAspectRatio(ScriptCCamera* thisPtr, float ratio)
	{
		thisPtr->GetHandle()->setAspectRatio(ratio);
	}

	float ScriptCCamera::Internal_getAspectRatio(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getAspectRatio();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_getProjectionMatrixRS(ScriptCCamera* thisPtr, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = thisPtr->GetHandle()->getProjectionMatrixRS();

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_getViewMatrix(ScriptCCamera* thisPtr, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = thisPtr->GetHandle()->getViewMatrix();

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_setProjectionType(ScriptCCamera* thisPtr, ProjectionType pt)
	{
		thisPtr->GetHandle()->setProjectionType(pt);
	}

	ProjectionType ScriptCCamera::Internal_getProjectionType(ScriptCCamera* thisPtr)
	{
		ProjectionType tmp__output;
		tmp__output = thisPtr->GetHandle()->getProjectionType();

		ProjectionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setOrthoWindowHeight(ScriptCCamera* thisPtr, float h)
	{
		thisPtr->GetHandle()->setOrthoWindowHeight(h);
	}

	float ScriptCCamera::Internal_getOrthoWindowHeight(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getOrthoWindowHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setOrthoWindowWidth(ScriptCCamera* thisPtr, float w)
	{
		thisPtr->GetHandle()->setOrthoWindowWidth(w);
	}

	float ScriptCCamera::Internal_getOrthoWindowWidth(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->getOrthoWindowWidth();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setPriority(ScriptCCamera* thisPtr, int32_t priority)
	{
		thisPtr->GetHandle()->setPriority(priority);
	}

	int32_t ScriptCCamera::Internal_getPriority(ScriptCCamera* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getPriority();

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setLayers(ScriptCCamera* thisPtr, uint64_t layers)
	{
		thisPtr->GetHandle()->setLayers(layers);
	}

	uint64_t ScriptCCamera::Internal_getLayers(ScriptCCamera* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getLayers();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setMSAACount(ScriptCCamera* thisPtr, uint32_t count)
	{
		thisPtr->GetHandle()->setMSAACount(count);
	}

	uint32_t ScriptCCamera::Internal_getMSAACount(ScriptCCamera* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetHandle()->getMSAACount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setRenderSettings(ScriptCCamera* thisPtr, MonoObject* settings)
	{
		SPtr<RenderSettings> tmpsettings;
		ScriptRenderSettings* scriptsettings;
		scriptsettings = ScriptRenderSettings::toNative(settings);
		if(scriptsettings != nullptr)
			tmpsettings = scriptsettings->GetInternal();
		thisPtr->GetHandle()->setRenderSettings(tmpsettings);
	}

	MonoObject* ScriptCCamera::Internal_getRenderSettings(ScriptCCamera* thisPtr)
	{
		SPtr<RenderSettings> tmp__output;
		tmp__output = thisPtr->GetHandle()->getRenderSettings();

		MonoObject* __output;
		__output = ScriptRenderSettings::create(tmp__output);

		return __output;
	}

	void ScriptCCamera::Internal_notifyNeedsRedraw(ScriptCCamera* thisPtr)
	{
		thisPtr->GetHandle()->notifyNeedsRedraw();
	}

	void ScriptCCamera::Internal_worldToScreenPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->GetHandle()->worldToScreenPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_worldToNdcPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->worldToNdcPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_worldToViewPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->worldToViewPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_screenToWorldPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->screenToWorldPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_screenToViewPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->screenToViewPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_screenToNdcPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->screenToNdcPoint(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_viewToWorldPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->viewToWorldPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_viewToScreenPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->GetHandle()->viewToScreenPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_viewToNdcPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetHandle()->viewToNdcPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_ndcToWorldPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->ndcToWorldPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_ndcToViewPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->ndcToViewPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_ndcToScreenPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->GetHandle()->ndcToScreenPoint(*ndcPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_screenPointToRay(ScriptCCamera* thisPtr, Vector2I* screenPoint, Ray* __output)
	{
		Ray tmp__output;
		tmp__output = thisPtr->GetHandle()->screenPointToRay(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_projectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->projectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_unprojectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->GetHandle()->unprojectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_setMain(ScriptCCamera* thisPtr, bool main)
	{
		thisPtr->GetHandle()->setMain(main);
	}

	bool ScriptCCamera::Internal_isMain(ScriptCCamera* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->isMain();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
