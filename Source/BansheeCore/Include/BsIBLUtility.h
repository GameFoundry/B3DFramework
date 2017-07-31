//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace bs { namespace ct
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
		virtual void filterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const = 0;

		/**
		 * Performs filtering on the cubemap, populating the output cubemap with values that can be used for evaluating
		 * irradiance for use in diffuse lighting. Uses order-5 SH (25 coefficients) and outputs the values in the form of
		 * a cubemap.
		 * 
		 * @param[in]		cubemap		Cubemap to filter. Its mip level 0 will be used as source.
		 * @param[in]		output		Output cubemap to store the irradiance data in.
		 */
		virtual void filterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const = 0;

		/**
		 * Performs filtering on the cubemap, populating the output cubemap with values that can be used for evaluating
		 * irradiance for use in diffuse lighting. Uses order-5 SH (9 coefficients) and outputs the values in the form of
		 * a cubemap.
		 * 
		 * @param[in]		cubemap		Cubemap to filter. Its mip level 0 will be used as source.
		 * @param[in]		output		Output buffer in which to place the results. Must be allocated using 
		 *								IrradianceReduceMat<ORDER>::createOutputBuffer();
		 * @param[in]		outputIdx	Index in the output buffer at which to write the output coefficients to.
		 */
		virtual void filterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<GpuBuffer>& output, 
			UINT32 outputIdx) const = 0;

		/**
		 * Scales a cubemap and outputs it in the destination texture, using hardware acceleration. If both textures are the
		 * same size, performs a copy instead.
		 *
		 * @param[in]   src				Source cubemap to scale.
		 * @param[in]   srcMip			Determines which mip level of the source texture to scale.
		 * @param[in]   dst				Desination texture to output the scaled data to. Must be usable as a render target.
		 * @param[in]   dstMip			Determines which mip level of the destination texture to scale.
		 */
		virtual void scaleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, UINT32 dstMip) const = 0;

		static const UINT32 REFLECTION_CUBEMAP_SIZE;
		static const UINT32 IRRADIANCE_CUBEMAP_SIZE;
	};

	/**	Provides easy access to IBLUtility. */
	BS_CORE_EXPORT const IBLUtility& gIBLUtility();

	/** @} */
}}
