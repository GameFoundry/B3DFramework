//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"

namespace bs { struct __SubResourceInterop; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMultiResource : public ScriptObject<ScriptMultiResource>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MultiResource")

		ScriptMultiResource(MonoObject* managedInstance, const SPtr<MultiResource>& value);

		SPtr<MultiResource> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<MultiResource>& value);

	private:
		SPtr<MultiResource> mInternal;

		static void InternalMultiResource(MonoObject* managedInstance);
		static void InternalMultiResource0(MonoObject* managedInstance, MonoArray* entries);
		static MonoArray* InternalGetEntries(ScriptMultiResource* self);
		static void InternalSetEntries(ScriptMultiResource* self, MonoArray* value);
	};
#endif
}
