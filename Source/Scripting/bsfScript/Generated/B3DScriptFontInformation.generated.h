//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Text/B3DFont.h"
#include "../../../Foundation/Core/Text/B3DFont.h"

namespace b3d
{
	struct __FontInformationInterop
	{
		MonoString* Name;
		uint32_t DPI;
		FontRenderMode RenderMode;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptFontInformation : public TScriptTypeDefinition<ScriptFontInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FontInformation")

		static MonoObject* Box(const __FontInformationInterop& value);
		static __FontInformationInterop Unbox(MonoObject* value);
		static FontInformation FromInterop(const __FontInformationInterop& value);
		static __FontInformationInterop ToInterop(const FontInformation& value);

	private:
		ScriptFontInformation();

	};
}
