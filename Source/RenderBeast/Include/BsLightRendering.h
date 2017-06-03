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

	/** Information about a single light, as seen by the lighting shader. */
	struct LightData
	{
		Vector3 position;
		float attRadius;
		Vector3 direction;
		float luminance;
		Vector3 spotAngles;
		float attRadiusSqrdInv;
		Vector3 color;
		float srcRadius;
		Vector3 shiftedLightPosition;
		float padding;
	};

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

	/**	Renderer information specific to a single light. */
	class RendererLight
	{
	public:
		RendererLight(Light* light);

		/** Populates the structure with light parameters. */
		void getParameters(LightData& output) const;

		/** 
		 * Populates the provided parameter block buffer with information about the light. Provided buffer's structure
		 * must match PerLightParamDef.
		 */
		void getParameters(SPtr<GpuParamBlockBuffer>& buffer) const;
		
		Light* internal;
	};

	/** Allows you to easily bind GBuffer textures to some material. */
	class GBufferParams
	{
	public:
		GBufferParams(const SPtr<Material>& material, const SPtr<GpuParamsSet>& paramsSet);

		/** Binds the GBuffer textures to the pipeline. */
		void bind(const SPtr<RenderTargets>& renderTargets);

	private:
		MaterialParamTexture mGBufferA;
		MaterialParamTexture mGBufferB;
		MaterialParamTexture mGBufferC;
		MaterialParamTexture mGBufferDepth;
	};

	/** Shader that renders directional light sources during deferred rendering light pass. */
	template<bool MSAA>
	class DirectionalLightMat : public RendererMaterial<DirectionalLightMat<MSAA>>
	{
		RMAT_DEF("DeferredDirectionalLight.bsl");

	public:
		DirectionalLightMat();

		/** Binds the material for rendering and sets up any global parameters. */
		void bind(const SPtr<RenderTargets>& gbuffer, const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Updates the per-light buffers used by the material. */
		void setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight);
	private:
		GBufferParams mGBufferParams;
	};

	/** Shader that renders point (radial & spot) light sources during deferred rendering light pass. */
	template<bool MSAA, bool InsideGeometry>
	class PointLightMat : public RendererMaterial<PointLightMat<MSAA, InsideGeometry>>
	{
		RMAT_DEF("DeferredPointLight.bsl");

	public:
		PointLightMat();

		/** Binds the material for rendering and sets up any global parameters. */
		void bind(const SPtr<RenderTargets>& gbuffer, const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Updates the per-light buffers used by the material. */
		void setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight);
	private:
		GBufferParams mGBufferParams;
	};

	/** Contains GPU buffers used by the renderer to manipulate lights. */
	class GPULightData
	{
	public:
		GPULightData();

		/** Updates the internal buffers with a new set of lights. */
		void setLights(const Vector<LightData>& lightData, UINT32 numDirLights, UINT32 numRadialLights,
					   UINT32 numSpotLights);

		/** Returns a GPU bindable buffer containing information about every light. */
		SPtr<GpuBuffer> getLightBuffer() const { return mLightBuffer; }

		/** Returns the number of directional lights in the lights buffer. */
		UINT32 getNumDirLights() const { return mNumLights[0]; }

		/** Returns the number of radial point lights in the lights buffer. */
		UINT32 getNumRadialLights() const { return mNumLights[1]; }

		/** Returns the number of spot point lights in the lights buffer. */
		UINT32 getNumSpotLights() const { return mNumLights[2]; }

	private:
		SPtr<GpuBuffer> mLightBuffer;

		UINT32 mNumLights[3];
	};

	BS_PARAM_BLOCK_BEGIN(TiledLightingParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector3I, gLightOffsets)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gFramebufferSize)
	BS_PARAM_BLOCK_END

	extern TiledLightingParamDef gTiledLightingParamDef;

	/** Functionality common to all versions of TiledDeferredLightingMat<T>. */
	class TiledDeferredLighting
	{
	public:
		TiledDeferredLighting(const SPtr<Material>& material, const SPtr<GpuParamsSet>& paramsSet, UINT32 sampleCount);

		/** Binds the material for rendering, sets up parameters and executes it. */
		void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera, bool noLighting);

		/** Binds all the active lights. */
		void setLights(const GPULightData& lightData);

		static const UINT32 TILE_SIZE;
	private:
		UINT32 mSampleCount;
		SPtr<Material> mMaterial;
		SPtr<GpuParamsSet> mParamsSet;

		GBufferParams mGBufferParams;

		Vector3I mLightOffsets;
		GpuParamBuffer mLightBufferParam;
		GpuParamLoadStoreTexture mOutputTextureParam;
		GpuParamBuffer mOutputBufferParam;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
	};

	/** Interface implemented by all versions of TTiledDeferredLightingMat<T>. */
	class ITiledDeferredLightingMat
	{
	public:
		virtual ~ITiledDeferredLightingMat() {}

		/** @copydoc TiledDeferredLighting::execute() */
		virtual void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
			bool noLighting) = 0;

		/** @copydoc TiledDeferredLighting::setLights() */
		virtual void setLights(const GPULightData& lightData) = 0;
	};

	/** Shader that performs a lighting pass over data stored in the Gbuffer. */
	template<int MSAA_COUNT>
	class TTiledDeferredLightingMat : public ITiledDeferredLightingMat, public RendererMaterial<TTiledDeferredLightingMat<MSAA_COUNT>>
	{
		RMAT_DEF("TiledDeferredLighting.bsl");

	public:
		TTiledDeferredLightingMat();

		/** @copydoc ITiledDeferredLightingMat::execute() */
		void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
			bool noLighting) override;

		/** @copydoc ITiledDeferredLightingMat::setLights() */
		void setLights(const GPULightData& lightData) override;
	private:
		TiledDeferredLighting mInternal;
	};

	/** Contains instances for all types of tile deferred lighting materials. */
	class TiledDeferredLightingMaterials
	{
	public:
		TiledDeferredLightingMaterials();
		~TiledDeferredLightingMaterials();

		/**
		 * Returns a version of the tile-deferred lighting material that matches the parameters.
		 * 
		 * @param[in]   msaa					Number of samples per pixel.
		 */
		ITiledDeferredLightingMat* get(UINT32 msaa);

	private:
		ITiledDeferredLightingMat* mInstances[4];
	};

	BS_PARAM_BLOCK_BEGIN(FlatFramebufferToTextureParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gFramebufferSize)
		BS_PARAM_BLOCK_ENTRY(INT32, gSampleCount)
	BS_PARAM_BLOCK_END

	/** Shader that copies a flattened framebuffer into a multisampled texture. */
	class FlatFramebufferToTextureMat : public RendererMaterial<FlatFramebufferToTextureMat>
	{
		RMAT_DEF("FlatFramebufferToTexture.bsl");

	public:
		FlatFramebufferToTextureMat();

		/** Binds the material for rendering, sets up parameters and executes it. */
		void execute(const SPtr<GpuBuffer>& flatFramebuffer, const SPtr<Texture>& target);
	private:
		GpuParamBuffer mInputParam;
		SPtr<GpuParamBlockBuffer> mParamBuffer;
	};

	/** @} */
}}
