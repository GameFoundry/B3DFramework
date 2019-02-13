//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	struct __SubResourceInterop
	{
		MonoString* name;
		MonoObject* value;
	};

	class BS_SCR_BE_EXPORT ScriptSubResource : public ScriptObject<ScriptSubResource>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SubResource")

		static MonoObject* box(const __SubResourceInterop& value);
		static __SubResourceInterop unbox(MonoObject* value);
		static SubResource fromInterop(const __SubResourceInterop& value);
		static __SubResourceInterop toInterop(const SubResource& value);

	private:
		ScriptSubResource(MonoObject* managedInstance);

	};
#endif
}
