//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Engine/Core/Components/B3DReflectionProbe.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"

namespace b3d { class ReflectionProbe; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptReflectionProbe : public TScriptGameObjectWrapper<ReflectionProbe, ScriptReflectionProbe>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ReflectionProbe")

		ScriptReflectionProbe(const TGameObjectHandle<ReflectionProbe>& nativeObject);
		~ScriptReflectionProbe();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetType(ScriptReflectionProbe* self, ReflectionProbeType type);
		static void InternalSetRadius(ScriptReflectionProbe* self, float radius);
		static void InternalSetExtents(ScriptReflectionProbe* self, TVector3<float>* extents);
		static void InternalSetCustomTexture(ScriptReflectionProbe* self, MonoObject* texture);
		static MonoObject* InternalGetCustomTexture(ScriptReflectionProbe* self);
		static float InternalGetWorldRadius(ScriptReflectionProbe* self);
		static void InternalGetWorldExtents(ScriptReflectionProbe* self, TVector3<float>* __output);
		static void InternalCapture(ScriptReflectionProbe* self);
		static ReflectionProbeType InternalGetType(ScriptReflectionProbe* self);
		static float InternalGetRadius(ScriptReflectionProbe* self);
		static void InternalGetExtents(ScriptReflectionProbe* self, TVector3<float>* __output);
	};
}
