//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfEngine/Input/BsInputConfiguration.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVirtualAxisCreateInformation : public TScriptTypeDefinition<ScriptVirtualAxisCreateInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VirtualAxisCreateInformation")

		static MonoObject* Box(const VirtualAxisCreateInformation& value);
		static VirtualAxisCreateInformation Unbox(MonoObject* value);

	private:
		ScriptVirtualAxisCreateInformation();

	};
}
