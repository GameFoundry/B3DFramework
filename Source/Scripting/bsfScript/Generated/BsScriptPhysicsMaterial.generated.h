//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"

namespace bs { class PhysicsMaterial; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPhysicsMaterial : public TScriptResource<ScriptPhysicsMaterial, PhysicsMaterial>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "PhysicsMaterial")

		ScriptPhysicsMaterial(MonoObject* managedInstance, const ResourceHandle<PhysicsMaterial>& value);

		static MonoObject* createInstance();

	private:
		static MonoObject* Internal_getRef(ScriptPhysicsMaterial* thisPtr);

		static void Internal_setStaticFriction(ScriptPhysicsMaterial* thisPtr, float value);
		static float Internal_getStaticFriction(ScriptPhysicsMaterial* thisPtr);
		static void Internal_setDynamicFriction(ScriptPhysicsMaterial* thisPtr, float value);
		static float Internal_getDynamicFriction(ScriptPhysicsMaterial* thisPtr);
		static void Internal_setRestitutionCoefficient(ScriptPhysicsMaterial* thisPtr, float value);
		static float Internal_getRestitutionCoefficient(ScriptPhysicsMaterial* thisPtr);
		static void Internal_create(MonoObject* managedInstance, float staticFriction, float dynamicFriction, float restitution);
	};
}
