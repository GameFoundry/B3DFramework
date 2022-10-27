//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Image/BsPixelVolume.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPixelVolume : public ScriptObject<ScriptPixelVolume>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PixelVolume")

		static MonoObject* Box(const PixelVolume& value);
		static PixelVolume Unbox(MonoObject* value);

	private:
		ScriptPixelVolume(MonoObject* managedInstance);
	};
} // namespace bs
