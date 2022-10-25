//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsBounds.h"

namespace bs
{
	class CRenderable;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCRenderable : public TScriptComponent<ScriptCRenderable, CRenderable>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Renderable")

		ScriptCRenderable(MonoObject* managedInstance, const GameObjectHandle<CRenderable>& value);

	private:
		static void InternalSetMesh(ScriptCRenderable* thisPtr, MonoObject* mesh);
		static MonoObject* InternalGetMesh(ScriptCRenderable* thisPtr);
		static void InternalSetMaterial(ScriptCRenderable* thisPtr, uint32_t idx, MonoObject* material);
		static void InternalSetMaterial0(ScriptCRenderable* thisPtr, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptCRenderable* thisPtr, uint32_t idx);
		static void InternalSetMaterials(ScriptCRenderable* thisPtr, MonoArray* materials);
		static MonoArray* InternalGetMaterials(ScriptCRenderable* thisPtr);
		static void InternalSetCullDistanceFactor(ScriptCRenderable* thisPtr, float factor);
		static float InternalGetCullDistanceFactor(ScriptCRenderable* thisPtr);
		static void InternalSetWriteVelocity(ScriptCRenderable* thisPtr, bool enable);
		static bool InternalGetWriteVelocity(ScriptCRenderable* thisPtr);
		static void InternalSetLayer(ScriptCRenderable* thisPtr, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptCRenderable* thisPtr);
		static void InternalGetBounds(ScriptCRenderable* thisPtr, Bounds* __output);
	};
} // namespace bs
