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
	template<bool MSAA>
	class DirectionalLightMat : public RendererMaterial<DirectionalLightMat<MSAA>>
	{
		RMAT_DEF("DeferredDirectionalLight.bsl");

	public:
		DirectionalLightMat();

		/** Binds the material for rendering and sets up any global parameters. */
		void bind(const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion, 
			const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Updates the per-light buffers used by the material. */
		void setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight);
	private:
		GBufferParams mGBufferParams;
		GpuParamTexture mLightOcclusionTexParam;
	};

	/** Shader that renders point (radial & spot) light sources during deferred rendering light pass. */
	template<bool MSAA, bool InsideGeometry>
	class PointLightMat : public RendererMaterial<PointLightMat<MSAA, InsideGeometry>>
	{
		RMAT_DEF("DeferredPointLight.bsl");

	public:
		PointLightMat();

		/** Binds the material for rendering and sets up any global parameters. */
		void bind(const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion, 
			const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Updates the per-light buffers used by the material. */
		void setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight);
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
			const GBufferInput& gBufferInput, const SPtr<Texture>& lightOcclusion);

	private:
		SPtr<GpuParamBlockBuffer> mPerLightBuffer;

		PointLightMat<true, true> mPointLightMat_TT;
		PointLightMat<true, false> mPointLightMat_TF;
		PointLightMat<false, true> mPointLightMat_FT;
		PointLightMat<false, false> mPointLightMat_FF;
		DirectionalLightMat<true> mDirLightMat_T;
		DirectionalLightMat<false> mDirLightMat_F;
	};
}}
