//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Particles/BsParticleDistribution.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for ColorDistribution, for adding additional functionality for the script interface. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(ColorDistribution)) ColorDistributionEx
	{
	public:
		/** @copydoc ColorDistribution::Evaluate(float, float) */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(ColorDistribution))
		static Color Evaluate(const SPtr<ColorDistribution>& thisPtr, float t, float factor);

		/** @copydoc ColorDistribution::Evaluate(float, const Random&) */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(ColorDistribution))
		static Color Evaluate(const SPtr<ColorDistribution>& thisPtr, float t, Random& factor);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
