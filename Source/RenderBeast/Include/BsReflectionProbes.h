//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(ReflectionCubemapFilterParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gCubeFace)
	BS_PARAM_BLOCK_END

	extern ReflectionCubemapFilterParamDef gReflectionCubemapFilterDef;

	/** Performs filtering on cubemap faces in order for make them suitable for specular evaluation. */
	class ReflectionCubemapFilterMat : public RendererMaterial<ReflectionCubemapFilterMat>
	{
		RMAT_DEF("ReflectionCubemapFilter.bsl")

	public:
		ReflectionCubemapFilterMat();

		/** Downsamples the provided texture face and outputs it to the provided target. */
		void execute(const SPtr<Texture>& source, UINT32 face, const TextureSurface& surface, 
					 const SPtr<RenderTarget>& target);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	/** Helper class that handles generation and processing of textures used for reflection probes. */
	class ReflectionProbes
	{
	public:
		/**
		 * Performs filtering on the cubemap, populating its mip-maps with filtered values that can be used for
		 * evaluating specular reflections.
		 */
		static void filterCubemapForSpecular(const SPtr<Texture>& cubemap);

		static const UINT32 REFLECTION_CUBEMAP_SIZE;
	};

	/** @} */
}}
