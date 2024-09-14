//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CPlaneCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlaneCollider : public TScriptGameObjectWrapper<CPlaneCollider, ScriptPlaneCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PlaneCollider")

		ScriptPlaneCollider(const GameObjectHandle<CPlaneCollider>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetNormal(ScriptPlaneCollider* self, TVector3<float>* normal);
		static void InternalGetNormal(ScriptPlaneCollider* self, TVector3<float>* __output);
		static void InternalSetDistance(ScriptPlaneCollider* self, float distance);
		static float InternalGetDistance(ScriptPlaneCollider* self);
	};
}
