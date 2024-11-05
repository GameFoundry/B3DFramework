//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"

namespace bs { struct __SubResourceInterop; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMultiResource : public TScriptNonReflectableWrapper<MultiResource, ScriptMultiResource>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MultiResource")

		ScriptMultiResource(const SPtr<MultiResource>& nativeObject);

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
