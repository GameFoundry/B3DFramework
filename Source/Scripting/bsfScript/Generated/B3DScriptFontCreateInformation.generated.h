//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Text/B3DFont.h"
#include "../../../Foundation/Core/Text/B3DFont.h"

namespace b3d
{
	struct __FontCreateInformationInterop
	{
		MonoString* Name;
		uint32_t DPI;
		FontRenderMode RenderMode;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptFontCreateInformation : public TScriptTypeDefinition<ScriptFontCreateInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FontCreateInformation")

		static MonoObject* Box(const __FontCreateInformationInterop& value);
		static __FontCreateInformationInterop Unbox(MonoObject* value);
		static FontCreateInformation FromInterop(const __FontCreateInformationInterop& value);
		static __FontCreateInformationInterop ToInterop(const FontCreateInformation& value);

	private:
		ScriptFontCreateInformation();

	};
}
