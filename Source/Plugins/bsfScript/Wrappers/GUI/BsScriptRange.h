//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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