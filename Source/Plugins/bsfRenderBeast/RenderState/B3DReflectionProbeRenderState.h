//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "RenderState/B3DLightRenderState.h"

namespace b3d
{
	namespace render
	{
		struct SkyInfo;
		class RenderBeastScene;
		class RendererViewGroup;
		class ReflectionProbeProxy;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Information about a single reflection probe, as seen by the lighting shader. */
		struct ReflectioneProbeData
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
		class VisibleReflectionProbeData
		{
		public:
			VisibleReflectionProbeData() = default;

			/**
			 * Updates the internal buffers with a new set of refl. probes. Before calling make sure that probe visibility has
			 * been calculated for the provided view group.
			 */
			void Update(const RenderBeastScene& scene, const RendererViewGroup& viewGroup);

			/** Returns a GPU bindable buffer containing information about every reflection probe. */
			TShared<GpuBuffer> GetProbeBuffer() const { return mProbeBuffer; }

			/** Returns the number of reflection probes in the probe buffer. */
			u32 GetProbeCount() const { return mNumProbes; }

			/** Returns information about a probe at the specified index. */
			const ReflectioneProbeData& GetProbeData(u32 idx) const { return mReflProbeData[idx]; }

		private:
			Vector<ReflectioneProbeData> mReflProbeData;
			TShared<GpuBuffer> mProbeBuffer;
			u32 mNumProbes = 0;
		};

		B3D_UNIFORM_BUFFER_BEGIN(GlobalReflectionProbeUniformBufferDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gReflCubemapNumMips)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gNumProbes)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gSkyCubemapAvailable)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gUseReflectionMaps)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gSkyCubemapNumMips)
			B3D_UNIFORM_BUFFER_MEMBER(float, gSkyBrightness)
		B3D_UNIFORM_BUFFER_END

		extern GlobalReflectionProbeUniformBufferDefinition gGlobalReflectionProbeUniformBufferDefinition;

		/**	Renderer information specific to a single reflection probe. */
		class ReflectionProbeRenderState
		{
		public:
			/** Default constructor for packed array usage. */
			ReflectionProbeRenderState();

			/** Populates the structure with reflection probe parameters. */
			void GetParameters(const ReflectionProbeProxy& proxy, ReflectioneProbeData& output) const;

			/**
			 * Populates a transient uniform buffer with reflection probe parameters.
			 *
			 * @param uniformBuffer			Buffer suballocation to populate.
			 * @param sky					Skybox to use for sky reflections (can be null).
			 * @param probeCount			Number of reflection probes.
			 * @param reflectionCubemaps	Texture array containing reflection probe cubemaps.
			 * @param capturingReflections	True if currently capturing reflections (disables reflection map usage).
			 */
			static void PopulateGlobalReflectionProbeUniformBuffer(const GpuBufferSuballocation& uniformBuffer, const Skybox* sky, u32 probeCount, const TShared<Texture>& reflectionCubemaps, bool capturingReflections);

			u32 ArrayIdx;
			bool ArrayDirty : 1;
			mutable bool ErrorFlagged : 1;
		};

		/** Helper struct containing all parameters for binding image lighting related data to the GPU programs using them .*/
		struct ImageBasedLightingParameterBinding
		{
			static constexpr const char* kSkyReflectionTextureName = "gSkyReflectionTex";
			static constexpr const char* kReflectionProbeCubemapsTextureName = "gReflProbeCubemaps";
			static constexpr const char* kPreintegratedEnvBRDFTextureName = "gPreintegratedEnvBRDF";
			static constexpr const char* kAmbientOcclusionTextureName = "gAmbientOcclusionTex";
			static constexpr const char* kSSRTexName = "gSSRTex";
			static constexpr const char* kReflectionProbesBufferName = "gReflectionProbes";

			/** Looks the parameters up on set 0 of @p parameterLayout. */
			void Initialize(const GpuPipelineParameterLayout& parameterLayout);

			GpuParameterSampledTexture SkyReflectionsTexParam;
			GpuParameterSampledTexture AmbientOcclusionTexParam;
			GpuParameterSampledTexture SsrTexParameter;
			GpuParameterSampledTexture ReflectionProbeCubemapsTexParameter;

			GpuParameterSampledTexture PreintegratedEnvBrdfParameter;
			GpuParameterStorageBuffer ReflectionProbesParameter;
		};

		/** @} */
	} // namespace render
} // namespace b3d
