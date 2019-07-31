//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsColorGradient.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	*  @{
	*/
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for ColorGradient, for adding additional functionality for the script interface. */
	class BS_SCRIPT_EXPORT(e:ColorGradient) ColorGradientEx
	{
	public:
		/** @copydoc ColorGradient::evaluate */
		BS_SCRIPT_EXPORT(e:ColorGradient)
		static Color evaluate(const SPtr<ColorGradient>& thisPtr, float t);
	};

	
	/** Extension class for ColorGradientHDr, for adding additional functionality for the script interface. */
	class BS_SCRIPT_EXPORT(e:ColorGradientHDR) ColorGradientHDREx
	{
	public:
		/** @copydoc ColorGradientHDR::evaluate */
		BS_SCRIPT_EXPORT(e:ColorGradientHDR)
		static Color evaluate(const SPtr<ColorGradientHDR>& thisPtr, float t);
	};

	/** @endcond */
	/** @} */
}
