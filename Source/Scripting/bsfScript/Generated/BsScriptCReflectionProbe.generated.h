//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Renderer/BsReflectionProbe.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"

namespace bs { class CReflectionProbe; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptReflectionProbe : public TScriptGameObjectWrapper<CReflectionProbe, ScriptReflectionProbe>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ReflectionProbe")

		ScriptReflectionProbe(const GameObjectHandle<CReflectionProbe>& nativeObject);
		~ScriptReflectionProbe();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static ReflectionProbeType InternalGetType(ScriptReflectionProbe* self);
		static void InternalSetType(ScriptReflectionProbe* self, ReflectionProbeType type);
		static float InternalGetRadius(ScriptReflectionProbe* self);
		static void InternalSetRadius(ScriptReflectionProbe* self, float radius);
		static void InternalGetExtents(ScriptReflectionProbe* self, TVector3<float>* __output);
		static void InternalSetExtents(ScriptReflectionProbe* self, TVector3<float>* extents);
		static MonoObject* InternalGetCustomTexture(ScriptReflectionProbe* self);
		static void InternalSetCustomTexture(ScriptReflectionProbe* self, MonoObject* texture);
		static void InternalCapture(ScriptReflectionProbe* self);
	};
}
