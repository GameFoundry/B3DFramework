//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Image/B3DColorGradient.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for ColorGradient, for adding additional functionality for the script interface. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(ColorGradient)) ColorGradientEx
	{
	public:
		/** @copydoc ColorGradient::Evaluate */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(ColorGradient))
		static Color Evaluate(const TShared<ColorGradient>& thisPtr, float t);
	};

	/** Extension class for ColorGradientHDr, for adding additional functionality for the script interface. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(ColorGradientHDR)) ColorGradientHDREx
	{
	public:
		/** @copydoc ColorGradientHDR::Evaluate */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(ColorGradientHDR))
		static Color Evaluate(const TShared<ColorGradientHDR>& thisPtr, float t);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
