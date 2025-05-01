//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace bs { class CSphereCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSphereCollider : public TScriptGameObjectWrapper<CSphereCollider, ScriptSphereCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SphereCollider")

		ScriptSphereCollider(const GameObjectHandle<CSphereCollider>& nativeObject);
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
