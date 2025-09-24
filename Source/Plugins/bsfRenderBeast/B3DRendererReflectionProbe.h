//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuDataParameterBlock.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"
#include "B3DRendererLight.h"

namespace b3d
{
	namespace render
	{
		struct SkyInfo;
		struct SceneInfo;
		class RendererViewGroup;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Maximum number of refl. probes that can influence an object when basic forward rendering is used. */
		static constexpr u32 kStandardForwardMaxNumProbes = 8;

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

		B3D_PARAM_BLOCK_BEGIN(ReflProbeParamsParamDef)
			B3D_PARAM_BLOCK_ENTRY(i32, gReflCubemapNumMips)
			B3D_PARAM_BLOCK_ENTRY(i32, gNumProbes)
			B3D_PARAM_BLOCK_ENTRY(i32, gSkyCubemapAvailable)
			B3D_PARAM_BLOCK_ENTRY(i32, gUseReflectionMaps)
			B3D_PARAM_BLOCK_ENTRY(i32, gSkyCubemapNumMips)
			B3D_PARAM_BLOCK_ENTRY(float, gSkyBrightness)
		B3D_PARAM_BLOCK_END

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
			void Populate(const SPtr<GpuParameters>& params, GpuProgramType programType, bool optional, bool gridIndices, bool probeArray);

			GpuParameterSampledTexture SkyReflectionsTexParam;
			GpuParameterSampledTexture AmbientOcclusionTexParam;
			GpuParameterSampledTexture SsrTexParam;
			GpuParameterSampledTexture ReflectionProbeCubemapsTexParam;

			GpuParameterSampledTexture PreintegratedEnvBrdfParam;
			GpuParameterBuffer ReflectionProbesParam;

			GpuParameterBuffer ReflectionProbeIndicesParam;
			GpuParameterBinding ReflProbeParamBindings;

			// Only utilized when standard forward rendering is used
			GpuParameterBinding ReflProbesBinding;
		};

		/** Parameter buffer containing information about reflection probes. */
		struct ReflProbeParamBuffer
		{
			ReflProbeParamBuffer();

			/** Updates the parameter buffer contents with required refl. probe data. */
			void Populate(const Skybox* sky, u32 numProbes, const SPtr<Texture>& reflectionCubemaps, bool capturingReflections);

			SPtr<GpuBuffer> Buffer;
		};

		B3D_PARAM_BLOCK_BEGIN(ReflProbesParamDef)
			B3D_PARAM_BLOCK_ENTRY_ARRAY(ReflProbeData, gReflectionProbes, kStandardForwardMaxNumProbes)
		B3D_PARAM_BLOCK_END

		extern ReflProbesParamDef gReflProbesParamDef;

		/** @} */
	} // namespace render
} // namespace b3d
