//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysicsCommon.h"

namespace bs { struct __CollisionDataInterop; }
namespace bs { class CCollider; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCColliderBase : public ScriptComponentBase
	{
	public:
		ScriptCColliderBase(MonoObject* instance);
		virtual ~ScriptCColliderBase() {}
	};

	class BS_SCR_BE_EXPORT ScriptCCollider : public TScriptComponent<ScriptCCollider, CCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Collider")

		ScriptCCollider(MonoObject* managedInstance, const GameObjectHandle<CCollider>& value);

	private:
		void onCollisionBegin(const CollisionData& p0);
		void onCollisionStay(const CollisionData& p0);
		void onCollisionEnd(const CollisionData& p0);

		typedef void(BS_THUNKCALL *onCollisionBeginThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static onCollisionBeginThunkDef onCollisionBeginThunk;
		typedef void(BS_THUNKCALL *onCollisionStayThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static onCollisionStayThunkDef onCollisionStayThunk;
		typedef void(BS_THUNKCALL *onCollisionEndThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static onCollisionEndThunkDef onCollisionEndThunk;

		static void Internal_setIsTrigger(ScriptCColliderBase* thisPtr, bool value);
		static bool Internal_getIsTrigger(ScriptCColliderBase* thisPtr);
		static void Internal_setMass(ScriptCColliderBase* thisPtr, float mass);
		static float Internal_getMass(ScriptCColliderBase* thisPtr);
		static void Internal_setMaterial(ScriptCColliderBase* thisPtr, MonoObject* material);
		static MonoObject* Internal_getMaterial(ScriptCColliderBase* thisPtr);
		static void Internal_setContactOffset(ScriptCColliderBase* thisPtr, float value);
		static float Internal_getContactOffset(ScriptCColliderBase* thisPtr);
		static void Internal_setRestOffset(ScriptCColliderBase* thisPtr, float value);
		static float Internal_getRestOffset(ScriptCColliderBase* thisPtr);
		static void Internal_setLayer(ScriptCColliderBase* thisPtr, uint64_t layer);
		static uint64_t Internal_getLayer(ScriptCColliderBase* thisPtr);
		static void Internal_setCollisionReportMode(ScriptCColliderBase* thisPtr, CollisionReportMode mode);
		static CollisionReportMode Internal_getCollisionReportMode(ScriptCColliderBase* thisPtr);
	};
}
