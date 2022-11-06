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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCColliderBase : public ScriptComponentBase
	{
	public:
		ScriptCColliderBase(MonoObject* instance);
		virtual ~ScriptCColliderBase() {}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptCCollider : public TScriptComponent<ScriptCCollider, CCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Collider")

		ScriptCCollider(MonoObject* managedInstance, const GameObjectHandle<CCollider>& value);

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

		static void InternalSetIsTrigger(ScriptCColliderBase* thisPtr, bool value);
		static bool InternalGetIsTrigger(ScriptCColliderBase* thisPtr);
		static void InternalSetMass(ScriptCColliderBase* thisPtr, float mass);
		static float InternalGetMass(ScriptCColliderBase* thisPtr);
		static void InternalSetMaterial(ScriptCColliderBase* thisPtr, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptCColliderBase* thisPtr);
		static void InternalSetContactOffset(ScriptCColliderBase* thisPtr, float value);
		static float InternalGetContactOffset(ScriptCColliderBase* thisPtr);
		static void InternalSetRestOffset(ScriptCColliderBase* thisPtr, float value);
		static float InternalGetRestOffset(ScriptCColliderBase* thisPtr);
		static void InternalSetLayer(ScriptCColliderBase* thisPtr, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptCColliderBase* thisPtr);
		static void InternalSetCollisionReportMode(ScriptCColliderBase* thisPtr, CollisionReportMode mode);
		static CollisionReportMode InternalGetCollisionReportMode(ScriptCColliderBase* thisPtr);
	};
}
