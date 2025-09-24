//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Resources/B3DResources.h"

namespace b3d
{
#if B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptResourceLoadOptions : public TScriptTypeDefinition<ScriptResourceLoadOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ResourceLoadOptions")

		static MonoObject* Box(const ResourceLoadOptions& value);
		static ResourceLoadOptions Unbox(MonoObject* value);

	private:
		ScriptResourceLoadOptions();

	};
#endif
}
