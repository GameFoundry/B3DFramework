//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsVector2.h"

namespace bs { class CDecal; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCDecal : public TScriptComponent<ScriptCDecal, CDecal>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Decal")

		ScriptCDecal(MonoObject* managedInstance, const GameObjectHandle<CDecal>& value);

	private:
		static void Internal_setMaterial(ScriptCDecal* thisPtr, MonoObject* material);
		static MonoObject* Internal_getMaterial(ScriptCDecal* thisPtr);
		static void Internal_setSize(ScriptCDecal* thisPtr, Vector2* size);
		static void Internal_getSize(ScriptCDecal* thisPtr, Vector2* __output);
		static void Internal_setMaxDistance(ScriptCDecal* thisPtr, float distance);
		static float Internal_getMaxDistance(ScriptCDecal* thisPtr);
		static void Internal_setLayer(ScriptCDecal* thisPtr, uint64_t layer);
		static uint64_t Internal_getLayer(ScriptCDecal* thisPtr);
		static void Internal_setLayerMask(ScriptCDecal* thisPtr, uint32_t mask);
		static uint32_t Internal_getLayerMask(ScriptCDecal* thisPtr);
	};
}
