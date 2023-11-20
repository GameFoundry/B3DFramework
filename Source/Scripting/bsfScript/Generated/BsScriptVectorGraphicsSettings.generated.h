//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs { struct VectorGraphicsSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVectorGraphicsSettings : public ScriptObject<ScriptVectorGraphicsSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "VectorGraphicsSettings")

		ScriptVectorGraphicsSettings(MonoObject* managedInstance, const SPtr<VectorGraphicsSettings>& value);

		SPtr<VectorGraphicsSettings> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<VectorGraphicsSettings>& value);

	private:
		SPtr<VectorGraphicsSettings> mInternal;

	};
}
