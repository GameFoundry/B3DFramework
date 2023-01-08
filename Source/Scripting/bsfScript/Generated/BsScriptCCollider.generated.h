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
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptCollider : public TScriptComponent<ScriptCollider, CCollider, ScriptColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Collider")

		ScriptCollider(MonoObject* managedInstance, const GameObjectHandle<CCollider>& value);

	private:
		void OnCollisionBegin(const CollisionData& p0);
		void OnCollisionStay(const CollisionData& p0);
		void OnCollisionEnd(const CollisionData& p0);

		typedef void(B3D_THUNKCALL *OnCollisionBeginThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionBeginThunkDef OnCollisionBeginThunk;
		typedef void(B3D_THUNKCALL *OnCollisionStayThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionStayThunkDef OnCollisionStayThunk;
		typedef void(B3D_THUNKCALL *OnCollisionEndThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static OnCollisionEndThunkDef OnCollisionEndThunk;

		static void InternalSetIsTrigger(ScriptColliderBase* thisPtr, bool value);
		static bool InternalGetIsTrigger(ScriptColliderBase* thisPtr);
		static void InternalSetMass(ScriptColliderBase* thisPtr, float mass);
		static float InternalGetMass(ScriptColliderBase* thisPtr);
		static void InternalSetMaterial(ScriptColliderBase* thisPtr, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptColliderBase* thisPtr);
		static void InternalSetContactOffset(ScriptColliderBase* thisPtr, float value);
		static float InternalGetContactOffset(ScriptColliderBase* thisPtr);
		static void InternalSetRestOffset(ScriptColliderBase* thisPtr, float value);
		static float InternalGetRestOffset(ScriptColliderBase* thisPtr);
		static void InternalSetLayer(ScriptColliderBase* thisPtr, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptColliderBase* thisPtr);
		static void InternalSetCollisionReportMode(ScriptColliderBase* thisPtr, CollisionReportMode mode);
		static CollisionReportMode InternalGetCollisionReportMode(ScriptColliderBase* thisPtr);
	};
}
