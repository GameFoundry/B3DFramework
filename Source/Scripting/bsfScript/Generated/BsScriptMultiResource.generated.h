//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"

namespace bs
{
	struct MultiResource;
}

namespace bs
{
	struct __SubResourceInterop;
}

namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptMultiResource : public ScriptObject<ScriptMultiResource>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MultiResource")

		ScriptMultiResource(MonoObject* managedInstance, const SPtr<MultiResource>& value);

		SPtr<MultiResource> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<MultiResource>& value);

	private:
		SPtr<MultiResource> mInternal;

		static void InternalMultiResource(MonoObject* managedInstance);
		static void InternalMultiResource0(MonoObject* managedInstance, MonoArray* entries);
		static MonoArray* InternalGetEntries(ScriptMultiResource* thisPtr);
		static void InternalSetEntries(ScriptMultiResource* thisPtr, MonoArray* value);
	};
#endif
} // namespace bs
