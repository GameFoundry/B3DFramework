//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Renderer
		 *  @{
		 */

		/** Helper class that handles generation and processing of textures used for image based lighting. */
		class B3D_EXPORT IBLUtility : public Module<IBLUtility>
		{
		public:
			/**
			 * Performs filtering on the cubemap, populating its mip-maps with filtered values that can be used for
			 * evaluating specular reflections.
			 *
			 * @param	commandBuffer		Command buffer to execute on.
			 * @param	cubemap				Cubemap to filter. Its mip level 0 will be read, filtered and written into
			 *								other mip levels.
			 * @param	scratch				Temporary cubemap texture to use for the filtering process. Must match the size of
			 *								the source cubemap. Provide null to automatically create a scratch cubemap.
			 */
			virtual void FilterCubemapForSpecular(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& scratch) const = 0;

			/**
			 * Performs filtering on the cubemap, populating the output cubemap with values that can be used for evaluating
			 * irradiance for use in diffuse lighting. Uses order-5 SH (25 coefficients) and outputs the values in the form of
			 * a cubemap.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param	cubemap			Cubemap to filter. Its mip level 0 will be used as source.
			 * @param	output			Output cubemap to store the irradiance data in.
			 */
			virtual void FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& output) const = 0;

			/**
			 * Performs filtering on the cubemap, populating the output texture with values that can be used for evaluating
			 * irradiance for use in diffuse lighting. Uses order-3 SH (9 coefficients) and outputs the values in the form of
			 * SH coefficients.
			 *
			 * @param	commandBuffer		Command buffer to execute on.
			 * @param	cubemap				Cubemap to filter. Its mip level 0 will be used as source.
			 * @param	output				Output texture in which to place the results. Must be allocated using
			 *								IrradianceReduceMat::createOutputTexture();
			 * @param	outputIdx			Index in the output buffer at which to write the output coefficients to.
			 */
			virtual void FilterCubemapForIrradiance(GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const TShared<Texture>& output, u32 outputIdx) const = 0;

			/**
			 * Scales a cubemap and outputs it in the destination texture, using hardware acceleration. If both textures are the
			 * same size, performs a copy instead.
			 *
			 * @param	commandBuffer	Command buffer to execute on.
			 * @param   src				Source cubemap to scale.
			 * @param   srcMip			Determines which mip level of the source texture to scale.
			 * @param   dst				Desination texture to output the scaled data to. Must be usable as a render target.
			 * @param   dstMip			Determines which mip level of the destination texture to scale.
			 */
			virtual void ScaleCubemap(GpuCommandBuffer& commandBuffer, const TShared<Texture>& src, u32 srcMip, const TShared<Texture>& dst, u32 dstMip) const = 0;

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
		B3D_EXPORT const IBLUtility& GetIBLUtility();

		/** @} */
	} // namespace render
} // namespace b3d
