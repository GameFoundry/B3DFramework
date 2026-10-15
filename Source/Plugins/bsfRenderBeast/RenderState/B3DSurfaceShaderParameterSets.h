//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "B3DLightRenderState.h"
#include "B3DReflectionProbeRenderState.h"

namespace b3d
{
	namespace render
	{
		struct RenderQueueEntry;
		class FrameGraphRasterPass;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/**
		 * Types of surface shaders, which differ in the per-pass renderer inputs they declare in set
		 * GpuPipelineSet::kSurfaceShaderInputSetIndex. Every shader of a type declares the same inputs through its includes, with
		 * the same register numbers.
		 */
		enum class SurfaceShaderType
		{
			/** Surfaces and particles that write the G-buffer, which the renderer lights later. PerCamera and PerFrame. */
			LitDeferred,

			/** Surfaces and particles with forward lighting. LitDeferred's inputs, plus clustered forward lighting and image based lighting. */
			LitForward,

			/** Transparent surfaces without lighting. LitDeferred's inputs. */
			Unlit,

			/** Transparent particles without lighting. LitDeferred's inputs, plus scene depth. */
			UnlitParticle,

			/** Decals. UnlitParticle's inputs, plus the G-buffer mask. Multisampled when the view is. */
			Decal,

			Count
		};

		/**
		 * Per-pass parameter set layouts for surface shaders, one per SurfaceShaderType. Hand-written to match the declarations
		 * in the shader includes.
		 */
		class SurfaceShaderParameterSetInfo
		{
		public:
			/** Creates the layouts on @p device. */
			void Initialize(GpuDevice& device);

			/** Returns the layout for @p type. @p msaa selects the multisampled scene inputs, which only Decal has. */
			const TShared<GpuPipelineParameterSetLayout>& Get(SurfaceShaderType type, bool msaa) const;

		private:
			Array<TShared<GpuPipelineParameterSetLayout>, (u32)SurfaceShaderType::Count> mLayouts;
			TShared<GpuPipelineParameterSetLayout> mDecalMSAALayout;
		};

		/**
		 * Per-pass parameter sets of surface shaders, one per SurfaceShaderType, holding the renderer inputs of the view a
		 * pass renders. Draw commands select theirs through DrawCommand::SurfaceShaderType.
		 */
		class SurfaceShaderParameterSets
		{
		public:
			/**
			 * Creates the sets and their parameter handles on first use, and recreates the multisampled ones when the view's
			 * sample count changes. Assigns the view's PerCamera and the scene's PerFrame buffers to every set, and the
			 * samplers to a newly created LitForward set.
			 */
			void Prepare(const RendererView& view, const RenderBeastScene& scene);

			/** Assigns the scene depth that the UnlitParticle set reads. */
			void SetSceneDepth(const TShared<Texture>& depth);

			/** Assigns the scene depth and the G-buffer mask that the Decal set reads. */
			void SetDecalInputs(const TShared<Texture>& depth, const TShared<Texture>& mask);

			/**
			 * Assigns the clustered forward lighting and image based lighting inputs that the LitForward set reads.
			 *
			 * @param	view				View whose light grid to read.
			 * @param	lights				Lights visible in the view.
			 * @param	reflectionProbes	Reflection probes visible in the view.
			 * @param	scene				Scene holding the reflection probe cubemaps.
			 * @param	skybox				Skybox providing sky reflections, or null if the view renders none.
			 */
			void SetForwardLightingInputs(const RendererView& view, const VisibleLightData& lights, const VisibleReflectionProbeData& reflectionProbes,
				const RenderBeastScene& scene, const Skybox* skybox);

			/** Adds the sets that @p entries use to @p pass, which declares the graph resources they hold. */
			void DeclareSets(FrameGraphRasterPass& pass, const Vector<RenderQueueEntry>& entries) const;

			/**
			 * Binds the set of @p entry, unless it is @p boundType.
			 *
			 * @param		commandBuffer	Command buffer to bind the set on.
			 * @param		entry			Entry whose set to bind.
			 * @param[in,out]	boundType	Type of the set bound last, or ~0u if none. Updated to the type of @p entry.
			 */
			void Bind(GpuCommandBuffer& commandBuffer, const RenderQueueEntry& entry, u32& boundType) const;

		private:
			/** Handles of the clustered forward lighting and image based lighting inputs of the LitForward set. */
			struct LitForwardParameters
			{
				GpuParameterUniformBuffer GridParams;
				GpuParameterStorageBuffer GridLightOffsetsAndSize;
				GpuParameterStorageBuffer GridProbeOffsetsAndSize;
				GpuParameterStorageBuffer LightIndices;
				GpuParameterStorageBuffer Lights;
				GpuParameterStorageBuffer ReflectionProbeIndices;
				GpuParameterStorageBuffer ReflectionProbes;
				GpuParameterUniformBuffer ReflectionProbeParams;
				GpuParameterSampledTexture SkyReflectionTexture;
				GpuParameterSampledTexture ReflectionProbeCubemaps;
				GpuParameterSampledTexture AmbientOcclusionTexture;
				GpuParameterSampledTexture SSRTexture;
				GpuParameterSampledTexture PreintegratedEnvBRDF;
			};

			Array<TShared<GpuParameterSet>, (u32)SurfaceShaderType::Count> mSets;
			Array<GpuParameterUniformBuffer, (u32)SurfaceShaderType::Count> mPerCameraParameters;
			Array<GpuParameterUniformBuffer, (u32)SurfaceShaderType::Count> mPerFrameParameters;

			GpuParameterSampledTexture mUnlitParticleDepthParameter;
			GpuParameterSampledTexture mDecalDepthParameter;
			GpuParameterSampledTexture mDecalMaskParameter;
			LitForwardParameters mLitForwardParameters;
		};

		/** @} */
	} // namespace render
} // namespace b3d
