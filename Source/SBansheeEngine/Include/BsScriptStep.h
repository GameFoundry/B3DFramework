//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
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