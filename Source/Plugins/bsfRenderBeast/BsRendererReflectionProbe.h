//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRendererMaterial.h"
#include "Renderer/BsParamBlocks.h"
#include "RenderAPI/BsGpuPipelineParamInfo.h"
#include "BsRendererLight.h"

namespace bs
{
	namespace ct
	{
		struct SkyInfo;
		struct SceneInfo;
		class RendererViewGroup;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Maximum number of refl. probes that can influence an object when basic forward rendering is used. */
		static constexpr u32 STANDARD_FORWARD_MAX_NUM_PROBES = 8;

		/** Information about a single reflection probe, as seen by the lighting shader. */
		struct ReflProbeData
		{
			Vector3 Position;
			float Radius;
			Vector3 BoxExtents;
			float TransitionDistance;
			Matrix4 InvBoxTransform;
			u32 CubemapIdx;
			u32 Type; // 0 - Sphere, 1 - Box
			Vector2 Padding;
		};

		/** Contains GPU buffers used by the renderer to manipulate reflection probes. */
		class VisibleReflProbeData
		{
		public:
			VisibleReflProbeData() = default;

			/**
			 * Updates the internal buffers with a new set of refl. probes. Before calling make sure that probe visibility has
			 * been calculated for the provided view group.
			 */
			void Update(const SceneInfo& sceneInfo, const RendererViewGroup& viewGroup);

			/** Returns a GPU bindable buffer containing information about every reflection probe. */
			SPtr<GpuBuffer> GetProbeBuffer() const { return mProbeBuffer; }

			/** Returns the number of reflection probes in the probe buffer. */
			u32 GetNumProbes() const { return mNumProbes; }

			/** Returns information about a probe at the specified index. */
			const ReflProbeData& GetProbeData(u32 idx) const { return mReflProbeData[idx]; }

		private:
			Vector<ReflProbeData> mReflProbeData;
			SPtr<GpuBuffer> mProbeBuffer;
			u32 mNumProbes = 0;
		};

		BS_PARAM_BLOCK_BEGIN(ReflProbeParamsParamDef)
			BS_PARAM_BLOCK_ENTRY(i32, gReflCubemapNumMips)
			BS_PARAM_BLOCK_ENTRY(i32, gNumProbes)
			BS_PARAM_BLOCK_ENTRY(i32, gSkyCubemapAvailable)
			BS_PARAM_BLOCK_ENTRY(i32, gUseReflectionMaps)
			BS_PARAM_BLOCK_ENTRY(i32, gSkyCubemapNumMips)
			BS_PARAM_BLOCK_ENTRY(float, gSkyBrightness)
		BS_PARAM_BLOCK_END

		extern ReflProbeParamsParamDef gReflProbeParamsParamDef;

		/**	Renderer information specific to a single reflection probe. */
		class RendererReflectionProbe
		{
		public:
			RendererReflectionProbe(ReflectionProbe* probe);

			/** Populates the structure with reflection probe parameters. */
			void GetParameters(ReflProbeData& output) const;

			ReflectionProbe* Probe;
			u32 ArrayIdx;
			bool ArrayDirty : 1;
			mutable bool ErrorFlagged : 1;
		};

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
			void Populate(const SPtr<GpuParams>& params, GpuProgramType programType, bool optional, bool gridIndices, bool probeArray);

			GpuParamTexture SkyReflectionsTexParam;
			GpuParamTexture AmbientOcclusionTexParam;
			GpuParamTexture SsrTexParam;
			GpuParamTexture ReflectionProbeCubemapsTexParam;

			GpuParamTexture PreintegratedEnvBrdfParam;
			GpuParamBuffer ReflectionProbesParam;

			GpuParamBuffer ReflectionProbeIndicesParam;
			GpuParamBinding ReflProbeParamBindings;

			// Only utilized when standard forward rendering is used
			GpuParamBinding ReflProbesBinding;
		};

		/** Parameter buffer containing information about reflection probes. */
		struct ReflProbeParamBuffer
		{
			ReflProbeParamBuffer();

			/** Updates the parameter buffer contents with required refl. probe data. */
			void Populate(const Skybox* sky, u32 numProbes, const SPtr<Texture>& reflectionCubemaps, bool capturingReflections);

			SPtr<GpuParamBlockBuffer> Buffer;
		};

		BS_PARAM_BLOCK_BEGIN(ReflProbesParamDef)
			BS_PARAM_BLOCK_ENTRY_ARRAY(ReflProbeData, gReflectionProbes, STANDARD_FORWARD_MAX_NUM_PROBES)
		BS_PARAM_BLOCK_END

		extern ReflProbesParamDef gReflProbesParamDef;

		/** @} */
	} // namespace ct
} // namespace bs
