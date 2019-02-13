//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptCharRange : public ScriptObject<ScriptCharRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "CharRange")

		static MonoObject* box(const CharRange& value);
		static CharRange unbox(MonoObject* value);

	private:
		ScriptCharRange(MonoObject* managedInstance);

	};
#endif
}
