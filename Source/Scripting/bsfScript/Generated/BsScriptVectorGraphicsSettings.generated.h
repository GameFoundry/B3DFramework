//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/VectorGraphics/BsVectorGraphics.h"
#include "BsScriptNonReflectableWrapper.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVectorGraphicsSettings : public TScriptNonReflectableWrapper<VectorGraphicsSettings, ScriptVectorGraphicsSettings>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "VectorGraphicsSettings")

		ScriptVectorGraphicsSettings(const SPtr<VectorGraphicsSettings>& nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
