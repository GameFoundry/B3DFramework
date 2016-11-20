//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptCollider.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for CapsuleCollider. */
	class BS_SCR_BE_EXPORT ScriptCapsuleCollider : public TScriptCollider<ScriptCapsuleCollider>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeCapsuleCollider")
	private:
		ScriptCapsuleCollider(MonoObject* instance, const SPtr<Collider>& collider);

		/** Returns the native capsule collider wrapped by this object. */
		CapsuleCollider* getCapsuleCollider() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static void internal_SetHalfHeight(ScriptCapsuleCollider* thisPtr, float halfHeight);
		static float internal_GetHalfHeight(ScriptCapsuleCollider* thisPtr);
		static void internal_SetRadius(ScriptCapsuleCollider* thisPtr, float radius);
		static float internal_GetRadius(ScriptCapsuleCollider* thisPtr);
	};

	/** @} */
}