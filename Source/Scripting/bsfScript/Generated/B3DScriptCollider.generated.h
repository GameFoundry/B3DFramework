//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsCommon.h"
#include "../../../Foundation/Core/Physics/B3DPhysicsCommon.h"

namespace b3d { class Collider; }
namespace b3d { struct __CollisionDataInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColliderWrapperBase : public ScriptGameObjectWrapper
	{
	public:
		using ScriptGameObjectWrapper::ScriptGameObjectWrapper;

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		void OnCollisionBegin(const CollisionData& p0);
		void OnCollisionStay(const CollisionData& p0);
		void OnCollisionEnd(const CollisionData& p0);

		typedef void(B3D_THUNKCALL *OnCollisionBeginThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionBeginThunkDefinition OnCollisionBeginThunk;
		typedef void(B3D_THUNKCALL *OnCollisionStayThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionStayThunkDefinition OnCollisionStayThunk;
		typedef void(B3D_THUNKCALL *OnCollisionEndThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionEndThunkDefinition OnCollisionEndThunk;

		HEvent OnCollisionBeginConnection;
		HEvent OnCollisionStayConnection;
		HEvent OnCollisionEndConnection;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptCollider : public TScriptGameObjectWrapper<Collider, ScriptCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Collider")

		ScriptCollider(const TGameObjectHandle<Collider>& nativeObject);
		~ScriptCollider();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetIsTrigger(ScriptColliderWrapperBase* self, bool value);
		static bool InternalGetIsTrigger(ScriptColliderWrapperBase* self);
		static void InternalSetMass(ScriptColliderWrapperBase* self, float mass);
		static float InternalGetMass(ScriptColliderWrapperBase* self);
		static void InternalSetMaterial(ScriptColliderWrapperBase* self, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptColliderWrapperBase* self);
		static void InternalSetContactOffset(ScriptColliderWrapperBase* self, float value);
		static float InternalGetContactOffset(ScriptColliderWrapperBase* self);
		static void InternalSetRestOffset(ScriptColliderWrapperBase* self, float value);
		static float InternalGetRestOffset(ScriptColliderWrapperBase* self);
		static void InternalSetLayer(ScriptColliderWrapperBase* self, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptColliderWrapperBase* self);
		static void InternalSetCollisionReportMode(ScriptColliderWrapperBase* self, CollisionReportMode mode);
		static CollisionReportMode InternalGetCollisionReportMode(ScriptColliderWrapperBase* self);
	};
}
