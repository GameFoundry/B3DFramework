//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../Extensions/BsShaderEx.h"

namespace bs
{
	class Shader;
	class ShaderEx;
	struct __ShaderParameterInterop;

	class BS_SCR_BE_EXPORT ScriptShader : public TScriptResource<ScriptShader, Shader>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Shader")

		ScriptShader(MonoObject* managedInstance, const ResourceHandle<Shader>& value);

		static MonoObject* createInstance();

	private:
		static MonoObject* Internal_getRef(ScriptShader* thisPtr);

		static MonoArray* Internal_getParameters(ScriptShader* thisPtr);
	};
}
