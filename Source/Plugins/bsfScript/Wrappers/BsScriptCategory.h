//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/**	Interop class between C++ & CLR for the Category attribute. */
	class BS_SCR_BE_EXPORT ScriptCategory : public ScriptObject <ScriptCategory>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Category")

		static MonoField* getNameField() { return nameField; }

	private:
		ScriptCategory(MonoObject* instance);

		static MonoField* nameField;
	};
}