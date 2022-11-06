//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "../Extensions/BsShaderEx.h"

namespace bs { class Shader; }
namespace bs { class ShaderEx; }
namespace bs { struct __ShaderVariationParamInfoInterop; }
namespace bs { struct __ShaderParameterInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShader : public TScriptResource<ScriptShader, Shader>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Shader")

		ScriptShader(MonoObject* managedInstance, const ResourceHandle<Shader>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptShader* thisPtr);

		static MonoArray* InternalGetVariationParams(ScriptShader* thisPtr);
		static MonoArray* InternalGetParameters(ScriptShader* thisPtr);
	};
}
