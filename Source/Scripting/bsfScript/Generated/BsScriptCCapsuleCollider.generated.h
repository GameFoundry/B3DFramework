//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CCapsuleCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCapsuleCollider : public TScriptGameObjectWrapper<CCapsuleCollider, ScriptCapsuleCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "CapsuleCollider")

		ScriptCapsuleCollider(const GameObjectHandle<CCapsuleCollider>& nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetNormal(ScriptCapsuleCollider* self, TVector3<float>* normal);
		static void InternalGetNormal(ScriptCapsuleCollider* self, TVector3<float>* __output);
		static void InternalSetCenter(ScriptCapsuleCollider* self, TVector3<float>* center);
		static void InternalGetCenter(ScriptCapsuleCollider* self, TVector3<float>* __output);
		static void InternalSetHalfHeight(ScriptCapsuleCollider* self, float halfHeight);
		static float InternalGetHalfHeight(ScriptCapsuleCollider* self);
		static void InternalSetRadius(ScriptCapsuleCollider* self, float radius);
		static float InternalGetRadius(ScriptCapsuleCollider* self);
	};
}
