//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Utility/Math/B3DVector2.h"

namespace b3d { class Decal; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDecal : public TScriptGameObjectWrapper<Decal, ScriptDecal>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Decal")

		ScriptDecal(const TGameObjectHandle<Decal>& nativeObject);
		~ScriptDecal();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSize(ScriptDecal* self, TVector2<float>* size);
		static void InternalGetSize(ScriptDecal* self, TVector2<float>* __output);
		static void InternalSetMaterial(ScriptDecal* self, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptDecal* self);
		static void InternalSetMaxDistance(ScriptDecal* self, float distance);
		static float InternalGetMaxDistance(ScriptDecal* self);
		static void InternalSetLayerMask(ScriptDecal* self, uint32_t mask);
		static uint32_t InternalGetLayerMask(ScriptDecal* self);
		static void InternalSetLayer(ScriptDecal* self, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptDecal* self);
	};
}
