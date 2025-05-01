//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptResourceMetaData.generated.h"
#include "../../../Foundation/bsfCore/Material/BsShader.h"

namespace bs { class ShaderMetaData; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderMetaData : public TScriptReflectableWrapper<ShaderMetaData, ScriptShaderMetaData, ScriptResourceMetaDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderMetaData")

		ScriptShaderMetaData(const SPtr<ShaderMetaData>& nativeObject);
		~ScriptShaderMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoArray* InternalGetIncludes(ScriptShaderMetaData* self);
		static void InternalSetIncludes(ScriptShaderMetaData* self, MonoArray* value);
	};
}
