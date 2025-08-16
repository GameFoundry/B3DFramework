//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace b3d { class PlaneCollider; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlaneCollider : public TScriptGameObjectWrapper<PlaneCollider, ScriptPlaneCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PlaneCollider")

		ScriptPlaneCollider(const GameObjectHandle<PlaneCollider>& nativeObject);
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
