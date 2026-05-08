//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptResourceMetaData.generated.h"
#include "../../../Engine/Core/Material/B3DShader.h"

namespace b3d { class ShaderMetaData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptShaderMetaData : public TScriptReflectableWrapper<ShaderMetaData, ScriptShaderMetaData, ScriptResourceMetaDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ShaderMetaData")

		ScriptShaderMetaData(const TShared<ShaderMetaData>& nativeObject);
		~ScriptShaderMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoArray* InternalGetIncludes(ScriptShaderMetaData* self);
		static void InternalSetIncludes(ScriptShaderMetaData* self, MonoArray* value);
	};
}
