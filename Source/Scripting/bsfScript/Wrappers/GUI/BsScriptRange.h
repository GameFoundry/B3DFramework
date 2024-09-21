//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptTypeDefinition.h"

namespace bs
{
	/**	Interop class between C++ & CLR for the Range attribute. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRange : public TScriptTypeDefinition<ScriptRange>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Range")

		ScriptRange();

		static void SetupScriptBindings();
		static MonoField* GetMinRangeField() { return sMinRangeField; }
		static MonoField* GetMaxRangeField() { return sMaxRangeField; }
		static MonoField* GetSliderField() { return sSliderField; }

	private:
		static MonoField* sMinRangeField;
		static MonoField* sMaxRangeField;
		static MonoField* sSliderField;
	};
} // namespace bs
