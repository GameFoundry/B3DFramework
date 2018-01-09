//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Utility/BsModule.h"
#include "Renderer/BsRendererMaterial.h"
#include "BsLightRendering.h"
#include "BsImageBasedLighting.h"

namespace bs { namespace ct {
	class RendererLight;

	BS_PARAM_BLOCK_BEGIN(PerLightParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector4, gLightPositionAndSrcRadius)
		BS_PARAM_BLOCK_ENTRY(Vector4, gLightColorAndLuminance)
		BS_PARAM_BLOCK_ENTRY(Vector4, gLightSpotAnglesAndSqrdInvAttRadius)
		BS_PARAM_BLOCK_ENTRY(Vector4, gLightDirectionAndAttRadius)
		BS_PARAM_BLOCK_ENTRY(Vector4, gShiftedLightPositionAndType)
		BS_PARAM_BLOCK_ENTRY(Vector4, gLightGeometry)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatConeTransform)
	BS_PARAM_BLOCK_END

	extern PerLightParamDef gPerLightParamDef;

	/** Shader that renders directional light sources during deferred rendering light pass. */
	class DeferredDirectionalLightMat : public RendererMaterial<DeferredDirectionalLightMat>
	{
		RMAT_DEF("DeferredDirectionalLight.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool msaa, bool singleSampleMSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation({
				ShaderVariation::Param("MSAA", msaa),
				ShaderVariation::Param("MSAA_RESOLVE_0TH", singleSampleMSAA)
			});

			return variation;
		}
	public:
		DeferredDirectionalLightMat();

		/** Binds the material for rendering and sets up any parameters. */
		void bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, 
			const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight);

		/** 
		 * Returns the material variation matching the provided parameters. 
		 * 
		 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
		 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
		 *									evaluated. Otherwise all samples will be evaluated.
		 * @return							Requested variation of the material.
		 */
		static DeferredDirectionalLightMat* getVariation(bool msaa, bool singleSampleMSAA = false);
	private:
		GBufferParams mGBufferParams;
		GpuParamTexture mLightOcclusionTexParam;
	};

	/** Shader that renders point (radial & spot) light sources during deferred rendering light pass. */
	class DeferredPointLightMat : public RendererMaterial<DeferredPointLightMat>
	{
		RMAT_DEF("DeferredPointLight.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool inside, bool msaa, bool singleSampleMSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation({
				ShaderVariation::Param("MSAA", msaa),
				ShaderVariation::Param("INSIDE_GEOMETRY", inside),
				ShaderVariation::Param("MSAA_RESOLVE_0TH", singleSampleMSAA)
			});

			return variation;
		}
	public:
		DeferredPointLightMat();

		/** Binds the material for rendering and sets up any parameters. */
		void bind(const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion, 
			const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<GpuParamBlockBuffer>& perLight);

		/** 
		 * Returns the material variation matching the provided parameters. 
		 * 
		 * @param[in]	inside				Set to true if viewer is inside the light's stencil geometry.
		 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
		 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
		 *									evaluated. Otherwise all samples will be evaluated.
		 * @return							Requested variation of the material.
		 */
		static DeferredPointLightMat* getVariation(bool inside, bool msaa, bool singleSampleMSAA = false);
	private:
		GBufferParams mGBufferParams;
		GpuParamTexture mLightOcclusionTexParam;
	};

	BS_PARAM_BLOCK_BEGIN(PerProbeParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector3, gPosition)
		BS_PARAM_BLOCK_ENTRY(Vector3, gExtents)
		BS_PARAM_BLOCK_ENTRY(float, gTransitionDistance)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gInvBoxTransform)
		BS_PARAM_BLOCK_ENTRY(INT32, gCubemapIdx)
		BS_PARAM_BLOCK_ENTRY(INT32, gType)
	BS_PARAM_BLOCK_END

	extern PerProbeParamDef gPerProbeParamDef;

	/** 
	 * Shader that prepares the surface for image based lighting. 
	 * 
	 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable. 
	 * Needs to be followed by execution of all other DeferredIBL* materials.
	 */
	class DeferredIBLSetupMat : public RendererMaterial<DeferredIBLSetupMat>
	{
		RMAT_DEF("DeferredIBLSetup.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool msaa, bool singleSampleMSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation({
				ShaderVariation::Param("MSAA", msaa),
				ShaderVariation::Param("MSAA_RESOLVE_0TH", singleSampleMSAA)
			});

			return variation;
		}
	public:
		DeferredIBLSetupMat();

		/** Binds the material for rendering and sets up any parameters. */
		void bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
			const SPtr<Texture>& ssr, const SPtr<Texture>& ao, const SPtr<GpuParamBlockBuffer>& reflProbeParams);

		/** 
		 * Returns the material variation matching the provided parameters. 
		 * 
		 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
		 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
		 *									evaluated. Otherwise all samples will be evaluated.
		 * @return							Requested variation of the material.
		 */
		static DeferredIBLSetupMat* getVariation(bool msaa, bool singleSampleMSAA = false);
	private:
		GBufferParams mGBufferParams;
		ImageBasedLightingParams mIBLParams;
	};

	/** 
	 * Shader that renders an individual reflection probe for image based lighting. 
	 * 
	 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable. 
	 * Must be preceeded by DeferredIBLSetupMat and followed by DeferredIBLSkyMat and DeferredIBLFinalizeMat. 
	 */
	class DeferredIBLProbeMat : public RendererMaterial<DeferredIBLProbeMat>
	{
		RMAT_DEF("DeferredIBLProbe.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool inside, bool msaa, bool singleSampleMSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation({
				ShaderVariation::Param("MSAA", msaa),
				ShaderVariation::Param("INSIDE_GEOMETRY", inside),
				ShaderVariation::Param("MSAA_RESOLVE_0TH", singleSampleMSAA)
			});

			return variation;
		}
	public:
		DeferredIBLProbeMat();

		/** Binds the material for rendering and sets up any parameters. */
		void bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera,
			const SceneInfo& sceneInfo, const ReflProbeData& probeData, const SPtr<GpuParamBlockBuffer>& reflProbeParams);

		/** 
		 * Returns the material variation matching the provided parameters. 
		 * 
		 * @param[in]	inside				Set to true if viewer is inside the probe's stencil geometry.
		 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
		 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
		 *									evaluated. Otherwise all samples will be evaluated.
		 * @return							Requested variation of the material.
		 */
		static DeferredIBLProbeMat* getVariation(bool inside, bool msaa, bool singleSampleMSAA = false);
	private:
		SPtr<GpuParamBlockBuffer> mParamBuffer;
		GBufferParams mGBufferParams;
		ImageBasedLightingParams mIBLParams;
	};

	/** 
	 * Shader that renders the sky reflections. The results are additively blended with the currently bound render target.
	 * 
	 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
	 * Must be preceeded by DeferredIBLSetupMat and followed by DeferredIBLFinalizeMat. 
	 */
	class DeferredIBLSkyMat : public RendererMaterial<DeferredIBLSkyMat>
	{
		RMAT_DEF("DeferredIBLSky.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool msaa, bool singleSampleMSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation({
				ShaderVariation::Param("MSAA", msaa),
				ShaderVariation::Param("MSAA_RESOLVE_0TH", singleSampleMSAA)
			});

			return variation;
		}
	public:
		DeferredIBLSkyMat();

		/** Binds the material for rendering and sets up any parameters. */
		void bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, 
			const Skybox* skybox, const SPtr<GpuParamBlockBuffer>& reflProbeParams);

		/** 
		 * Returns the material variation matching the provided parameters. 
		 * 
		 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
		 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
		 *									evaluated. Otherwise all samples will be evaluated.
		 * @return							Requested variation of the material.
		 */
		static DeferredIBLSkyMat* getVariation(bool msaa, bool singleSampleMSAA = false);
	private:
		GBufferParams mGBufferParams;
		ImageBasedLightingParams mIBLParams;
	};

	/** 
	 * Material that finalizes the rendering of reflections. As input it takes the texture output by previous DeferredIBL*
	 * materials, and the resulting output is blended additively with the current render target. 
	 * 
	 * This is an alternative to TiledDeferredImageBasedLighting for cases when compute shaders are not usable or suitable.
	 */
	class DeferredIBLFinalizeMat : public RendererMaterial<DeferredIBLFinalizeMat>
	{
		RMAT_DEF("DeferredIBLFinalize.bsl");

		/** Helper method used for initializing variations of this material. */
		template<bool msaa, bool singleSampleMSAA>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation({
				ShaderVariation::Param("MSAA", msaa),
				ShaderVariation::Param("MSAA_RESOLVE_0TH", singleSampleMSAA)
			});

			return variation;
		}
	public:
		DeferredIBLFinalizeMat();

		/** Binds the material for rendering and sets up any parameters. */
		void bind(const GBufferTextures& gBufferInput, const SPtr<GpuParamBlockBuffer>& perCamera, 
			const SPtr<Texture>& iblRadiance, const SPtr<Texture>& preintegratedBrdf, 
			const SPtr<GpuParamBlockBuffer>& reflProbeParams);

		/** 
		 * Returns the material variation matching the provided parameters. 
		 * 
		 * @param[in]	msaa				True if the shader will operate on a multisampled surface.
		 * @param[in]	singleSampleMSAA	Only relevant of @p msaa is true. When enabled only the first sample will be
		 *									evaluated. Otherwise all samples will be evaluated.
		 * @return							Requested variation of the material.
		 */
		static DeferredIBLFinalizeMat* getVariation(bool msaa, bool singleSampleMSAA = false);
	private:
		GBufferParams mGBufferParams;
		ImageBasedLightingParams mIBLParams;
		GpuParamTexture mIBLRadiance;
	};

	/** Provides functionality for standard (non-tiled) deferred rendering. */
	class StandardDeferred : public Module<StandardDeferred>
	{
	public:
		StandardDeferred();

		/** Calculates lighting for the specified light, using the standard deferred renderer. */
		void renderLight(LightType type, const RendererLight& light, const RendererView& view, 
			const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion);

		/** 
		 * Evaluates filtered radiance from a single reflection probe and blends it into the current render target. 
		 * Alpha value of the render target is used for determining the contribution and will be updated with new
		 * contibution after blending.
		 */
		void renderReflProbe(const ReflProbeData& probeData, const RendererView& view, 
			const GBufferTextures& gBufferInput, const SceneInfo& sceneInfo, 
			const SPtr<GpuParamBlockBuffer>& reflProbeParams);

	private:
		SPtr<GpuParamBlockBuffer> mPerLightBuffer;
	};
}}
