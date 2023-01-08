//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Renderer/BsReflectionProbe.h"
#include "Math/BsVector3.h"

namespace bs { class CReflectionProbe; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptReflectionProbe : public TScriptComponent<ScriptReflectionProbe, CReflectionProbe>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ReflectionProbe")

		ScriptReflectionProbe(MonoObject* managedInstance, const GameObjectHandle<CReflectionProbe>& value);

	private:
		static ReflectionProbeType InternalGetType(ScriptReflectionProbe* thisPtr);
		static void InternalSetType(ScriptReflectionProbe* thisPtr, ReflectionProbeType type);
		static float InternalGetRadius(ScriptReflectionProbe* thisPtr);
		static void InternalSetRadius(ScriptReflectionProbe* thisPtr, float radius);
		static void InternalGetExtents(ScriptReflectionProbe* thisPtr, Vector3* __output);
		static void InternalSetExtents(ScriptReflectionProbe* thisPtr, Vector3* extents);
		static MonoObject* InternalGetCustomTexture(ScriptReflectionProbe* thisPtr);
		static void InternalSetCustomTexture(ScriptReflectionProbe* thisPtr, MonoObject* texture);
		static void InternalCapture(ScriptReflectionProbe* thisPtr);
	};
}
