//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/**	Interop class between C++ & CLR for Step attribute. */
	class BS_SCR_BE_EXPORT ScriptStep : public ScriptObject <ScriptStep>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Step")

		static MonoField* getStepField() { return stepField; }

	private:
		ScriptStep(MonoObject* instance);

		static MonoField* stepField;
	};
}