//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCollider.generated.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace b3d { class CapsuleCollider; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCapsuleCollider : public TScriptGameObjectWrapper<CapsuleCollider, ScriptCapsuleCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CapsuleCollider")

		ScriptCapsuleCollider(const GameObjectHandle<CapsuleCollider>& nativeObject);
		~ScriptCapsuleCollider();

		static void SetupScriptBindings();

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
