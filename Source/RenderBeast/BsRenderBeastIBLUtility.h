//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "Renderer/BsIBLUtility.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsParamBlocks.h"
#include "BsGpuResourcePool.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(ReflectionCubeDownsampleParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gCubeFace)
		BS_PARAM_BLOCK_ENTRY(int, gMipLevel)
	BS_PARAM_BLOCK_END

	extern ReflectionCubeDownsampleParamDef gReflectionCubeDownsampleParamDef;

	/** Performs filtering on cubemap faces in order to prepare them for importance sampling. */
	class ReflectionCubeDownsampleMat : public RendererMaterial<ReflectionCubeDownsampleMat>
	{
		RMAT_DEF("ReflectionCubeDownsample.bsl")

	public:
		ReflectionCubeDownsampleMat();

		/** Downsamples the provided texture face and outputs it to the provided target. */
		void execute(const SPtr<Texture>& source, UINT32 face, UINT32 mip, const SPtr<RenderTarget>& target);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	BS_PARAM_BLOCK_BEGIN(ReflectionCubeImportanceSampleParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gCubeFace)
		BS_PARAM_BLOCK_ENTRY(int, gMipLevel)
		BS_PARAM_BLOCK_ENTRY(int, gNumMips)
		BS_PARAM_BLOCK_ENTRY(float, gPrecomputedMipFactor)
	BS_PARAM_BLOCK_END

	extern ReflectionCubeImportanceSampleParamDef gReflectionCubeImportanceSampleParamDef;

	/** Performs importance sampling on cubemap faces in order for make them suitable for specular evaluation. */
	class ReflectionCubeImportanceSampleMat : public RendererMaterial<ReflectionCubeImportanceSampleMat>
	{
		RMAT_DEF("ReflectionCubeImportanceSample.bsl")

	public:
		ReflectionCubeImportanceSampleMat();

		/** Importance samples the provided texture face and outputs it to the provided target. */
		void execute(const SPtr<Texture>& source, UINT32 face, UINT32 mip, const SPtr<RenderTarget>& target);

	private:
		static const UINT32 NUM_SAMPLES;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	/** Vector representing spherical harmonic coefficients for 5 bands. */
	struct SHVector5
	{
		SHVector5()
			:coeffs()
		{ }

		float coeffs[25];
	};

	/** Vector representing spherical coefficients for 5 bands, separate for red, green and blue components. */
	struct SHVector5RGB
	{
		SHVector5 R, G, B;
	};

	/** Vector representing spherical harmonic coefficients for 3 bands. */
	struct SHVector3
	{
		float coeffs[9];
	};

	/** Vector representing spherical coefficients for 3 bands, separate for red, green and blue components. */
	struct SHVector3RGB
	{
		SHVector3 R, G, B;
	};

	/** Intermediate structure used for spherical coefficient calculation. Contains RGB coefficients and weight. */
	struct SHCoeffsAndWeight5
	{
		SHVector5RGB coeffs;
		float weight;
	};

	/** Intermediate structure used for spherical coefficient calculation. Contains RGB coefficients and weight. */
	struct SHCoeffsAndWeight3
	{
		SHVector3RGB coeffs;
		float weight;
	};

	BS_PARAM_BLOCK_BEGIN(IrradianceComputeSHParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gCubeFace)
		BS_PARAM_BLOCK_ENTRY(int, gFaceSize)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gDispatchSize)
	BS_PARAM_BLOCK_END

	extern IrradianceComputeSHParamDef gIrradianceComputeSHParamDef;

	/** Computes spherical harmonic coefficients from a radiance cubemap. */
	class IrradianceComputeSHMat : public RendererMaterial<IrradianceComputeSHMat>
	{
		RMAT_DEF("IrradianceComputeSH.bsl")

	public:
		IrradianceComputeSHMat();

		/** 
		 * Computes spherical harmonic coefficients from a radiance texture and outputs a buffer containing a list of
		 * coefficient sets (one set of coefficients for each thread group). Coefficients must be reduced and normalized
		 * by IrradianceReduceSHMat before use. Output buffer should be created by calling createOutputBuffer().
		 */
		void execute(const SPtr<Texture>& source, UINT32 face, const SPtr<GpuBuffer>& output);

		/** Creates a buffer of adequate size to be used as output for this material. */
		SPtr<GpuBuffer> createOutputBuffer(const SPtr<Texture>& source, UINT32& numCoeffSets);

		/** 
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param order		SH order, which defines the number of coefficients and quality. Only values of 3 and 5 are 
		 *					supported.
		 */
		static IrradianceComputeSHMat* getVariation(int order = 5);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
		GpuParamBuffer mOutputBuffer;

		static ShaderVariation VAR_Order3;
		static ShaderVariation VAR_Order5;
	};

	BS_PARAM_BLOCK_BEGIN(IrradianceReduceSHParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gNumEntries)
		BS_PARAM_BLOCK_ENTRY(int, gOutputIdx)
	BS_PARAM_BLOCK_END

	extern IrradianceReduceSHParamDef gIrradianceReduceSHParamDef;

	/** 
	 * Sums spherical harmonic coefficients calculated by each thread group of IrradianceComputeSHMat and outputs a single
	 * set of normalized coefficients. 
	 */
	class IrradianceReduceSHMat : public RendererMaterial<IrradianceReduceSHMat>
	{
		RMAT_DEF("IrradianceReduceSH.bsl")

	public:
		IrradianceReduceSHMat();

		/** 
		 * Sums spherical harmonic coefficients calculated by each thread group of IrradianceComputeSHMat and outputs a
		 * single set of normalized coefficients. Output buffer should be created by calling createOutputBuffer(). The
		 * value will be recorded at the @p outputIdx position in the buffer.
		 */
		void execute(const SPtr<GpuBuffer>& source, UINT32 numCoeffSets, const SPtr<GpuBuffer>& output, UINT32 outputIdx);

		/** Creates a buffer of adequate size to be used as output for this material. */
		SPtr<GpuBuffer> createOutputBuffer(UINT32 numEntries);

		/** 
		 * Returns the material variation matching the provided parameters.
		 *
		 * @param order		SH order, which defines the number of coefficients and quality. Only values of 3 and 5 are 
		 *					supported.
		 */
		static IrradianceReduceSHMat* getVariation(int order = 5);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamBuffer mInputBuffer;
		GpuParamBuffer mOutputBuffer;

		static ShaderVariation VAR_Order3;
		static ShaderVariation VAR_Order5;
	};

	BS_PARAM_BLOCK_BEGIN(IrradianceComputeSHFragParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gCubeFace)
		BS_PARAM_BLOCK_ENTRY(int, gFaceSize)
		BS_PARAM_BLOCK_ENTRY(int, gCoeffIdx)
	BS_PARAM_BLOCK_END

	extern IrradianceComputeSHFragParamDef gIrradianceComputeSHFragParamDef;

	/** 
	 * Computes spherical harmonic coefficients from a radiance cubemap. This is an alternative to IrradianceComputeSHMat
	 * that does not require compute shader support. 
	 */
	class IrradianceComputeSHFragMat : public RendererMaterial<IrradianceComputeSHFragMat>
	{
		RMAT_DEF("IrradianceComputeSHFrag.bsl")

	public:
		IrradianceComputeSHFragMat();

		/** 
		 * Computes spherical harmonic coefficients from a face of an input cube radiance texture and outputs them to the
		 * specified face of the output cube texture. Only a single coefficient is output per execution. The output texture
		 * will contain the coefficients for red, green and blue channels in the corresponding texture channels, and 
		 * per-texel weight in the alpha channel. Output coefficients must be summed up and normalized before use (using 
		 * IrradianceAccumulateCubeSH).
		 */
		void execute(const SPtr<Texture>& source, UINT32 face, UINT32 coefficientIdx, const SPtr<RenderTarget>& output);

		/** 
		 * Returns the texture descriptor that can be used for initializing the output render target. Note that the
		 * output texture is a cubemap but the execute() method expects a render target that is a single face of a
		 * cubemap. 
		 */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc(const SPtr<Texture>& source);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	BS_PARAM_BLOCK_BEGIN(IrradianceAccumulateSHParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gCubeFace)
		BS_PARAM_BLOCK_ENTRY(int, gCubeMip)
		BS_PARAM_BLOCK_ENTRY(Vector2, gHalfPixel)
	BS_PARAM_BLOCK_END

	extern IrradianceAccumulateSHParamDef gIrradianceAccumulateSHParamDef;

	/** 
	 * Downsamples a cubemap face containing SH coefficient and weight values as output by IrradianceComputeSHFragMat. Each
	 * downsample sums up 2x2 pixel area coefficients/weights from the previous mip level.
	 */
	class IrradianceAccumulateSHMat : public RendererMaterial<IrradianceAccumulateSHMat>
	{
		RMAT_DEF("IrradianceAccumulateSH.bsl")

	public:
		IrradianceAccumulateSHMat();

		/** 
		 * Downsamples the provided face and mip level of the source texture and outputs the downsampled (i.e summed up)
		 * values in the resulting output texture. 
		 */
		void execute(const SPtr<Texture>& source, UINT32 face, UINT32 sourceMip, const SPtr<RenderTarget>& output);

		/** 
		 * Returns the texture descriptor that can be used for initializing the output render target. Note the output
		 * is a cubemap.
		 */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc(const SPtr<Texture>& source);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	/** 
	 * Accumulates SH coefficient values from all six faces of a cubemap and normalizes them. The cubemap is expected to be
	 * 1x1 in size (previously downsampled by IrradianceAccumulateSHMat). After this shader is ran for all SH coefficients
	 * the output texture will contain final valid set of SH coefficients.
	 */
	class IrradianceAccumulateCubeSHMat : public RendererMaterial<IrradianceAccumulateCubeSHMat>
	{
		RMAT_DEF("IrradianceAccumulateCubeSH.bsl")

	public:
		IrradianceAccumulateCubeSHMat();

		/** 
		 * Sums up all faces of the input cube texture and writes the value to the corresponding index in the output
		 * texture. The source mip should point to a mip level with size 1x1.
		 */
		void execute(const SPtr<Texture>& source, UINT32 sourceMip, UINT32 coefficientIdx, 
			const SPtr<RenderTarget>& output);

		/** 
		 * Returns the texture descriptor that can be used for initializing the output render target. The render target
		 * will be able to hold all required SH coefficients (even though execute() outputs just one coefficient at a time).
		 */
		static POOLED_RENDER_TEXTURE_DESC getOutputDesc();

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamTexture mInputTexture;
	};

	BS_PARAM_BLOCK_BEGIN(IrradianceProjectSHParamDef)
		BS_PARAM_BLOCK_ENTRY(int, gCubeFace)
	BS_PARAM_BLOCK_END

	extern IrradianceProjectSHParamDef gIrradianceProjectSHParamDef;

	/** 
	 * Projects spherical harmonic coefficients calculated by IrradianceReduceSHMat and projects them onto faces of
	 * a cubemap.
	 */
	class IrradianceProjectSHMat : public RendererMaterial<IrradianceProjectSHMat>
	{
		RMAT_DEF("IrradianceProjectSH.bsl")

	public:
		IrradianceProjectSHMat();

		/** 
		 * Projects spherical harmonic coefficients calculated by IrradianceReduceSHMat and projects them onto faces of
		 * a cubemap.
		 */
		void execute(const SPtr<GpuBuffer>& shCoeffs, UINT32 face, const SPtr<RenderTarget>& target);

	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GpuParamBuffer mInputBuffer;
	};

	/** Render beast implementation of IBLUtility. */
	class RenderBeastIBLUtility : public IBLUtility
	{
	public:
		/** @copydoc IBLUtility::filterCubemapForSpecular */
		void filterCubemapForSpecular(const SPtr<Texture>& cubemap, const SPtr<Texture>& scratch) const override;

		/** @copydoc IBLUtility::filterCubemapForIrradiance(const SPtr<Texture>&, const SPtr<Texture>&) */
		void filterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<Texture>& output) const override;

		/** @copydoc IBLUtility::filterCubemapForIrradiance(const SPtr<Texture>&, const SPtr<GpuBuffer>&, UINT32) */
		void filterCubemapForIrradiance(const SPtr<Texture>& cubemap, const SPtr<GpuBuffer>& output, 
			UINT32 outputIdx) const override;

		/** @copydoc IBLUtility::scaleCubemap */
		void scaleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, UINT32 dstMip) const override;

	private:
		/** 
		 * Downsamples a cubemap using hardware bilinear filtering. 
		 * 
		 * @param[in]	src		Cubemap to downsample.
		 * @param[in]   srcMip	Determines which mip level of the source texture to downsample.
		 * @param[in]   dst		Desination texture to output the scaled data to. Must be usable as a render target.
		 * @param[in]   dstMip	Determines which mip level of the destination texture to scale.
		 */
		static void downsampleCubemap(const SPtr<Texture>& src, UINT32 srcMip, const SPtr<Texture>& dst, UINT32 dstMip);

		/** 
		 * Generates irradiance SH coefficients from the input cubemap and writes them to a 1D texture. Does not make
		 * use of the compute shader.
		 */
		static SPtr<Texture> filterCubemapForIrradianceNonCompute(const SPtr<Texture>& cubemap);
	};

	/** @} */
}}
