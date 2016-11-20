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

	/** Interop class between C++ & CLR for BoxCollider. */
	class BS_SCR_BE_EXPORT ScriptBoxCollider : public TScriptCollider<ScriptBoxCollider>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeBoxCollider")
	private:
		ScriptBoxCollider(MonoObject* instance, const SPtr<Collider>& collider);

		/** Returns the native box collider wrapped by this object. */
		BoxCollider* getBoxCollider() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static void internal_SetExtents(ScriptBoxCollider* thisPtr, Vector3* extents);
		static void internal_GetExtents(ScriptBoxCollider* thisPtr, Vector3* extents);
	};

	/** @} */
}