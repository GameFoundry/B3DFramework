//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptCollider.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d { class SphereCollider; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSphereCollider : public TScriptGameObjectWrapper<SphereCollider, ScriptSphereCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SphereCollider")

		ScriptSphereCollider(const TGameObjectHandle<SphereCollider>& nativeObject);
		~ScriptSphereCollider();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetRadius(ScriptSphereCollider* self, float radius);
		static float InternalGetRadius(ScriptSphereCollider* self);
		static void InternalSetCenter(ScriptSphereCollider* self, TVector3<float>* center);
		static void InternalGetCenter(ScriptSphereCollider* self, TVector3<float>* __output);
	};
}
