//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CBoxCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBoxCollider : public TScriptGameObjectWrapper<CBoxCollider, ScriptBoxCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "BoxCollider")

		ScriptBoxCollider(const GameObjectHandle<CBoxCollider>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetExtents(ScriptBoxCollider* self, TVector3<float>* extents);
		static void InternalGetExtents(ScriptBoxCollider* self, TVector3<float>* __output);
		static void InternalSetCenter(ScriptBoxCollider* self, TVector3<float>* center);
		static void InternalGetCenter(ScriptBoxCollider* self, TVector3<float>* __output);
	};
}
