//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Image/BsPixelUtility.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMipMapGenOptions : public TScriptTypeDefinition<ScriptMipMapGenOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MipMapGenOptions")

		static MonoObject* Box(const MipMapGenOptions& value);
		static MipMapGenOptions Unbox(MonoObject* value);

	private:
		ScriptMipMapGenOptions();

	};
}
