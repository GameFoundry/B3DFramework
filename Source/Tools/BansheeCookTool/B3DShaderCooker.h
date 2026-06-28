//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "FileSystem/B3DPath.h"
#include "Material/B3DShaderCookerSource.h"

namespace b3d
{
	class Package;
	struct PrecompiledVariationData;

	/**
	 * Drives the offline shader cook: compiles each cook item for a single shading language and writes the resulting
	 * shader/variation artifacts into a prebuilt Package that the runtime resolver (ShaderRegistry) looks up. 
	 */
	class ShaderCooker
	{
	public:
		/** Options controlling a single cook run. */
		struct CookOptions
		{
			/** 
			 * Low-level shading language identifier to cook for (for example "vksl"). This determines the cross-compile target language and bytecode format.
			 * e.g. for 'vksl' the input is BSL which gets converted to Vulkan flavored GLSL, which is then converted into SPIR-V bytecode. All the caller
			 * needs to know is that Vulkan backend expects the 'vksl' language id.
			 */
			String Language;

			/** Full path (including filename) of the output package to write. */
			Path OutputPath;
		};

		/**
		 * Cooks all @p items into a single package written to @p options.OutputPath. Individual shader/variation failures
		 * are logged and skipped; the cook continues with the remaining work.
		 *
		 * @return	True if the package was written, false on a fatal error (for example the output could not be saved).
		 */
		static bool Cook(const Vector<ShaderCookItem>& items, const CookOptions& options);

	private:
		/** Compiles and adds the metadata + variation artifacts for a single item to @p package. */
		static bool CookItem(const ShaderCookItem& item, const String& language, Package& package, u32& outVariationCount);

		/**
		 * Drops the high-level cross-compiled program source from each pass of a cooked variation. The variation carries
		 * the language's self-contained baked bytecode, so the source is redundant weight in the shipped artifact.
		 */
		static void StripVariationSource(PrecompiledVariationData& variationData);
	};
} // namespace b3d
