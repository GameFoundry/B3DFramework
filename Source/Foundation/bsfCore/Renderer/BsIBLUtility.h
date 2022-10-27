//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Renderer-Internal
		 *  @{
		 */

		/** Helper class that handles generation and processing of textures used for image based lighting. */
		class BS_CORE_EXPORT IBLUtility : public Module<IBLUtility>
		{
		public:
			/**
			 * Performs filtering on the cubemap, populating its mip-maps with filtered values that can be used for
			 * evaluating specular reflections.
			 *
			 * @param[in, out]	cubemap		Cubemap to filter. Its mip level 0 will be read, filtered and written into
			 *								other mip levels.
			 * @param[in]		scratch		Temporary cubemap texture to use for the filtering process. Must match the size of
			 *								the source cubemap. Provide null to automatically create a scratch cubemap.
			 */
			virtual void FilterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const = 0;

			/**
			 * Performs filtering on the cubemap, populating the output cubemap with values that can be used for evaluating
			 * irradiance for use in diffuse lighting. Uses order-5 SH (25 coefficients) and outputs the values in the form of
			 * a cubemap.
			 *
			 * @param[in]		cubemap		Cubemap to filter. Its mip level 0 will be used as source.
			 * @param[in]		output		Output cubemap to store the irradiance data in.
			 */
			virtual void FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const = 0;

			/**
			 * Performs filtering on the cubemap, populating the output texture with values that can be used for evaluating
			 * irradiance for use in diffuse lighting. Uses order-3 SH (9 coefficients) and outputs the values in the form of
			 * SH coefficients.
			 *
			 * @param[in]		cubemap		Cubemap to filter. Its mip level 0 will be used as source.
			 * @param[in]		output		Output texture in which to place the results. Must be allocated using
			 *								IrradianceReduceMat::createOutputTexture();
			 * @param[in]		outputIdx	Index in the output buffer at which to write the output coefficients to.
			 */
			virtual void FilterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output, u32 outputIdx) const = 0;

			/**
			 * Scales a cubemap and outputs it in the destination texture, using hardware acceleration. If both textures are the
			 * same size, performs a copy instead.
			 *
			 * @param[in]   src				Source cubemap to scale.
			 * @param[in]   srcMip			Determines which mip level of the source texture to scale.
			 * @param[in]   dst				Desination texture to output the scaled data to. Must be usable as a render target.
			 * @param[in]   dstMip			Determines which mip level of the destination texture to scale.
			 */
			virtual void ScaleCubemap(const SPtr<Texture>& src, u32 srcMip, const SPtr<Texture>& dst, u32 dstMip) const = 0;

			/** Returns the size of the texture required to store the provided number of SH coefficient sets. */
			static Vector2I GetShCoeffTextureSize(u32 numCoeffSets, u32 shOrder);

			/**
			 * Determines the position of a set of coefficients in the coefficient texture, depending on the coefficient index.
			 */
			static Vector2I GetShCoeffXyFromIdx(u32 idx, u32 shOrder);

			static const u32 kReflectionCubemapSize;
			static const u32 kIrradianceCubemapSize;
		};

		/**	Provides easy access to IBLUtility. */
		BS_CORE_EXPORT const IBLUtility& gIBLUtility();

		/** @} */
	} // namespace ct
} // namespace bs
