//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptCollider.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d { class PlaneCollider; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlaneCollider : public TScriptGameObjectWrapper<PlaneCollider, ScriptPlaneCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PlaneCollider")

		ScriptPlaneCollider(const TGameObjectHandle<PlaneCollider>& nativeObject);
		~ScriptPlaneCollider();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetNormal(ScriptPlaneCollider* self, TVector3<float>* normal);
		static void InternalGetNormal(ScriptPlaneCollider* self, TVector3<float>* __output);
		static void InternalSetDistance(ScriptPlaneCollider* self, float distance);
		static float InternalGetDistance(ScriptPlaneCollider* self);
	};
}
