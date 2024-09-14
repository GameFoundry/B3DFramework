//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Image/BsPixelVolume.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPixelVolume : public TScriptStructWrapper<ScriptPixelVolume>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PixelVolume")

		static MonoObject* Box(const PixelVolume& value);
		static PixelVolume Unbox(MonoObject* value);

	private:
		ScriptPixelVolume();

	};
}
