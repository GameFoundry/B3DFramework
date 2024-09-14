//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"

namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCharRange : public TScriptStructWrapper<ScriptCharRange>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CharRange")

		static MonoObject* Box(const CharRange& value);
		static CharRange Unbox(MonoObject* value);

	private:
		ScriptCharRange();

	};
#endif
}
