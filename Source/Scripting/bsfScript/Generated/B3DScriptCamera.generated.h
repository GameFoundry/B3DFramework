//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Core/Utility/B3DCommonTypes.h"
#include "../../../Foundation/Core/Components/B3DCamera.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DRadian.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"
#include "../../../Foundation/Utility/Math/B3DRay.h"

namespace b3d { class Camera; }
namespace b3d { struct __TRay_float_Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCamera : public TScriptGameObjectWrapper<Camera, ScriptCamera>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Camera")

		ScriptCamera(const GameObjectHandle<Camera>& nativeObject);
		~ScriptCamera();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetMain(ScriptCamera* self, bool main);
		static bool InternalIsMain(ScriptCamera* self);
		static void InternalSetFlags(ScriptCamera* self, CameraFlag flags);
		static CameraFlag InternalGetFlags(ScriptCamera* self);
		static void InternalSetHorizontalFOV(ScriptCamera* self, TRadian<float>* fovy);
		static void InternalGetHorizontalFOV(ScriptCamera* self, TRadian<float>* __output);
		static void InternalSetNearClipDistance(ScriptCamera* self, float nearDist);
		static float InternalGetNearClipDistance(ScriptCamera* self);
		static void InternalSetFarClipDistance(ScriptCamera* self, float farDist);
		static float InternalGetFarClipDistance(ScriptCamera* self);
		static void InternalSetAspectRatio(ScriptCamera* self, float ratio);
		static float InternalGetAspectRatio(ScriptCamera* self);
		static void InternalGetProjectionMatrix(ScriptCamera* self, TMatrix4<float>* __output);
		static void InternalGetViewMatrix(ScriptCamera* self, TMatrix4<float>* __output);
		static void InternalSetProjectionType(ScriptCamera* self, ProjectionType pt);
		static ProjectionType InternalGetProjectionType(ScriptCamera* self);
		static void InternalSetOrthographicHeight(ScriptCamera* self, float height);
		static float InternalGetOrthographicHeight(ScriptCamera* self);
		static void InternalSetOrthographicWidth(ScriptCamera* self, float width);
		static float InternalGetOrthographicWidth(ScriptCamera* self);
		static void InternalSetPriority(ScriptCamera* self, int32_t priority);
		static int32_t InternalGetPriority(ScriptCamera* self);
		static void InternalSetLayers(ScriptCamera* self, uint64_t layers);
		static uint64_t InternalGetLayers(ScriptCamera* self);
		static void InternalSetSampleCount(ScriptCamera* self, uint32_t count);
		static uint32_t InternalGetSampleCount(ScriptCamera* self);
		static MonoObject* InternalGetViewport(ScriptCamera* self);
		static void InternalSetRenderSettings(ScriptCamera* self, MonoObject* settings);
		static MonoObject* InternalGetRenderSettings(ScriptCamera* self);
		static void InternalNotifyNeedsRedraw(ScriptCamera* self);
		static void InternalWorldToScreenPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2<int32_t>* __output);
		static void InternalWorldToNDCPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector2<float>* __output);
		static void InternalWorldToViewPoint(ScriptCamera* self, TVector3<float>* worldPoint, TVector3<float>* __output);
		static void InternalScreenToWorldPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, float depth, TVector3<float>* __output);
		static void InternalScreenToViewPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, float depth, TVector3<float>* __output);
		static void InternalScreenToNDCPoint(ScriptCamera* self, TVector2<int32_t>* screenPoint, TVector2<float>* __output);
		static void InternalViewToWorldPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector3<float>* __output);
		static void InternalViewToScreenPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2<int32_t>* __output);
		static void InternalViewToNDCPoint(ScriptCamera* self, TVector3<float>* viewPoint, TVector2<float>* __output);
		static void InternalNDCToWorldPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output);
		static void InternalNDCToViewPoint(ScriptCamera* self, TVector2<float>* ndcPoint, float depth, TVector3<float>* __output);
		static void InternalNDCToScreenPoint(ScriptCamera* self, TVector2<float>* ndcPoint, TVector2<int32_t>* __output);
		static void InternalScreenPointToRay(ScriptCamera* self, TVector2<int32_t>* screenPoint, __TRay_float_Interop* __output);
		static void InternalProjectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output);
		static void InternalUnprojectPoint(ScriptCamera* self, TVector3<float>* point, TVector3<float>* __output);
	};
}
