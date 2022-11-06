//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"

namespace bs { class PhysicsMaterial; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysicsMaterial : public TScriptResource<ScriptPhysicsMaterial, PhysicsMaterial>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PhysicsMaterial")

		ScriptPhysicsMaterial(MonoObject* managedInstance, const ResourceHandle<PhysicsMaterial>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptPhysicsMaterial* thisPtr);

		static void InternalSetStaticFriction(ScriptPhysicsMaterial* thisPtr, float value);
		static float InternalGetStaticFriction(ScriptPhysicsMaterial* thisPtr);
		static void InternalSetDynamicFriction(ScriptPhysicsMaterial* thisPtr, float value);
		static float InternalGetDynamicFriction(ScriptPhysicsMaterial* thisPtr);
		static void InternalSetRestitutionCoefficient(ScriptPhysicsMaterial* thisPtr, float value);
		static float InternalGetRestitutionCoefficient(ScriptPhysicsMaterial* thisPtr);
		static void InternalCreate(MonoObject* managedInstance, float staticFriction, float dynamicFriction, float restitution);
	};
}
