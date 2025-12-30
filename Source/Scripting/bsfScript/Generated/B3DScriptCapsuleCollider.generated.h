//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptCollider.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d { class CapsuleCollider; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCapsuleCollider : public TScriptGameObjectWrapper<CapsuleCollider, ScriptCapsuleCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CapsuleCollider")

		ScriptCapsuleCollider(const TGameObjectHandle<CapsuleCollider>& nativeObject);
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
