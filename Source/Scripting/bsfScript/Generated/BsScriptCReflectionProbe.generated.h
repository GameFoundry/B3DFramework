//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Renderer/BsReflectionProbe.h"
#include "Math/BsVector3.h"

namespace bs
{
	class CReflectionProbe;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCReflectionProbe : public TScriptComponent<ScriptCReflectionProbe, CReflectionProbe>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ReflectionProbe")

		ScriptCReflectionProbe(MonoObject* managedInstance, const GameObjectHandle<CReflectionProbe>& value);

	private:
		static ReflectionProbeType InternalGetType(ScriptCReflectionProbe* thisPtr);
		static void InternalSetType(ScriptCReflectionProbe* thisPtr, ReflectionProbeType type);
		static float InternalGetRadius(ScriptCReflectionProbe* thisPtr);
		static void InternalSetRadius(ScriptCReflectionProbe* thisPtr, float radius);
		static void InternalGetExtents(ScriptCReflectionProbe* thisPtr, Vector3* __output);
		static void InternalSetExtents(ScriptCReflectionProbe* thisPtr, Vector3* extents);
		static MonoObject* InternalGetCustomTexture(ScriptCReflectionProbe* thisPtr);
		static void InternalSetCustomTexture(ScriptCReflectionProbe* thisPtr, MonoObject* texture);
		static void InternalCapture(ScriptCReflectionProbe* thisPtr);
	};
} // namespace bs
