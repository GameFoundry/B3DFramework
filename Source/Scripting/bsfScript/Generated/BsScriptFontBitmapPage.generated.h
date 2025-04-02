//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"

namespace bs
{
	struct __FontBitmapPageInterop
	{
		MonoObject* Texture;
		FontBitmapPageType Type;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptFontBitmapPage : public TScriptTypeDefinition<ScriptFontBitmapPage>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "FontBitmapPage")

		static MonoObject* Box(const __FontBitmapPageInterop& value);
		static __FontBitmapPageInterop Unbox(MonoObject* value);
		static FontBitmapPage FromInterop(const __FontBitmapPageInterop& value);
		static __FontBitmapPageInterop ToInterop(const FontBitmapPage& value);

	private:
		ScriptFontBitmapPage();

	};
}
