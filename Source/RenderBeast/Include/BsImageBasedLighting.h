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

	/** Information about a single reflection probe, as seen by the lighting shader. */
	struct ReflProbeData
	{
		Vector3 position;
		float radius;
		Vector3 boxExtents;
		Matrix4 invBoxTransform;
		float transitionDistance;
		UINT32 cubemapIdx;
		UINT32 type;
	};

	/** Contains GPU buffers used by the renderer to manipulate reflection probes. */
	class GPUReflProbeData
	{
	public:
		GPUReflProbeData();

		/** Updates the internal buffers with a new set of probes. */
		void setProbes(const Vector<ReflProbeData>& probeData, UINT32 numProbes);

		/** Returns a GPU bindable buffer containing information about every reflection probe. */
		SPtr<GpuBuffer> getProbeBuffer() const { return mProbeBuffer; }

		/** Returns the number of reflection probes in the probe buffer. */
		UINT32 getNumProbes() const { return mNumProbes; }

	private:
		SPtr<GpuBuffer> mProbeBuffer;

		UINT32 mNumProbes;
	};

	BS_PARAM_BLOCK_BEGIN(ReflProbeParamsParamDef)
		BS_PARAM_BLOCK_ENTRY(INT32, gReflCubemapNumMips)
		BS_PARAM_BLOCK_ENTRY(INT32, gNumProbes)
		BS_PARAM_BLOCK_ENTRY(INT32, gSkyCubemapAvailable)
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
		bool errorFlagged : 1;
	};

	BS_PARAM_BLOCK_BEGIN(TiledImageBasedLightingParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector2I, gFramebufferSize)
	BS_PARAM_BLOCK_END

	extern TiledImageBasedLightingParamDef gTiledImageBasedLightingParamDef;

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
		void setReflectionProbes(const GPUReflProbeData& probeData, const SPtr<Texture>& reflectionCubemaps);

		/** Binds the sky reflection & irradiance textures. Set textures to null if not available. */
		void setSky(const SPtr<Texture>& skyReflections, const SPtr<Texture>& skyIrradiance, float brightness);

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
		GpuParamBuffer mInColorBufferParam;

		GpuParamTexture mSkyReflectionsParam;
		GpuParamTexture mSkyIrradianceParam;
		GpuParamTexture mReflectionProbeCubemapsParam;
		GpuParamTexture mPreintegratedEnvBRDFParam;
		GpuParamBuffer mReflectionProbesParam;

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

		/** @copydoc ITiledDeferredImageBasedLightingMat::execute() */
		virtual void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
			const SPtr<Texture>& preintegratedGF) = 0;

		/** @copydoc ITiledDeferredImageBasedLightingMat::setReflectionProbes() */
		virtual void setReflectionProbes(const GPUReflProbeData& probeData, const SPtr<Texture>& reflectionCubemaps) = 0;

		/** @copydoc ITiledDeferredImageBasedLightingMat::setSky() */
		virtual void setSky(const SPtr<Texture>& skyReflections, const SPtr<Texture>& skyIrradiance, float brightness) = 0;
	};

	/** Shader that performs a lighting pass over data stored in the Gbuffer. */
	template<int MSAA_COUNT, bool CapturingReflections>
	class TTiledDeferredImageBasedLightingMat : public ITiledDeferredImageBasedLightingMat, 
		public RendererMaterial<TTiledDeferredImageBasedLightingMat<MSAA_COUNT, CapturingReflections>>
	{
		RMAT_DEF("TiledDeferredImageBasedLighting.bsl");

	public:
		TTiledDeferredImageBasedLightingMat();

		/** @copydoc ITiledDeferredImageBasedLightingMat::execute() */
		void execute(const SPtr<RenderTargets>& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera,
			const SPtr<Texture>& preintegratedGF) override;

		/** @copydoc ITiledDeferredImageBasedLightingMat::setReflectionProbes() */
		void setReflectionProbes(const GPUReflProbeData& probeData, const SPtr<Texture>& reflectionCubemaps) override;

		/** @copydoc ITiledDeferredImageBasedLightingMat::setSky() */
		void setSky(const SPtr<Texture>& skyReflections, const SPtr<Texture>& skyIrradiance, float brightness) override;
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
		* @param[in]   capturingReflections		If true reflection probes will not be evaluated and instead the material's
		*										specular color will be returned instead. Useful when rendering reflection
		*										probes.
		*/
		ITiledDeferredImageBasedLightingMat* get(UINT32 msaa, bool capturingReflections);

	private:
		ITiledDeferredImageBasedLightingMat* mInstances[8];
	};

	/** @} */
}}