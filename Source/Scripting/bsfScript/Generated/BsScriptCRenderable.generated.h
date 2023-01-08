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
		static void InternalSetMesh(ScriptRenderable* thisPtr, MonoObject* mesh);
		static MonoObject* InternalGetMesh(ScriptRenderable* thisPtr);
		static void InternalSetMaterial(ScriptRenderable* thisPtr, uint32_t idx, MonoObject* material);
		static void InternalSetMaterial0(ScriptRenderable* thisPtr, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptRenderable* thisPtr, uint32_t idx);
		static void InternalSetMaterials(ScriptRenderable* thisPtr, MonoArray* materials);
		static MonoArray* InternalGetMaterials(ScriptRenderable* thisPtr);
		static void InternalSetCullDistanceFactor(ScriptRenderable* thisPtr, float factor);
		static float InternalGetCullDistanceFactor(ScriptRenderable* thisPtr);
		static void InternalSetWriteVelocity(ScriptRenderable* thisPtr, bool enable);
		static bool InternalGetWriteVelocity(ScriptRenderable* thisPtr);
		static void InternalSetLayer(ScriptRenderable* thisPtr, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptRenderable* thisPtr);
		static void InternalGetBounds(ScriptRenderable* thisPtr, Bounds* __output);
	};
}
