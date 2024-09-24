//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptTypeDefinition.h"

namespace bs
{
	/**	Interop class between C++ & CLR for the Order attribute. */
	class B3D_SCRIPT_INTEROP_EXPORT ScriptOrder : public TScriptTypeDefinition<ScriptOrder>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Order")

		ScriptOrder();
		void SetupScriptBindings();

		static MonoField* GetIndexField() { return sIndexField; }

	private:
		static MonoField* sIndexField;
	};
} // namespace bs
