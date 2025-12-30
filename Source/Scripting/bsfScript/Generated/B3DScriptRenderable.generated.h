//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "Math/B3DBounds.h"

namespace b3d { class Renderable; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderable : public TScriptGameObjectWrapper<Renderable, ScriptRenderable>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Renderable")

		ScriptRenderable(const TGameObjectHandle<Renderable>& nativeObject);
		~ScriptRenderable();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalGetBounds(ScriptRenderable* self, TBounds<float>* __output);
		static void InternalSetMesh(ScriptRenderable* self, MonoObject* mesh);
		static MonoObject* InternalGetMesh(ScriptRenderable* self);
		static void InternalSetMaterial(ScriptRenderable* self, uint32_t index, MonoObject* material);
		static MonoObject* InternalGetMaterial(ScriptRenderable* self, uint32_t index);
		static void InternalSetMaterial0(ScriptRenderable* self, MonoObject* material);
		static MonoArray* InternalGetMaterials(ScriptRenderable* self);
		static void InternalSetMaterials(ScriptRenderable* self, MonoArray* materials);
		static void InternalSetLayer(ScriptRenderable* self, uint64_t layer);
		static void InternalSetWriteVelocity(ScriptRenderable* self, bool enable);
		static bool InternalGetWriteVelocity(ScriptRenderable* self);
		static void InternalSetCullDistanceFactor(ScriptRenderable* self, float factor);
		static float InternalGetCullDistanceFactor(ScriptRenderable* self);
		static uint64_t InternalGetLayer(ScriptRenderable* self);
	};
}
