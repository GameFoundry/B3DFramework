//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfCore/Renderer/BsCamera.h"
#include "Math/BsMatrix4.h"
#include "Math/BsRadian.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "../../../Foundation/bsfUtility/Math/BsRay.h"

namespace b3d { class CCamera; }
namespace b3d { struct __TRay_float_Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCamera : public TScriptGameObjectWrapper<CCamera, ScriptCamera>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Camera")

		ScriptCamera(const GameObjectHandle<CCamera>& nativeObject);
		~ScriptCamera();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetFlags(ScriptCamera* self, CameraFlag flags);
		static CameraFlag InternalGetFlags(ScriptCamera* self);
		static MonoObject* InternalGetViewport(ScriptCamera* self);
		static void InternalSetHorzFov(ScriptCamera* self, TRadian<float>* fovy);
		static void InternalGetHorzFov(ScriptCamera* self, TRadian<float>* __output);
		static void InternalSetNearClipDistance(ScriptCamera* self, float nearDist);
		static float InternalGetNearClipDistance(ScriptCamera* self);
		static void InternalSetFarClipDistance(ScriptCamera* self, float farDist);
		static float InternalGetFarClipDistance(ScriptCamera* self);
		static void InternalSetAspectRatio(ScriptCamera* self, float ratio);
		static float InternalGetAspectRatio(ScriptCamera* self);
		static void InternalGetProjectionMatrixRs(ScriptCamera* self, TMatrix4<float>* __output);
		static void InternalGetViewMatrix(ScriptCamera* self, TMatrix4<float>* __output);
		static void InternalSetProjectionType(ScriptCamera* self, ProjectionType pt);
		static ProjectionType InternalGetProjectionType(ScriptCamera* self);
		static void InternalSetOrthoWindowHeight(ScriptCamera* self, float h);
		static float InternalGetOrthoWindowHeight(ScriptCamera* self);
		static void InternalSetOrthoWindowWidth(ScriptCamera* self, float w);
		static float InternalGetOrthoWindowWidth(ScriptCamera* self);
		static void InternalSetPriority(ScriptCamera* self, int32_t priority);
		static int32_t InternalGetPriority(ScriptCamera* self);
		static void InternalSetLayers(ScriptCamera* self, uint64_t layers);
		static uint64_t InternalGetLayers(ScriptCamera* self);
		static void InternalSetMsaaCount(ScriptCamera* self, uint32_t count);
		static uint32_t InternalGetMsaaCount(ScriptCamera* self);
		static void InternalSetRenderSettings(ScriptCamera* self, MonoObject* settings);
		static MonoObject* InternalGetRenderSettings(ScriptCamera* self);
		static void InternalNotifyNeedsRedraw(ScriptCamera* self);
		static void InternalWorldToScreenPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2<int32_t>* __output);
		static void InternalWorldToNdcPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2<float>* __output);
		static void InternalWorldToViewPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector3<float>* __output);
		static void InternalScreenToWorldPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, float depth, TVector3<float>* __output);
		static void InternalScreenToViewPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, float depth, TVector3<float>* __output);
		static void InternalScreenToNdcPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, TVector2<float>* __output);
		static void InternalViewToWorldPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector3<float>* __output);
		static void InternalViewToScreenPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2<int32_t>* __output);
		static void InternalViewToNdcPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2<float>* __output);
		static void InternalNdcToWorldPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output);
		static void InternalNdcToViewPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output);
		static void InternalNdcToScreenPoint(ScriptCamera* self, TVector2<float>* ndcPoint, TVector2<int32_t>* __output);
		static void InternalScreenPointToRay(ScriptCamera* self, TVector2<int32_t>* screenPoint, __TRay_float_Interop* __output);
		static void InternalProjectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output);
		static void InternalUnprojectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output);
		static void InternalSetMain(ScriptCamera* self, bool main);
		static bool InternalIsMain(ScriptCamera* self);
	};
}
