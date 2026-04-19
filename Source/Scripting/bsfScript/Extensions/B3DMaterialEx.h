//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Material/B3DMaterial.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for Material, for adding additional functionality for the script version of the class. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(Material)) MaterialEx
	{
	public:
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Material), InteropOnly(true))
		static void SetTexture(const HMaterial& thisPtr, const String& name, const HTexture& value, u32 mipLevel, u32 numMipLevels, u32 arraySlice, u32 numArraySlices);

		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Material), InteropOnly(true))
		static HTexture GetTexture(const HMaterial& thisPtr, const String& name);

		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Material), InteropOnly(true))
		static void SetSpriteImage(const HMaterial& thisPtr, const String& name, const HSpriteImage& value);

		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Material), InteropOnly(true))
		static HSpriteImage GetSpriteImage(const HMaterial& thisPtr, const String& name);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
