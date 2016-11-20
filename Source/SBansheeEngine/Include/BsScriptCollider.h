//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsPhysicsCommon.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	class ScriptCollider;

	/** Base class for all Collider interop objects. */
	class BS_SCR_BE_EXPORT ScriptColliderBase : public ScriptObjectBase
	{
	public:
		/** Returns the native Collider object. */
		virtual Collider* getCollider() const { return mCollider.get(); };
	protected:
		friend ScriptCollider;

		ScriptColliderBase(MonoObject* instance);
		virtual ~ScriptColliderBase() {}

		/** Initializes the interop object with a native collider. Must be called right after construction. */
		void initialize(const SPtr<Collider>& collider);

		/** Destroys the internal collider object. */
		void destroyCollider();

		SPtr<Collider> mCollider;
	};

	/** A more specialized version of ScriptObject that allows the constructor to set the native collider. */
	template <class Type>
	class TScriptCollider : public ScriptObject<Type, ScriptColliderBase>
	{
	public:
		virtual ~TScriptCollider() {}

	protected:
		TScriptCollider(MonoObject* instance, const SPtr<Collider>& collider)
			:ScriptObject<Type, ScriptColliderBase>(instance)
		{
			this->initialize(collider);
		}
	};

	/** Interop class between C++ & CLR for Collider. */
	class BS_SCR_BE_EXPORT ScriptCollider : public TScriptCollider<ScriptCollider>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeCollider")

	private:
		friend class ScriptColliderBase;

		ScriptCollider(MonoObject* instance);

		/** Triggered when some object starts interacting with the collider. */
		static void onCollisionBegin(MonoObject* instance, const CollisionData& collisionData);

		/** Triggered when some object remains interacting with the collider throughout a frame. */
		static void onCollisionStay(MonoObject* instance, const CollisionData& collisionData);

		/** Triggered when some object ends interacting with the collider. */
		static void onCollisionEnd(MonoObject* instance, const CollisionData& collisionData);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Destroy(ScriptColliderBase* thisPtr);
		
		static void internal_GetPosition(ScriptColliderBase* thisPtr, Vector3* pos);
		static void internal_GetRotation(ScriptColliderBase* thisPtr, Quaternion* rot);
		static void internal_SetTransform(ScriptColliderBase* thisPtr, Vector3* pos, Quaternion* rot);

		static void internal_SetScale(ScriptColliderBase* thisPtr, Vector3* scale);
		static void internal_GetScale(ScriptColliderBase* thisPtr, Vector3* scale);

		static void internal_SetIsTrigger(ScriptColliderBase* thisPtr, bool value);
		static bool internal_GetIsTrigger(ScriptColliderBase* thisPtr);

		static void internal_SetRigidbody(ScriptColliderBase* thisPtr, ScriptRigidbody* value);
		static MonoObject* internal_GetRigidbody(ScriptColliderBase* thisPtr);

		static void internal_SetMass(ScriptColliderBase* thisPtr, float mass);
		static float internal_GetMass(ScriptColliderBase* thisPtr);

		static void internal_SetMaterial(ScriptColliderBase* thisPtr, ScriptPhysicsMaterial* material);
		static MonoObject* internal_GetMaterial(ScriptColliderBase* thisPtr);

		static void internal_SetContactOffset(ScriptColliderBase* thisPtr, float value);
		static float internal_GetContactOffset(ScriptColliderBase* thisPtr);

		static void internal_SetRestOffset(ScriptColliderBase* thisPtr, float value);
		static float internal_GetRestOffset(ScriptColliderBase* thisPtr);

		static void internal_SetLayer(ScriptColliderBase* thisPtr, UINT64 layer);
		static UINT64 internal_GetLayer(ScriptColliderBase* thisPtr);

		static void internal_SetCollisionReportMode(ScriptColliderBase* thisPtr, CollisionReportMode mode);
		static CollisionReportMode internal_GetCollisionReportMode(ScriptColliderBase* thisPtr);

		static bool internal_RayCast(ScriptColliderBase* thisPtr, Vector3* origin, Vector3* unitDir, MonoObject** hit, 
			float maxDist);

		typedef void(__stdcall *OnCollisionThunkDef) (MonoObject*, MonoObject*, MonoException**);

		static OnCollisionThunkDef onCollisionBeginThunk;
		static OnCollisionThunkDef onCollisionStayThunk;
		static OnCollisionThunkDef onCollisionEndThunk;
	};

	/** @} */
}