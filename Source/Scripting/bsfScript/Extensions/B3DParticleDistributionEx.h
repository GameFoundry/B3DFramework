//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Particles/B3DParticleDistribution.h"

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
		static Color Evaluate(const TShared<ColorDistribution>& thisPtr, float t, float factor);

		/** @copydoc ColorDistribution::Evaluate(float, const Random&) */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(ColorDistribution))
		static Color Evaluate(const TShared<ColorDistribution>& thisPtr, float t, Random& factor);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
