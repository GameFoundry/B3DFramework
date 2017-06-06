//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRendererMaterial.h"
#include "BsParamBlocks.h"

namespace bs { namespace ct
{
	struct SceneInfo;
	class RendererViewGroup;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Information about a single reflection probe, as seen by the lighting shader. */
	struct ReflProbeData
	{
		Vector3 position;
		float radius;
		Vector3 boxExtents;
		float transitionDistance;
		Matrix4 invBoxTransform;
		UINT32 cubemapIdx;
		UINT32 type;
		Vector2 padding;
	};

	/** Contains GPU buffers used by the renderer to manipulate reflection probes. */
	class VisibleReflProbeData
	{
	public:
		VisibleReflProbeData();

		/** 
		 * Updates the internal buffers with a new set of refl. probes. Before calling make sure that probe visibility has
		 * been calculated for the provided view group.
		 */
		void update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup);

		/** Returns a GPU bindable buffer containing information about every reflection probe. */
		SPtr<GpuBuffer> getProbeBuffer() const { return mProbeBuffer; }

		/** Returns the number of reflection probes in the probe buffer. */
		UINT32 getNumProbes() const { return mNumProbes; }

	private:
		SPtr<GpuBuffer> mProbeBuffer;
		UINT32 mNumProbes;

		// Helper to avoid memory allocations
		Vector<ReflProbeData> mReflProbeDataTemp;
	};

	BS_PARAM_BLOCK_BEGIN(ReflProbeParamsParamDef)
		BS_PARAM_BLOCK_ENTRY(INT32, gReflCubemapNumMips)
		BS_PARAM_BLOCK_ENTRY(INT32, gNumProbes)
		BS_PARAM_BLOCK_ENTRY(INT32, gSkyCubemapAvailable)
		BS_PARAM_BLOCK_ENTRY(INT32, gUseReflectionMaps)
		BS_PARAM_BLOCK_ENTRY(INT32, gSkyCubemapNumMips)
		BS_PARAM_BLOCK_ENTRY(float, gSkyBrightness)
	BS_PARAM_BLOCK_END

	extern ReflProbeParamsParamDef gReflProbeParamsParamDef;

	/**	Renderer information specific to a single reflection probe. */
	class RendererReflectionProbe
	{
	public:
		RendererReflectionProbe(ReflectionProbe* probe);

		/** Populates the structure with reflection probe parameters. */
		void getParameters(ReflProbeData& output) const;

		ReflectionProbe* probe;
		UINT32 arrayIdx;
		SPtr<Texture> texture;
		bool customTexture : 1;
		bool textureDirty : 1;
		bool arrayDirty : 1;
		mutable bool errorFlagged : 1;
	};

	BS_PARAM_BLOCK_BEGIN(TiledImageBasedLightingParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gFramebufferSize)
	BS_PARAM_BLOCK_END

	extern TiledImageBasedLightingParamDef gTiledImageBasedLightingParamDef;

	/** Helper struct containing all parameters for binding image lighting related data to the GPU programs using them .*/
	struct ImageBasedLightingParams
	{
		/** 
		 * Initializes the parameters from the provided @p params object. 
		 *
		 * @param[in]	paramsSet	GPU parameters object to look for the parameters in.
		 * @param[in]	programType	Type of the GPU program to look up the parameters for.
		 * @param[in]	optional	If true no warnings will be thrown if some or all of the parameters will be found.
		 * @param[in]	gridIndices	Set to true if grid indices (used by light grid) parameter is required.
		 */
		void populate(const SPtr<GpuParamsSet>& paramsSet, GpuProgramType programType, bool optional, bool gridIndices);

		GpuParamTexture skyReflectionsTexParam;
		GpuParamSampState skyReflectionsSampParam;

		GpuParamTexture skyIrradianceTexParam;

		GpuParamTexture reflectionProbeCubemapsTexParam;
		GpuParamSampState reflectionProbeCubemapsSampParam;

		GpuParamTexture preintegratedEnvBRDFParam;
		GpuParamBuffer reflectionProbesParam;

		GpuParamBuffer reflectionProbeIndicesParam;
		UINT32 reflProbeParamsBindingIdx;
	};

	/** Functionality common to all versions of TiledDeferredImageBasedLightingMat<T>. */
	class TiledDeferredImageBasedLighting
	{
	public:
		TiledDeferredImageBasedLighting(const SPtr<Material>& material, const SPtr<GpuParamsSet>& paramsSet, 
			UINT32 sampleCount);

		/** Binds the material for rendering, sets up parameters and executes it. */
		void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
					 const SPtr<Texture>& preintegratedGF);

		/** Binds all the active reflection probes. */
		void setReflectionProbes(const VisibleReflProbeData& probeData, const SPtr<Texture>& reflectionCubemaps, 
			bool capturingReflections);

		/** Binds the sky reflection & irradiance textures. Set textures to null if not available. */
		void setSky(const SPtr<Texture>& skyReflections, const SPtr<Texture>& skyIrradiance, float brightness);

		/** Returns a param buffer containing information required to evaluate reflection probe data. */
		SPtr<GpuParamBlockBuffer> getReflectionsParamBuffer() const { return mReflectionsParamBuffer; }

		/** Returns a sampler state object that should be used for evaluating reflection maps. */
		SPtr<SamplerState> getReflectionsSamplerState() const { return mReflectionSamplerState; }

		/**
		 * Generates a 2D 2-channel texture containing a pre-integrated G and F factors of the microfactet BRDF. This is an
		 * approximation used for image based lighting, so we can avoid sampling environment maps for each light. Works in
		 * tandem with the importance sampled reflection cubemaps.
		 * 
		 * (u, v) = (NoV, roughness)
		 * (r, g) = (scale, bias)
		 */
		static SPtr<Texture> generatePreintegratedEnvBRDF();

		static const UINT32 TILE_SIZE;
	private:
		UINT32 mSampleCount;
		SPtr<Material> mMaterial;
		SPtr<GpuParamsSet> mParamsSet;

		GpuParamTexture mGBufferA;
		GpuParamTexture mGBufferB;
		GpuParamTexture mGBufferC;
		GpuParamTexture mGBufferDepth;

		GpuParamTexture mInColorTextureParam;

		ImageBasedLightingParams mImageBasedParams;

		GpuParamLoadStoreTexture mOutputTextureParam;
		GpuParamBuffer mOutputBufferParam;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		SPtr<GpuParamBlockBuffer> mReflectionsParamBuffer;
		SPtr<SamplerState> mReflectionSamplerState;
	};

	/** Interface implemented by all versions of TTiledDeferredImageBasedLightingMat<T>. */
	class ITiledDeferredImageBasedLightingMat
	{
	public:
		virtual ~ITiledDeferredImageBasedLightingMat() {}

		/** @copydoc TiledDeferredImageBasedLighting::execute() */
		virtual void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
			const SPtr<Texture>& preintegratedGF) = 0;

		/** @copydoc TiledDeferredImageBasedLighting::setReflectionProbes() */
		virtual void setReflectionProbes(const VisibleReflProbeData& probeData, const SPtr<Texture>& reflectionCubemaps,
			bool capturingReflections) = 0;

		/** @copydoc TiledDeferredImageBasedLighting::setSky() */
		virtual void setSky(const SPtr<Texture>& skyReflections, const SPtr<Texture>& skyIrradiance, float brightness) = 0;

		/** @copydoc TiledDeferredImageBasedLighting::getReflectionsParamBuffer() */
		virtual SPtr<GpuParamBlockBuffer> getReflectionsParamBuffer() const = 0;

		/** @copydoc TiledDeferredImageBasedLighting::getReflectionsSamplerState() */
		virtual SPtr<SamplerState> getReflectionsSamplerState() const = 0;
	};

	/** Shader that performs a lighting pass over data stored in the Gbuffer. */
	template<int MSAA_COUNT>
	class TTiledDeferredImageBasedLightingMat : public ITiledDeferredImageBasedLightingMat, 
		public RendererMaterial<TTiledDeferredImageBasedLightingMat<MSAA_COUNT>>
	{
		RMAT_DEF("TiledDeferredImageBasedLighting.bsl");

	public:
		TTiledDeferredImageBasedLightingMat();

		/** @copydoc ITiledDeferredImageBasedLightingMat::execute() */
		void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
			const SPtr<Texture>& preintegratedGF) override;

		/** @copydoc ITiledDeferredImageBasedLightingMat::setReflectionProbes() */
		void setReflectionProbes(const VisibleReflProbeData& probeData, const SPtr<Texture>& reflectionCubemaps, 
			bool capturingReflections) override;

		/** @copydoc ITiledDeferredImageBasedLightingMat::setSky() */
		void setSky(const SPtr<Texture>& skyReflections, const SPtr<Texture>& skyIrradiance, float brightness) override;

		/** @copydoc ITiledDeferredImageBasedLightingMat::getReflectionsParamBuffer() */
		SPtr<GpuParamBlockBuffer> getReflectionsParamBuffer() const override;

		/** @copydoc ITiledDeferredImageBasedLightingMat::getReflectionsSamplerState() */
		SPtr<SamplerState> getReflectionsSamplerState() const override;
	private:
		TiledDeferredImageBasedLighting mInternal;
	};

	/** Contains instances for all types of tile deferred image based lighting materials. */
	class TiledDeferredImageBasedLightingMaterials
	{
	public:
		TiledDeferredImageBasedLightingMaterials();
		~TiledDeferredImageBasedLightingMaterials();

		/**
		 * Returns a version of the tile-deferred image based lighting material that matches the parameters.
		 *
		 * @param[in]   msaa						Number of samples per pixel.
		 */
		ITiledDeferredImageBasedLightingMat* get(UINT32 msaa);

	private:
		ITiledDeferredImageBasedLightingMat* mInstances[4];
	};

	/** @} */
}}