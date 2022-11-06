//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Text/BsFontDesc.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptKerningPair : public ScriptObject<ScriptKerningPair>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "KerningPair")

		static MonoObject* Box(const KerningPair& value);
		static KerningPair Unbox(MonoObject* value);

	private:
		ScriptKerningPair(MonoObject* managedInstance);

	};
}
