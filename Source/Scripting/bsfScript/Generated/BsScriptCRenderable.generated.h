//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsBounds.h"

namespace bs { class CRenderable; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCRenderable : public TScriptComponent<ScriptCRenderable, CRenderable>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Renderable")

		ScriptCRenderable(MonoObject* managedInstance, const GameObjectHandle<CRenderable>& value);

	private:
		static void Internal_setMesh(ScriptCRenderable* thisPtr, MonoObject* mesh);
		static MonoObject* Internal_getMesh(ScriptCRenderable* thisPtr);
		static void Internal_setMaterial(ScriptCRenderable* thisPtr, uint32_t idx, MonoObject* material);
		static void Internal_setMaterial0(ScriptCRenderable* thisPtr, MonoObject* material);
		static MonoObject* Internal_getMaterial(ScriptCRenderable* thisPtr, uint32_t idx);
		static void Internal_setMaterials(ScriptCRenderable* thisPtr, MonoArray* materials);
		static MonoArray* Internal_getMaterials(ScriptCRenderable* thisPtr);
		static void Internal_setCullDistanceFactor(ScriptCRenderable* thisPtr, float factor);
		static float Internal_getCullDistanceFactor(ScriptCRenderable* thisPtr);
		static void Internal_setWriteVelocity(ScriptCRenderable* thisPtr, bool enable);
		static bool Internal_getWriteVelocity(ScriptCRenderable* thisPtr);
		static void Internal_setLayer(ScriptCRenderable* thisPtr, uint64_t layer);
		static uint64_t Internal_getLayer(ScriptCRenderable* thisPtr);
		static void Internal_getBounds(ScriptCRenderable* thisPtr, Bounds* __output);
	};
}
