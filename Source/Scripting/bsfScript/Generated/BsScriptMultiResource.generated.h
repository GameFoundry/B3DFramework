//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs { struct MultiResource; }
namespace bs { struct __SubResourceInterop; }
namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptMultiResource : public ScriptObject<ScriptMultiResource>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MultiResource")

		ScriptMultiResource(MonoObject* managedInstance, const SPtr<MultiResource>& value);

		SPtr<MultiResource> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<MultiResource>& value);

	private:
		SPtr<MultiResource> mInternal;

		static void Internal_MultiResource(MonoObject* managedInstance);
		static void Internal_MultiResource0(MonoObject* managedInstance, MonoArray* entries);
		static MonoArray* Internal_getentries(ScriptMultiResource* thisPtr);
		static void Internal_setentries(ScriptMultiResource* thisPtr, MonoArray* value);
	};
#endif
}
