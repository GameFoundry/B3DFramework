//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsBounds.h"

namespace bs { class CRenderable; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderable : public TScriptComponent<ScriptRenderable, CRenderable>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Renderable")

		ScriptRenderable(MonoObject* managedInstance, const GameObjectHandle<CRenderable>& value);

	private:
		static void InternalSetMesh(ScriptRenderable* self, MonoObject* mesh);
		static MonoObject* InternalGetMesh(ScriptRenderable* self);
		static void InternalSetMaterial(ScriptRenderable* self, uint32_t idx, MonoObject* material);
		static void InternalSetMaterial0(ScriptRenderable* self, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptRenderable* self, uint32_t idx);
		static void InternalSetMaterials(ScriptRenderable* self, MonoArray* materials);
		static MonoArray* InternalGetMaterials(ScriptRenderable* self);
		static void InternalSetCullDistanceFactor(ScriptRenderable* self, float factor);
		static float InternalGetCullDistanceFactor(ScriptRenderable* self);
		static void InternalSetWriteVelocity(ScriptRenderable* self, bool enable);
		static bool InternalGetWriteVelocity(ScriptRenderable* self);
		static void InternalSetLayer(ScriptRenderable* self, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptRenderable* self);
		static void InternalGetBounds(ScriptRenderable* self, Bounds* __output);
	};
}
