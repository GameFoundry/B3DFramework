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

	/** Interop class between C++ & CLR for SphereCollider. */
	class BS_SCR_BE_EXPORT ScriptSphereCollider : public TScriptCollider<ScriptSphereCollider>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeSphereCollider")

	private:
		ScriptSphereCollider(MonoObject* instance, const SPtr<Collider>& collider);

		/** Returns the native sphere collider wrapped by this object. */
		SphereCollider* getSphereCollider() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static void internal_SetRadius(ScriptSphereCollider* thisPtr, float radius);
		static float internal_GetRadius(ScriptSphereCollider* thisPtr);
	};

	/** @} */
}