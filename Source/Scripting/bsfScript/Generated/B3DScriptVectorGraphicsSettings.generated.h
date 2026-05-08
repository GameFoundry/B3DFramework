//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/VectorGraphics/B3DVectorGraphics.h"
#include "B3DScriptNonReflectableWrapper.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVectorGraphicsSettings : public TScriptNonReflectableWrapper<VectorGraphicsSettings, ScriptVectorGraphicsSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VectorGraphicsSettings")

		ScriptVectorGraphicsSettings(const TShared<VectorGraphicsSettings>& nativeObject);
		~ScriptVectorGraphicsSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
