//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"

namespace bs { class Font; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptFont : public TScriptResource<ScriptFont, Font>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Font")

		ScriptFont(MonoObject* managedInstance, const ResourceHandle<Font>& value);

		static MonoObject* createInstance();

	private:
		static MonoObject* Internal_getRef(ScriptFont* thisPtr);

		static MonoObject* Internal_getBitmap(ScriptFont* thisPtr, uint32_t size);
		static int32_t Internal_getClosestSize(ScriptFont* thisPtr, uint32_t size);
	};
}
