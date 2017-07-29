//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsModule.h"
#include "BsRendererMaterial.h"
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

	public:
		DirectionalLightMat();

		/** Binds the material for rendering and sets up any global parameters. */
		void bind(const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion, 
			const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Updates the per-light buffers used by the material. */
		void setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight);
		
		/** Returns the material variation matching the provided parameters. */
		static DirectionalLightMat* getVariation(bool msaa);
	private:
		GBufferParams mGBufferParams;
		GpuParamTexture mLightOcclusionTexParam;

		static ShaderVariation VAR_MSAA;
		static ShaderVariation VAR_NoMSAA;
	};

	/** Shader that renders point (radial & spot) light sources during deferred rendering light pass. */
	class PointLightMat : public RendererMaterial<PointLightMat>
	{
		RMAT_DEF("DeferredPointLight.bsl");

	public:
		PointLightMat();

		/** Binds the material for rendering and sets up any global parameters. */
		void bind(const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion, 
			const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Updates the per-light buffers used by the material. */
		void setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight);

		/** Returns the material variation matching the provided parameters. */
		static PointLightMat* getVariation(bool msaa, bool inside);
	private:
		GBufferParams mGBufferParams;
		GpuParamTexture mLightOcclusionTexParam;

		static ShaderVariation VAR_MSAA_Inside;
		static ShaderVariation VAR_MSAA_Outside;
		static ShaderVariation VAR_NoMSAA_Inside;
		static ShaderVariation VAR_NoMSAA_Outside;
	};

	/** Provides functionality for standard (non-tiled) deferred rendering. */
	class StandardDeferred : public Module<StandardDeferred>
	{
	public:
		StandardDeferred();

		/** Calculates lighting for the specified light, using the standard deferred renderer. */
		void renderLight(LightType type, const RendererLight& light, const RendererView& view, 
			const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion);

	private:
		SPtr<GpuParamBlockBuffer> mPerLightBuffer;
	};
}}
