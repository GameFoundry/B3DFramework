//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "Material/BsMaterial.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for Material, for adding additional functionality for the script version of the class. */
	class BS_SCRIPT_EXPORT(e:Material) MaterialEx
	{
	public:
		BS_SCRIPT_EXPORT(e:Material,in:true)
		static void setTexture(const HMaterial& thisPtr, const String& name, const HTexture& value, 
			UINT32 mipLevel, UINT32 numMipLevels, UINT32 arraySlice, UINT32 numArraySlices);

		BS_SCRIPT_EXPORT(e:Material,in:true)
		static HTexture getTexture(const HMaterial& thisPtr, const String& name);

		BS_SCRIPT_EXPORT(e:Material,in:true)
		static void setSpriteTexture(const HMaterial& thisPtr, const String& name, const HSpriteTexture& value);

		BS_SCRIPT_EXPORT(e:Material,in:true)
		static HSpriteTexture getSpriteTexture(const HMaterial& thisPtr, const String& name);
	};

	/** @endcond */
	/** @} */
}
