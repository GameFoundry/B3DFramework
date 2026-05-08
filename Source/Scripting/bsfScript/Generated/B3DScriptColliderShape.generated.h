//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Physics/B3DColliderShape.h"
#include "../../../Engine/Core/Physics/B3DPhysicsCommon.h"
#include "../../../Engine/Core/Physics/B3DColliderShape.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"
#include "../../../Engine/Core/Physics/B3DColliderShape.h"
#include "../../../Engine/Utility/Math/B3DQuaternion.h"
#include "../../../Engine/Core/Physics/B3DColliderShape.h"
#include "../../../Engine/Core/Physics/B3DColliderShape.h"
#include "../../../Engine/Core/Physics/B3DColliderShape.h"
#include "../../../Engine/Core/Physics/B3DColliderShape.h"

namespace b3d { struct __BoxColliderShapeInformationInterop; }
namespace b3d { class ColliderShape; }
namespace b3d { struct __MeshColliderShapeInformationInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColliderShape : public TScriptReflectableWrapper<ColliderShape, ScriptColliderShape>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ColliderShape")

		ScriptColliderShape(const TShared<ColliderShape>& nativeObject);
		~ScriptColliderShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static ColliderShapeType InternalGetType(ScriptColliderShape* self);
		static void InternalSetPosition(ScriptColliderShape* self, TVector3<float>* position);
		static void InternalGetPosition(ScriptColliderShape* self, TVector3<float>* __output);
		static void InternalSetRotation(ScriptColliderShape* self, TQuaternion<float>* rotation);
		static void InternalGetRotation(ScriptColliderShape* self, TQuaternion<float>* __output);
		static void InternalSetScale(ScriptColliderShape* self, TVector3<float>* scale);
		static void InternalGetScale(ScriptColliderShape* self, TVector3<float>* __output);
		static void InternalSetIsTrigger(ScriptColliderShape* self, bool value);
		static bool InternalGetIsTrigger(ScriptColliderShape* self);
		static void InternalSetMass(ScriptColliderShape* self, float mass);
		static float InternalGetMass(ScriptColliderShape* self);
		static void InternalSetMaterial(ScriptColliderShape* self, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptColliderShape* self);
		static void InternalSetContactOffset(ScriptColliderShape* self, float value);
		static float InternalGetContactOffset(ScriptColliderShape* self);
		static void InternalSetRestOffset(ScriptColliderShape* self, float value);
		static float InternalGetRestOffset(ScriptColliderShape* self);
		static void InternalSetLayer(ScriptColliderShape* self, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptColliderShape* self);
		static void InternalSetCollisionReportMode(ScriptColliderShape* self, CollisionReportMode mode);
		static CollisionReportMode InternalGetCollisionReportMode(ScriptColliderShape* self);
		static void InternalSetShape(ScriptColliderShape* self, PlaneColliderShapeInformation* information);
		static void InternalSetShape0(ScriptColliderShape* self, __BoxColliderShapeInformationInterop* information);
		static void InternalSetShape1(ScriptColliderShape* self, SphereColliderShapeInformation* information);
		static void InternalSetShape2(ScriptColliderShape* self, CapsuleColliderShapeInformation* information);
		static void InternalSetShape3(ScriptColliderShape* self, __MeshColliderShapeInformationInterop* information);
		static void InternalGetPlaneShapeInformation(ScriptColliderShape* self, PlaneColliderShapeInformation* __output);
		static void InternalGetBoxShapeInformation(ScriptColliderShape* self, __BoxColliderShapeInformationInterop* __output);
		static void InternalGetSphereShapeInformation(ScriptColliderShape* self, SphereColliderShapeInformation* __output);
		static void InternalGetCapsuleShapeInformation(ScriptColliderShape* self, CapsuleColliderShapeInformation* __output);
		static void InternalGetMeshShapeInformation(ScriptColliderShape* self, __MeshColliderShapeInformationInterop* __output);
		static void InternalCreatePlane(MonoObject* scriptObject, PlaneColliderShapeInformation* information);
		static void InternalCreateBox(MonoObject* scriptObject, __BoxColliderShapeInformationInterop* information);
		static void InternalCreateSphere(MonoObject* scriptObject, SphereColliderShapeInformation* information);
		static void InternalCreateCapsule(MonoObject* scriptObject, CapsuleColliderShapeInformation* information);
		static void InternalCreateMesh(MonoObject* scriptObject, __MeshColliderShapeInformationInterop* information);
	};
}
