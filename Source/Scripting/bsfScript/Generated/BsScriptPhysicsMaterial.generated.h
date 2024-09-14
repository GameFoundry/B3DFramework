//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"

namespace bs { class PhysicsMaterial; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysicsMaterial : public TScriptResourceWrapper<PhysicsMaterial, ScriptPhysicsMaterial>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PhysicsMaterial")

		ScriptPhysicsMaterial(const TResourceHandle<PhysicsMaterial>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptPhysicsMaterial* self);

		static void InternalSetStaticFriction(ScriptPhysicsMaterial* self, float value);
		static float InternalGetStaticFriction(ScriptPhysicsMaterial* self);
		static void InternalSetDynamicFriction(ScriptPhysicsMaterial* self, float value);
		static float InternalGetDynamicFriction(ScriptPhysicsMaterial* self);
		static void InternalSetRestitutionCoefficient(ScriptPhysicsMaterial* self, float value);
		static float InternalGetRestitutionCoefficient(ScriptPhysicsMaterial* self);
		static void InternalCreate(MonoObject* scriptObject, float staticFriction, float dynamicFriction, float restitution);
	};
}
