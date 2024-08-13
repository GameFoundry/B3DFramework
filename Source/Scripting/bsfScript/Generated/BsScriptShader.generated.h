//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"
#include "../Extensions/BsShaderEx.h"

namespace bs { class Shader; }
namespace bs { struct __ShaderVariationParameterInformationInterop; }
namespace bs { class ShaderEx; }
namespace bs { struct __ShaderParameterInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShader : public TScriptResource<ScriptShader, Shader>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Shader")

		ScriptShader(MonoObject* managedInstance, const TResourceHandle<Shader>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptShader* self);

		static MonoArray* InternalGetVariationParams(ScriptShader* self);
		static MonoArray* InternalGetParameters(ScriptShader* self);
	};
}
