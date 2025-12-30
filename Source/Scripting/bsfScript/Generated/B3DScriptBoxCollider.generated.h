//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "B3DScriptCollider.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d { class BoxCollider; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBoxCollider : public TScriptGameObjectWrapper<BoxCollider, ScriptBoxCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "BoxCollider")

		ScriptBoxCollider(const TGameObjectHandle<BoxCollider>& nativeObject);
		~ScriptBoxCollider();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetExtents(ScriptBoxCollider* self, TVector3<float>* extents);
		static void InternalGetExtents(ScriptBoxCollider* self, TVector3<float>* __output);
		static void InternalSetCenter(ScriptBoxCollider* self, TVector3<float>* center);
		static void InternalGetCenter(ScriptBoxCollider* self, TVector3<float>* __output);
	};
}
