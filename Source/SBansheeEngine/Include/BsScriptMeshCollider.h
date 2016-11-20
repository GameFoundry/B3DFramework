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

	/** Interop class between C++ & CLR for MeshCollider. */
	class BS_SCR_BE_EXPORT ScriptMeshCollider : public TScriptCollider<ScriptMeshCollider>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeMeshCollider")

	private:
		ScriptMeshCollider(MonoObject* instance, const SPtr<Collider>& collider);

		/** Returns the native mesh collider wrapped by this object. */
		MeshCollider* getMeshCollider() const;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static void internal_SetMesh(ScriptMeshCollider* thisPtr, ScriptPhysicsMesh* mesh);
		static MonoObject* internal_GetMesh(ScriptMeshCollider* thisPtr);
	};

	/** @} */
}