//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsVector2.h"

namespace bs { class CDecal; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDecal : public TScriptComponent<ScriptDecal, CDecal>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Decal")

		ScriptDecal(MonoObject* managedInstance, const GameObjectHandle<CDecal>& value);

	private:
		static void InternalSetMaterial(ScriptDecal* self, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptDecal* self);
		static void InternalSetSize(ScriptDecal* self, TVector2<float>* size);
		static void InternalGetSize(ScriptDecal* self, TVector2<float>* __output);
		static void InternalSetMaxDistance(ScriptDecal* self, float distance);
		static float InternalGetMaxDistance(ScriptDecal* self);
		static void InternalSetLayer(ScriptDecal* self, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptDecal* self);
		static void InternalSetLayerMask(ScriptDecal* self, uint32_t mask);
		static uint32_t InternalGetLayerMask(ScriptDecal* self);
	};
}
