//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/**	Interop class between C++ & CLR for the Order attribute. */
	class BS_SCR_BE_EXPORT ScriptOrder : public ScriptObject <ScriptOrder>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Order")

		static MonoField* getIndexField() { return indexField; }

	private:
		ScriptOrder(MonoObject* instance);

		static MonoField* indexField;
	};
}