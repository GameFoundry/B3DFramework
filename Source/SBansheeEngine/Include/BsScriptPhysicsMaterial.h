//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsPhysicsMaterial.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for PhysicsMaterial. */
	class BS_SCR_BE_EXPORT ScriptPhysicsMaterial : public TScriptResource<ScriptPhysicsMaterial, PhysicsMaterial>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "PhysicsMaterial")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();

	private:
		friend class ScriptResourceManager;
		friend class BuiltinResourceTypes;

		ScriptPhysicsMaterial(MonoObject* instance, const HPhysicsMaterial& material);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, float staticFriction,
			float dynamicFriction, float restitution);
		static float internal_GetStaticFriction(ScriptPhysicsMaterial* thisPtr);
		static void internal_SetStaticFriction(ScriptPhysicsMaterial* thisPtr, float value);
		static float internal_GetDynamicFriction(ScriptPhysicsMaterial* thisPtr);
		static void internal_SetDynamicFriction(ScriptPhysicsMaterial* thisPtr, float value);
		static float internal_GetRestitution(ScriptPhysicsMaterial* thisPtr);
		static void internal_SetRestitution(ScriptPhysicsMaterial* thisPtr, float value);
	};

	/** @} */
}