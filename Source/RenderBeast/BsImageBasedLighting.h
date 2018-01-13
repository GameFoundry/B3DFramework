//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsParamBlocks.h"
#include "BsLightRendering.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"

namespace bs { namespace ct
{
	struct SkyInfo;
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
		UINT32 type; // 0 - Sphere, 1 - Box
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

		/** Returns information about a probe at the specified index. */
		const ReflProbeData& getProbeData(UINT32 idx) const { return mReflProbeData[idx]; }

	private:
		Vector<ReflProbeData> mReflProbeData;
		SPtr<GpuBuffer> mProbeBuffer;
		UINT32 mNumProbes;
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
		 * Initializes the parameters from the provided parameters. 
		 *
		 * @param[in]	params		GPU parameters object to look for the parameters in.
		 * @param[in]	programType	Type of the GPU program to look up the parameters for.
		 * @param[in]	optional	If true no warnings will be thrown if some or all of the parameters will be found.
		 * @param[in]	gridIndices	Set to true if grid indices (used by light grid) parameter is required.
		 * @param[in]	probeArray	True if the refl. probe data is to be provided in a structured buffer.
		 */
		void populate(const SPtr<GpuParams>& params, GpuProgramType programType, bool optional, bool gridIndices, 
			bool probeArray);

		GpuParamTexture skyReflectionsTexParam;
		GpuParamTexture ambientOcclusionTexParam;
		GpuParamTexture ssrTexParam;
		GpuParamTexture reflectionProbeCubemapsTexParam;

		GpuParamTexture preintegratedEnvBRDFParam;
		GpuParamBuffer reflectionProbesParam;

		GpuParamBuffer reflectionProbeIndicesParam;
		GpuParamBinding reflProbeParamBindings;
	};

	/** Parameter buffer containing information about reflection probes. */
	struct ReflProbeParamBuffer
	{
		ReflProbeParamBuffer();

		/** Updates the parameter buffer contents with required refl. probe data. */
		void populate(const Skybox* sky, UINT32 numProbes, const SPtr<Texture>& reflectionCubemaps, 
			bool capturingReflections);

		SPtr<GpuParamBlockBuffer> buffer;
	};

	/** Shader that performs a lighting pass over data stored in the Gbuffer. */
	class TiledDeferredImageBasedLightingMat : public RendererMaterial<TiledDeferredImageBasedLightingMat>
	{
		RMAT_DEF_CUSTOMIZED("TiledDeferredImageBasedLighting.bsl");

		/** Helper method used for initializing variations of this material. */
		template<UINT32 msaa>
		static const ShaderVariation& getVariation()
		{
			static ShaderVariation variation = ShaderVariation({
				ShaderVariation::Param("MSAA_COUNT", msaa)
			});

			return variation;
		}
	public:
		/** Container for parameters to be passed to the execute() method. */
		struct Inputs
		{
			GBufferTextures gbuffer;
			SPtr<Texture> lightAccumulation;
			SPtr<Texture> sceneColorTex;
			SPtr<GpuBuffer> sceneColorBuffer;
			SPtr<Texture> preIntegratedGF;
			SPtr<Texture> ambientOcclusion;
			SPtr<Texture> ssr;
			SPtr<Texture> msaaCoverage;
		};

		TiledDeferredImageBasedLightingMat();

		/** Binds the material for rendering, sets up parameters and executes it. */
		void execute(const RendererView& view, const SceneInfo& sceneInfo, const VisibleReflProbeData& probeData, 
			const Inputs& inputs);

		/** Returns the material variation matching the provided parameters. */
		static TiledDeferredImageBasedLightingMat* getVariation(UINT32 msaaCount);

	private:
		UINT32 mSampleCount;

		GpuParamTexture mGBufferA;
		GpuParamTexture mGBufferB;
		GpuParamTexture mGBufferC;
		GpuParamTexture mGBufferDepth;

		GpuParamTexture mInColorTextureParam;
		GpuParamTexture mMSAACoverageTexParam;

		ImageBasedLightingParams mImageBasedParams;

		GpuParamLoadStoreTexture mOutputTextureParam;
		GpuParamBuffer mOutputBufferParam;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		ReflProbeParamBuffer mReflProbeParamBuffer;

		static const UINT32 TILE_SIZE;
	};

	/** @} */
}}
