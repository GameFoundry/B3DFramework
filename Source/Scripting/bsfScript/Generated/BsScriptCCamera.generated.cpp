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

	void ScriptCCamera::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setFlags", (void*)&ScriptCCamera::Internal_setFlags);
		metaData.scriptClass->addInternalCall("Internal_getFlags", (void*)&ScriptCCamera::Internal_getFlags);
		metaData.scriptClass->addInternalCall("Internal_getViewport", (void*)&ScriptCCamera::Internal_getViewport);
		metaData.scriptClass->addInternalCall("Internal_setHorzFOV", (void*)&ScriptCCamera::Internal_setHorzFOV);
		metaData.scriptClass->addInternalCall("Internal_getHorzFOV", (void*)&ScriptCCamera::Internal_getHorzFOV);
		metaData.scriptClass->addInternalCall("Internal_setNearClipDistance", (void*)&ScriptCCamera::Internal_setNearClipDistance);
		metaData.scriptClass->addInternalCall("Internal_getNearClipDistance", (void*)&ScriptCCamera::Internal_getNearClipDistance);
		metaData.scriptClass->addInternalCall("Internal_setFarClipDistance", (void*)&ScriptCCamera::Internal_setFarClipDistance);
		metaData.scriptClass->addInternalCall("Internal_getFarClipDistance", (void*)&ScriptCCamera::Internal_getFarClipDistance);
		metaData.scriptClass->addInternalCall("Internal_setAspectRatio", (void*)&ScriptCCamera::Internal_setAspectRatio);
		metaData.scriptClass->addInternalCall("Internal_getAspectRatio", (void*)&ScriptCCamera::Internal_getAspectRatio);
		metaData.scriptClass->addInternalCall("Internal_getProjectionMatrixRS", (void*)&ScriptCCamera::Internal_getProjectionMatrixRS);
		metaData.scriptClass->addInternalCall("Internal_getViewMatrix", (void*)&ScriptCCamera::Internal_getViewMatrix);
		metaData.scriptClass->addInternalCall("Internal_setProjectionType", (void*)&ScriptCCamera::Internal_setProjectionType);
		metaData.scriptClass->addInternalCall("Internal_getProjectionType", (void*)&ScriptCCamera::Internal_getProjectionType);
		metaData.scriptClass->addInternalCall("Internal_setOrthoWindowHeight", (void*)&ScriptCCamera::Internal_setOrthoWindowHeight);
		metaData.scriptClass->addInternalCall("Internal_getOrthoWindowHeight", (void*)&ScriptCCamera::Internal_getOrthoWindowHeight);
		metaData.scriptClass->addInternalCall("Internal_setOrthoWindowWidth", (void*)&ScriptCCamera::Internal_setOrthoWindowWidth);
		metaData.scriptClass->addInternalCall("Internal_getOrthoWindowWidth", (void*)&ScriptCCamera::Internal_getOrthoWindowWidth);
		metaData.scriptClass->addInternalCall("Internal_setPriority", (void*)&ScriptCCamera::Internal_setPriority);
		metaData.scriptClass->addInternalCall("Internal_getPriority", (void*)&ScriptCCamera::Internal_getPriority);
		metaData.scriptClass->addInternalCall("Internal_setLayers", (void*)&ScriptCCamera::Internal_setLayers);
		metaData.scriptClass->addInternalCall("Internal_getLayers", (void*)&ScriptCCamera::Internal_getLayers);
		metaData.scriptClass->addInternalCall("Internal_setMSAACount", (void*)&ScriptCCamera::Internal_setMSAACount);
		metaData.scriptClass->addInternalCall("Internal_getMSAACount", (void*)&ScriptCCamera::Internal_getMSAACount);
		metaData.scriptClass->addInternalCall("Internal_setRenderSettings", (void*)&ScriptCCamera::Internal_setRenderSettings);
		metaData.scriptClass->addInternalCall("Internal_getRenderSettings", (void*)&ScriptCCamera::Internal_getRenderSettings);
		metaData.scriptClass->addInternalCall("Internal_notifyNeedsRedraw", (void*)&ScriptCCamera::Internal_notifyNeedsRedraw);
		metaData.scriptClass->addInternalCall("Internal_worldToScreenPoint", (void*)&ScriptCCamera::Internal_worldToScreenPoint);
		metaData.scriptClass->addInternalCall("Internal_worldToNdcPoint", (void*)&ScriptCCamera::Internal_worldToNdcPoint);
		metaData.scriptClass->addInternalCall("Internal_worldToViewPoint", (void*)&ScriptCCamera::Internal_worldToViewPoint);
		metaData.scriptClass->addInternalCall("Internal_screenToWorldPoint", (void*)&ScriptCCamera::Internal_screenToWorldPoint);
		metaData.scriptClass->addInternalCall("Internal_screenToViewPoint", (void*)&ScriptCCamera::Internal_screenToViewPoint);
		metaData.scriptClass->addInternalCall("Internal_screenToNdcPoint", (void*)&ScriptCCamera::Internal_screenToNdcPoint);
		metaData.scriptClass->addInternalCall("Internal_viewToWorldPoint", (void*)&ScriptCCamera::Internal_viewToWorldPoint);
		metaData.scriptClass->addInternalCall("Internal_viewToScreenPoint", (void*)&ScriptCCamera::Internal_viewToScreenPoint);
		metaData.scriptClass->addInternalCall("Internal_viewToNdcPoint", (void*)&ScriptCCamera::Internal_viewToNdcPoint);
		metaData.scriptClass->addInternalCall("Internal_ndcToWorldPoint", (void*)&ScriptCCamera::Internal_ndcToWorldPoint);
		metaData.scriptClass->addInternalCall("Internal_ndcToViewPoint", (void*)&ScriptCCamera::Internal_ndcToViewPoint);
		metaData.scriptClass->addInternalCall("Internal_ndcToScreenPoint", (void*)&ScriptCCamera::Internal_ndcToScreenPoint);
		metaData.scriptClass->addInternalCall("Internal_screenPointToRay", (void*)&ScriptCCamera::Internal_screenPointToRay);
		metaData.scriptClass->addInternalCall("Internal_projectPoint", (void*)&ScriptCCamera::Internal_projectPoint);
		metaData.scriptClass->addInternalCall("Internal_unprojectPoint", (void*)&ScriptCCamera::Internal_unprojectPoint);
		metaData.scriptClass->addInternalCall("Internal_setMain", (void*)&ScriptCCamera::Internal_setMain);
		metaData.scriptClass->addInternalCall("Internal_isMain", (void*)&ScriptCCamera::Internal_isMain);

	}

	void ScriptCCamera::Internal_setFlags(ScriptCCamera* thisPtr, CameraFlag flags)
	{
		thisPtr->getHandle()->setFlags(flags);
	}

	CameraFlag ScriptCCamera::Internal_getFlags(ScriptCCamera* thisPtr)
	{
		Flags<CameraFlag> tmp__output;
		tmp__output = thisPtr->getHandle()->getFlags();

		CameraFlag __output;
		__output = (CameraFlag)(uint32_t)tmp__output;

		return __output;
	}

	MonoObject* ScriptCCamera::Internal_getViewport(ScriptCCamera* thisPtr)
	{
		SPtr<Viewport> tmp__output;
		tmp__output = thisPtr->getHandle()->getViewport();

		MonoObject* __output;
		__output = ScriptViewport::create(tmp__output);

		return __output;
	}

	void ScriptCCamera::Internal_setHorzFOV(ScriptCCamera* thisPtr, Radian* fovy)
	{
		thisPtr->getHandle()->setHorzFOV(*fovy);
	}

	void ScriptCCamera::Internal_getHorzFOV(ScriptCCamera* thisPtr, Radian* __output)
	{
		Radian tmp__output;
		tmp__output = thisPtr->getHandle()->getHorzFOV();

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_setNearClipDistance(ScriptCCamera* thisPtr, float nearDist)
	{
		thisPtr->getHandle()->setNearClipDistance(nearDist);
	}

	float ScriptCCamera::Internal_getNearClipDistance(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getNearClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setFarClipDistance(ScriptCCamera* thisPtr, float farDist)
	{
		thisPtr->getHandle()->setFarClipDistance(farDist);
	}

	float ScriptCCamera::Internal_getFarClipDistance(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getFarClipDistance();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setAspectRatio(ScriptCCamera* thisPtr, float ratio)
	{
		thisPtr->getHandle()->setAspectRatio(ratio);
	}

	float ScriptCCamera::Internal_getAspectRatio(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getAspectRatio();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_getProjectionMatrixRS(ScriptCCamera* thisPtr, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = thisPtr->getHandle()->getProjectionMatrixRS();

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_getViewMatrix(ScriptCCamera* thisPtr, Matrix4* __output)
	{
		Matrix4 tmp__output;
		tmp__output = thisPtr->getHandle()->getViewMatrix();

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_setProjectionType(ScriptCCamera* thisPtr, ProjectionType pt)
	{
		thisPtr->getHandle()->setProjectionType(pt);
	}

	ProjectionType ScriptCCamera::Internal_getProjectionType(ScriptCCamera* thisPtr)
	{
		ProjectionType tmp__output;
		tmp__output = thisPtr->getHandle()->getProjectionType();

		ProjectionType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setOrthoWindowHeight(ScriptCCamera* thisPtr, float h)
	{
		thisPtr->getHandle()->setOrthoWindowHeight(h);
	}

	float ScriptCCamera::Internal_getOrthoWindowHeight(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getOrthoWindowHeight();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setOrthoWindowWidth(ScriptCCamera* thisPtr, float w)
	{
		thisPtr->getHandle()->setOrthoWindowWidth(w);
	}

	float ScriptCCamera::Internal_getOrthoWindowWidth(ScriptCCamera* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getOrthoWindowWidth();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setPriority(ScriptCCamera* thisPtr, int32_t priority)
	{
		thisPtr->getHandle()->setPriority(priority);
	}

	int32_t ScriptCCamera::Internal_getPriority(ScriptCCamera* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getPriority();

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setLayers(ScriptCCamera* thisPtr, uint64_t layers)
	{
		thisPtr->getHandle()->setLayers(layers);
	}

	uint64_t ScriptCCamera::Internal_getLayers(ScriptCCamera* thisPtr)
	{
		uint64_t tmp__output;
		tmp__output = thisPtr->getHandle()->getLayers();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCCamera::Internal_setMSAACount(ScriptCCamera* thisPtr, uint32_t count)
	{
		thisPtr->getHandle()->setMSAACount(count);
	}

	uint32_t ScriptCCamera::Internal_getMSAACount(ScriptCCamera* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getHandle()->getMSAACount();

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
			tmpsettings = scriptsettings->getInternal();
		thisPtr->getHandle()->setRenderSettings(tmpsettings);
	}

	MonoObject* ScriptCCamera::Internal_getRenderSettings(ScriptCCamera* thisPtr)
	{
		SPtr<RenderSettings> tmp__output;
		tmp__output = thisPtr->getHandle()->getRenderSettings();

		MonoObject* __output;
		__output = ScriptRenderSettings::create(tmp__output);

		return __output;
	}

	void ScriptCCamera::Internal_notifyNeedsRedraw(ScriptCCamera* thisPtr)
	{
		thisPtr->getHandle()->notifyNeedsRedraw();
	}

	void ScriptCCamera::Internal_worldToScreenPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->getHandle()->worldToScreenPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_worldToNdcPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->getHandle()->worldToNdcPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_worldToViewPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->worldToViewPoint(*worldPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_screenToWorldPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->screenToWorldPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_screenToViewPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->screenToViewPoint(*screenPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_screenToNdcPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->getHandle()->screenToNdcPoint(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_viewToWorldPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->viewToWorldPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_viewToScreenPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->getHandle()->viewToScreenPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_viewToNdcPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->getHandle()->viewToNdcPoint(*viewPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_ndcToWorldPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->ndcToWorldPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_ndcToViewPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->ndcToViewPoint(*ndcPoint, depth);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_ndcToScreenPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, Vector2I* __output)
	{
		Vector2I tmp__output;
		tmp__output = thisPtr->getHandle()->ndcToScreenPoint(*ndcPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_screenPointToRay(ScriptCCamera* thisPtr, Vector2I* screenPoint, Ray* __output)
	{
		Ray tmp__output;
		tmp__output = thisPtr->getHandle()->screenPointToRay(*screenPoint);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_projectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->projectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_unprojectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output)
	{
		Vector3 tmp__output;
		tmp__output = thisPtr->getHandle()->unprojectPoint(*point);

		*__output = tmp__output;
	}

	void ScriptCCamera::Internal_setMain(ScriptCCamera* thisPtr, bool main)
	{
		thisPtr->getHandle()->setMain(main);
	}

	bool ScriptCCamera::Internal_isMain(ScriptCCamera* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->isMain();

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
