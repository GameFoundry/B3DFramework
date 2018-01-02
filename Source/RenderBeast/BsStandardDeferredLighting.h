//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Utility/BsModule.h"
#include "Renderer/BsRendererMaterial.h"
#include "BsLightRendering.h"

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
	class DirectionalLightMat : public RendererMaterial<DirectionalLightMat>
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
		DirectionalLightMat();

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
		static DirectionalLightMat* getVariation(bool msaa, bool singleSampleMSAA = false);
	private:
		GBufferParams mGBufferParams;
		GpuParamTexture mLightOcclusionTexParam;
	};

	/** Shader that renders point (radial & spot) light sources during deferred rendering light pass. */
	class PointLightMat : public RendererMaterial<PointLightMat>
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
		PointLightMat();

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
		static PointLightMat* getVariation(bool inside, bool msaa, bool singleSampleMSAA = false);
	private:
		GBufferParams mGBufferParams;
		GpuParamTexture mLightOcclusionTexParam;
	};

	/** Provides functionality for standard (non-tiled) deferred rendering. */
	class StandardDeferred : public Module<StandardDeferred>
	{
	public:
		StandardDeferred();

		/** Calculates lighting for the specified light, using the standard deferred renderer. */
		void renderLight(LightType type, const RendererLight& light, const RendererView& view, 
			const GBufferTextures& gBufferInput, const SPtr<Texture>& lightOcclusion);

	private:
		SPtr<GpuParamBlockBuffer> mPerLightBuffer;
	};
}}
