//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Image/B3DPixelUtility.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCompressionOptions : public TScriptTypeDefinition<ScriptCompressionOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CompressionOptions")

		static MonoObject* Box(const CompressionOptions& value);
		static CompressionOptions Unbox(MonoObject* value);

	private:
		ScriptCompressionOptions();

	};
}
