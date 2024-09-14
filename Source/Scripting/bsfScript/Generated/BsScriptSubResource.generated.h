//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Importer/BsImporter.h"

namespace bs
{
#if !B3D_IS_ENGINE
	struct __SubResourceInterop
	{
		MonoString* Name;
		MonoObject* Value;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptSubResource : public TScriptTypeDefinition<ScriptSubResource>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SubResource")

		static MonoObject* Box(const __SubResourceInterop& value);
		static __SubResourceInterop Unbox(MonoObject* value);
		static SubResource FromInterop(const __SubResourceInterop& value);
		static __SubResourceInterop ToInterop(const SubResource& value);

	private:
		ScriptSubResource();

	};
#endif
}
