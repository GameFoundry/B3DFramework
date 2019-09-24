//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsVector2.h"
#include "Math/BsRadian.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "Math/BsMatrix4.h"
#include "../../../Foundation/bsfCore/Renderer/BsCamera.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2I.h"
#include "Math/BsRay.h"

namespace bs { class CCamera; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCCamera : public TScriptComponent<ScriptCCamera, CCamera>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Camera")

		ScriptCCamera(MonoObject* managedInstance, const GameObjectHandle<CCamera>& value);

	private:
		static void Internal_setFlags(ScriptCCamera* thisPtr, CameraFlag flags);
		static CameraFlag Internal_getFlags(ScriptCCamera* thisPtr);
		static MonoObject* Internal_getViewport(ScriptCCamera* thisPtr);
		static void Internal_setHorzFOV(ScriptCCamera* thisPtr, Radian* fovy);
		static void Internal_getHorzFOV(ScriptCCamera* thisPtr, Radian* __output);
		static void Internal_setNearClipDistance(ScriptCCamera* thisPtr, float nearDist);
		static float Internal_getNearClipDistance(ScriptCCamera* thisPtr);
		static void Internal_setFarClipDistance(ScriptCCamera* thisPtr, float farDist);
		static float Internal_getFarClipDistance(ScriptCCamera* thisPtr);
		static void Internal_setAspectRatio(ScriptCCamera* thisPtr, float ratio);
		static float Internal_getAspectRatio(ScriptCCamera* thisPtr);
		static void Internal_getProjectionMatrixRS(ScriptCCamera* thisPtr, Matrix4* __output);
		static void Internal_getViewMatrix(ScriptCCamera* thisPtr, Matrix4* __output);
		static void Internal_setProjectionType(ScriptCCamera* thisPtr, ProjectionType pt);
		static ProjectionType Internal_getProjectionType(ScriptCCamera* thisPtr);
		static void Internal_setOrthoWindowHeight(ScriptCCamera* thisPtr, float h);
		static float Internal_getOrthoWindowHeight(ScriptCCamera* thisPtr);
		static void Internal_setOrthoWindowWidth(ScriptCCamera* thisPtr, float w);
		static float Internal_getOrthoWindowWidth(ScriptCCamera* thisPtr);
		static void Internal_setPriority(ScriptCCamera* thisPtr, int32_t priority);
		static int32_t Internal_getPriority(ScriptCCamera* thisPtr);
		static void Internal_setLayers(ScriptCCamera* thisPtr, uint64_t layers);
		static uint64_t Internal_getLayers(ScriptCCamera* thisPtr);
		static void Internal_setMSAACount(ScriptCCamera* thisPtr, uint32_t count);
		static uint32_t Internal_getMSAACount(ScriptCCamera* thisPtr);
		static void Internal_setRenderSettings(ScriptCCamera* thisPtr, MonoObject* settings);
		static MonoObject* Internal_getRenderSettings(ScriptCCamera* thisPtr);
		static void Internal_notifyNeedsRedraw(ScriptCCamera* thisPtr);
		static void Internal_worldToScreenPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2I* __output);
		static void Internal_worldToNdcPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2* __output);
		static void Internal_worldToViewPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector3* __output);
		static void Internal_screenToWorldPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output);
		static void Internal_screenToViewPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output);
		static void Internal_screenToNdcPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, Vector2* __output);
		static void Internal_viewToWorldPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector3* __output);
		static void Internal_viewToScreenPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2I* __output);
		static void Internal_viewToNdcPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2* __output);
		static void Internal_ndcToWorldPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output);
		static void Internal_ndcToViewPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output);
		static void Internal_ndcToScreenPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, Vector2I* __output);
		static void Internal_screenPointToRay(ScriptCCamera* thisPtr, Vector2I* screenPoint, Ray* __output);
		static void Internal_projectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output);
		static void Internal_unprojectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output);
		static void Internal_setMain(ScriptCCamera* thisPtr, bool main);
		static bool Internal_isMain(ScriptCCamera* thisPtr);
	};
}
