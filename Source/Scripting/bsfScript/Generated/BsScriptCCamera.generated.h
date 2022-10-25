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

namespace bs
{
	class CCamera;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCCamera : public TScriptComponent<ScriptCCamera, CCamera>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Camera")

		ScriptCCamera(MonoObject* managedInstance, const GameObjectHandle<CCamera>& value);

	private:
		static void InternalSetFlags(ScriptCCamera* thisPtr, CameraFlag flags);
		static CameraFlag InternalGetFlags(ScriptCCamera* thisPtr);
		static MonoObject* InternalGetViewport(ScriptCCamera* thisPtr);
		static void InternalSetHorzFov(ScriptCCamera* thisPtr, Radian* fovy);
		static void InternalGetHorzFov(ScriptCCamera* thisPtr, Radian* __output);
		static void InternalSetNearClipDistance(ScriptCCamera* thisPtr, float nearDist);
		static float InternalGetNearClipDistance(ScriptCCamera* thisPtr);
		static void InternalSetFarClipDistance(ScriptCCamera* thisPtr, float farDist);
		static float InternalGetFarClipDistance(ScriptCCamera* thisPtr);
		static void InternalSetAspectRatio(ScriptCCamera* thisPtr, float ratio);
		static float InternalGetAspectRatio(ScriptCCamera* thisPtr);
		static void InternalGetProjectionMatrixRs(ScriptCCamera* thisPtr, Matrix4* __output);
		static void InternalGetViewMatrix(ScriptCCamera* thisPtr, Matrix4* __output);
		static void InternalSetProjectionType(ScriptCCamera* thisPtr, ProjectionType pt);
		static ProjectionType InternalGetProjectionType(ScriptCCamera* thisPtr);
		static void InternalSetOrthoWindowHeight(ScriptCCamera* thisPtr, float h);
		static float InternalGetOrthoWindowHeight(ScriptCCamera* thisPtr);
		static void InternalSetOrthoWindowWidth(ScriptCCamera* thisPtr, float w);
		static float InternalGetOrthoWindowWidth(ScriptCCamera* thisPtr);
		static void InternalSetPriority(ScriptCCamera* thisPtr, int32_t priority);
		static int32_t InternalGetPriority(ScriptCCamera* thisPtr);
		static void InternalSetLayers(ScriptCCamera* thisPtr, uint64_t layers);
		static uint64_t InternalGetLayers(ScriptCCamera* thisPtr);
		static void InternalSetMsaaCount(ScriptCCamera* thisPtr, uint32_t count);
		static uint32_t InternalGetMsaaCount(ScriptCCamera* thisPtr);
		static void InternalSetRenderSettings(ScriptCCamera* thisPtr, MonoObject* settings);
		static MonoObject* InternalGetRenderSettings(ScriptCCamera* thisPtr);
		static void InternalNotifyNeedsRedraw(ScriptCCamera* thisPtr);
		static void InternalWorldToScreenPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2I* __output);
		static void InternalWorldToNdcPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector2* __output);
		static void InternalWorldToViewPoint(ScriptCCamera* thisPtr, Vector3* worldPoint, Vector3* __output);
		static void InternalScreenToWorldPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output);
		static void InternalScreenToViewPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, float depth, Vector3* __output);
		static void InternalScreenToNdcPoint(ScriptCCamera* thisPtr, Vector2I* screenPoint, Vector2* __output);
		static void InternalViewToWorldPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector3* __output);
		static void InternalViewToScreenPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2I* __output);
		static void InternalViewToNdcPoint(ScriptCCamera* thisPtr, Vector3* viewPoint, Vector2* __output);
		static void InternalNdcToWorldPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output);
		static void InternalNdcToViewPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, float depth, Vector3* __output);
		static void InternalNdcToScreenPoint(ScriptCCamera* thisPtr, Vector2* ndcPoint, Vector2I* __output);
		static void InternalScreenPointToRay(ScriptCCamera* thisPtr, Vector2I* screenPoint, Ray* __output);
		static void InternalProjectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output);
		static void InternalUnprojectPoint(ScriptCCamera* thisPtr, Vector3* point, Vector3* __output);
		static void InternalSetMain(ScriptCCamera* thisPtr, bool main);
		static bool InternalIsMain(ScriptCCamera* thisPtr);
	};
} // namespace bs
