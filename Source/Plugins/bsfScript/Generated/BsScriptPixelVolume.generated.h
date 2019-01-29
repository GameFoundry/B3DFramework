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
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "PixelVolume")

		static MonoObject* box(const PixelVolume& value);
		static PixelVolume unbox(MonoObject* value);

	private:
		ScriptPixelVolume(MonoObject* managedInstance);

	};
}
