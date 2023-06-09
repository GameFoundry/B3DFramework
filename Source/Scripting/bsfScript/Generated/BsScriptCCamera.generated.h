//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2I.h"
#include "Math/BsMatrix4.h"
#include "../../../Foundation/bsfCore/Renderer/BsCamera.h"
#include "Math/BsRadian.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Math/BsRay.h"

namespace bs { class CCamera; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCamera : public TScriptComponent<ScriptCamera, CCamera>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Camera")

		ScriptCamera(MonoObject* managedInstance, const GameObjectHandle<CCamera>& value);

	private:
		static void InternalSetFlags(ScriptCamera* thisPtr, CameraFlag flags);
		static CameraFlag InternalGetFlags(ScriptCamera* thisPtr);
		static MonoObject* InternalGetViewport(ScriptCamera* thisPtr);
		static void InternalSetHorzFov(ScriptCamera* thisPtr, TRadian<float>* fovy);
		static void InternalGetHorzFov(ScriptCamera* thisPtr, TRadian<float>* __output);
		static void InternalSetNearClipDistance(ScriptCamera* thisPtr, float nearDist);
		static float InternalGetNearClipDistance(ScriptCamera* thisPtr);
		static void InternalSetFarClipDistance(ScriptCamera* thisPtr, float farDist);
		static float InternalGetFarClipDistance(ScriptCamera* thisPtr);
		static void InternalSetAspectRatio(ScriptCamera* thisPtr, float ratio);
		static float InternalGetAspectRatio(ScriptCamera* thisPtr);
		static void InternalGetProjectionMatrixRs(ScriptCamera* thisPtr, Matrix4* __output);
		static void InternalGetViewMatrix(ScriptCamera* thisPtr, Matrix4* __output);
		static void InternalSetProjectionType(ScriptCamera* thisPtr, ProjectionType pt);
		static ProjectionType InternalGetProjectionType(ScriptCamera* thisPtr);
		static void InternalSetOrthoWindowHeight(ScriptCamera* thisPtr, float h);
		static float InternalGetOrthoWindowHeight(ScriptCamera* thisPtr);
		static void InternalSetOrthoWindowWidth(ScriptCamera* thisPtr, float w);
		static float InternalGetOrthoWindowWidth(ScriptCamera* thisPtr);
		static void InternalSetPriority(ScriptCamera* thisPtr, int32_t priority);
		static int32_t InternalGetPriority(ScriptCamera* thisPtr);
		static void InternalSetLayers(ScriptCamera* thisPtr, uint64_t layers);
		static uint64_t InternalGetLayers(ScriptCamera* thisPtr);
		static void InternalSetMsaaCount(ScriptCamera* thisPtr, uint32_t count);
		static uint32_t InternalGetMsaaCount(ScriptCamera* thisPtr);
		static void InternalSetRenderSettings(ScriptCamera* thisPtr, MonoObject* settings);
		static MonoObject* InternalGetRenderSettings(ScriptCamera* thisPtr);
		static void InternalNotifyNeedsRedraw(ScriptCamera* thisPtr);
		static void InternalWorldToScreenPoint(ScriptCamera* thisPtr, TVector3<float>* worldPoint, TVector2I<int32_t>* __output);
		static void InternalWorldToNdcPoint(ScriptCamera* thisPtr, TVector3<float>* worldPoint, TVector2<float>* __output);
		static void InternalWorldToViewPoint(ScriptCamera* thisPtr, TVector3<float>* worldPoint, TVector3<float>* __output);
		static void InternalScreenToWorldPoint(ScriptCamera* thisPtr, TVector2I<int32_t>* screenPoint, float depth, TVector3<float>* __output);
		static void InternalScreenToViewPoint(ScriptCamera* thisPtr, TVector2I<int32_t>* screenPoint, float depth, TVector3<float>* __output);
		static void InternalScreenToNdcPoint(ScriptCamera* thisPtr, TVector2I<int32_t>* screenPoint, TVector2<float>* __output);
		static void InternalViewToWorldPoint(ScriptCamera* thisPtr, TVector3<float>* viewPoint, TVector3<float>* __output);
		static void InternalViewToScreenPoint(ScriptCamera* thisPtr, TVector3<float>* viewPoint, TVector2I<int32_t>* __output);
		static void InternalViewToNdcPoint(ScriptCamera* thisPtr, TVector3<float>* viewPoint, TVector2<float>* __output);
		static void InternalNdcToWorldPoint(ScriptCamera* thisPtr, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output);
		static void InternalNdcToViewPoint(ScriptCamera* thisPtr, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output);
		static void InternalNdcToScreenPoint(ScriptCamera* thisPtr, TVector2<float>* ndcPoint, TVector2I<int32_t>* __output);
		static void InternalScreenPointToRay(ScriptCamera* thisPtr, TVector2I<int32_t>* screenPoint, Ray* __output);
		static void InternalProjectPoint(ScriptCamera* thisPtr, TVector3<float>* point, TVector3<float>* __output);
		static void InternalUnprojectPoint(ScriptCamera* thisPtr, TVector3<float>* point, TVector3<float>* __output);
		static void InternalSetMain(ScriptCamera* thisPtr, bool main);
		static bool InternalIsMain(ScriptCamera* thisPtr);
	};
}
