//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/Importer/B3DImporter.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Importer/B3DImporter.h"

namespace b3d { struct __SubResourceInterop; }
namespace b3d
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMultiResource : public TScriptNonReflectableWrapper<MultiResource, ScriptMultiResource>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MultiResource")

		ScriptMultiResource(const TShared<MultiResource>& nativeObject);
		~ScriptMultiResource();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalMultiResource(MonoObject* scriptObject);
		static void InternalMultiResource0(MonoObject* scriptObject, MonoArray* entries);
		static MonoArray* InternalGetEntries(ScriptMultiResource* self);
		static void InternalSetEntries(ScriptMultiResource* self, MonoArray* value);
	};
#endif
}
