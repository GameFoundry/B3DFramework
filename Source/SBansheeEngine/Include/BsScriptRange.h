//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	/**	Interop class between C++ & CLR for the Range attribute. */
	class BS_SCR_BE_EXPORT ScriptRange : public ScriptObject <ScriptRange>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Range")

		static MonoField* getMinRangeField() { return minRangeField; }
		static MonoField* getMaxRangeField() { return maxRangeField; }
		static MonoField* getSliderField() { return sliderField; }

	private:
		ScriptRange(MonoObject* instance);

		static MonoField* minRangeField;
		static MonoField* maxRangeField;
		static MonoField* sliderField;
	};
}