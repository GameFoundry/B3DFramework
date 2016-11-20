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

	/** Interop class between C++ & CLR for PlaneCollider. */
	class BS_SCR_BE_EXPORT ScriptPlaneCollider : public TScriptCollider<ScriptPlaneCollider>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativePlaneCollider")

	private:
		ScriptPlaneCollider(MonoObject* instance, const SPtr<Collider>& collider);

		/** Returns the native plane collider wrapped by this object. */
		PlaneCollider* getPlaneCollider() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
	};

	/** @} */
}