//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsVector2.h"

namespace bs
{
	class CDecal;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCDecal : public TScriptComponent<ScriptCDecal, CDecal>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Decal")

		ScriptCDecal(MonoObject* managedInstance, const GameObjectHandle<CDecal>& value);

	private:
		static void InternalSetMaterial(ScriptCDecal* thisPtr, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptCDecal* thisPtr);
		static void InternalSetSize(ScriptCDecal* thisPtr, Vector2* size);
		static void InternalGetSize(ScriptCDecal* thisPtr, Vector2* __output);
		static void InternalSetMaxDistance(ScriptCDecal* thisPtr, float distance);
		static float InternalGetMaxDistance(ScriptCDecal* thisPtr);
		static void InternalSetLayer(ScriptCDecal* thisPtr, uint64_t layer);
		static uint64_t InternalGetLayer(ScriptCDecal* thisPtr);
		static void InternalSetLayerMask(ScriptCDecal* thisPtr, uint32_t mask);
		static uint32_t InternalGetLayerMask(ScriptCDecal* thisPtr);
	};
} // namespace bs
