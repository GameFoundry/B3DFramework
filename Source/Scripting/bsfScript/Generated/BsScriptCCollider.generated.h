//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"

namespace bs { struct __CollisionDataInterop; }
namespace bs { class CCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptColliderBase : public ScriptComponentBase
	{
	public:
		ScriptColliderBase(MonoObject* instance);
		virtual ~ScriptColliderBase() {}
		void OnCollisionBegin(const CollisionData& p0);
		void OnCollisionStay(const CollisionData& p0);
		void OnCollisionEnd(const CollisionData& p0);

		typedef void(B3D_THUNKCALL *OnCollisionBeginThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionBeginThunkDefinition OnCollisionBeginThunk;
		typedef void(B3D_THUNKCALL *OnCollisionStayThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionStayThunkDefinition OnCollisionStayThunk;
		typedef void(B3D_THUNKCALL *OnCollisionEndThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionEndThunkDefinition OnCollisionEndThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptCollider : public TScriptComponent<ScriptCollider, CCollider, ScriptColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Collider")

		ScriptCollider(MonoObject* managedInstance, const GameObjectHandle<CCollider>& value);

	private:
		static void InternalSetIsTrigger(ScriptColliderBase* self, bool value);
		static bool InternalGetIsTrigger(ScriptColliderBase* self);
		static void InternalSetMass(ScriptColliderBase* self, float mass);
		static float InternalGetMass(ScriptColliderBase* self);
		static void InternalSetMaterial(ScriptColliderBase* self, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptColliderBase* self);
		static void InternalSetContactOffset(ScriptColliderBase* self, float value);
		static float InternalGetContactOffset(ScriptColliderBase* self);
		static void InternalSetRestOffset(ScriptColliderBase* self, float value);
		static float InternalGetRestOffset(ScriptColliderBase* self);
		static void InternalSetLayer(ScriptColliderBase* self, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptColliderBase* self);
		static void InternalSetCollisionReportMode(ScriptColliderBase* self, CollisionReportMode mode);
		static CollisionReportMode InternalGetCollisionReportMode(ScriptColliderBase* self);
	};
}
