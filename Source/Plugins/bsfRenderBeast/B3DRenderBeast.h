//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererMaterial.h"
#include "B3DRendererView.h"
#include "B3DRenderBeastScene.h"
#include "Utility/B3DUniformBufferPools.h"

namespace b3d
{
	struct EvaluatedAnimationData;

	namespace render
	{
		class LightGrid;
		struct LoadedRendererTextures;

		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Contains information global to an entire frame. */
		struct FrameInfo
		{
			FrameInfo(const FrameTimings& timings, bool isUsingAsynchronousAnimation, PerSceneFrameData perSceneFrameData)
				: Timings(timings), PerSceneFrameData(perSceneFrameData), IsUsingAsynchronousAnimation(isUsingAsynchronousAnimation)
			{}

			FrameTimings Timings;
			PerSceneFrameData PerSceneFrameData;
			bool IsUsingAsynchronousAnimation;
		};

		/** Per-object parameter set layout and dynamic offset index for normal renderables. */
		struct RenderableParameterSetInfo
		{
			TShared<GpuPipelineParameterSetLayout> Layout;
			u32 PerObjectDynamicOffsetIndex = 0;
		};

		/** Per-object parameter set layout and dynamic offset indices for decals. */
		struct DecalParameterSetInfo
		{
			TShared<GpuPipelineParameterSetLayout> Layout;
			u32 PerObjectDynamicOffsetIndex = 0;
			u32 DecalDynamicOffsetIndex = 0;
		};

		/** Per-object parameter set layout and dynamic offset indices for GPU-simulated particles. */
		struct GpuParticlesParameterSetInfo
		{
			TShared<GpuPipelineParameterSetLayout> Layout;
			u32 PerObjectDynamicOffsetIndex = 0;
			u32 GpuParticlesDynamicOffsetIndex = 0;
		};

		/**
		 * Default framework renderer. Performs frustum culling, sorting and renders all scene objects while applying
		 * lighting, shadowing, special effects and post-processing.
		 */
		class RenderBeast : public Renderer
		{
			/** Renderer information for a single material. */
			struct RendererMaterial
			{
				Vector<TShared<MaterialParameterAdapter>> Params;
				u32 MatVersion;
			};

		public:
			RenderBeast();
			~RenderBeast() = default;

			const StringID& GetName() const override;
			void RenderAll(PerFrameData perFrameData) override;
			void SetOptions(const TShared<RendererOptions>& options) override;
			TShared<RendererOptions> GetOptions() const override;

			/** Returns the feature set the renderer is operating on. Render thread only. */
			RenderBeastFeatureSet GetFeatureSet() const { return mFeatureSet; }

			/** Returns the per-object parameter set info for normal renderables. */
			const RenderableParameterSetInfo& GetRenderableParameterSetInfo() const { return mRenderableParameterSetInfo; }

			/** Returns the per-object parameter set info for decals. */
			const DecalParameterSetInfo& GetDecalParameterSetInfo() const { return mDecalParameterSetInfo; }

			/** Returns the per-object parameter set info for GPU-simulated particles. */
			const GpuParticlesParameterSetInfo& GetGpuParticlesParameterSetInfo() const { return mGpuParticlesParameterSetInfo; }

			/** Returns the type configurations for the renderable uniform buffer manager. */
			const TInlineArray<UniformBufferPools::PoolConfiguration, 4>& GetPerObjectUniformTypeConfigurations() const { return mTypeConfigurations; }

			void Initialize(const TShared<GpuDevice>& gpuDevice) override;
			void Destroy() override;
			void CaptureSceneCubeMap(RendererScene& scene, GpuCommandBuffer& commandBuffer, const TShared<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings) override;
			void RequestDebugFrameCapture() override { mIsFrameCaptureRequested = true; }
			void RequestScreenCapture(Camera* camera, TAsyncOp<TShared<PixelData>> asyncOp) override;
			TShared<GpuDevice> GetGpuDevice() const { return mDevice; }
			TShared<RendererScene> CreateScene() override;

		private:
			friend class RenderBeastScene;

			/**
			 * Updates the render options on the render thread.
			 *
			 * @note	Render thread only.
			 */
			void SyncOptions(const RenderBeastOptions& options);

			/**
			 * Performs rendering over all camera proxies.
			 *
			 * @param[in]	timings			Information about frame time and frame index.
			 * @param[in]	perFrameData	Per-frame data provided by external systems.
			 *
			 * @note	Render thread only.
			 */
			void RenderAllOnRenderThread(FrameTimings timings, PerFrameData perFrameData);

			/**
			 * Renders all views in the provided scene. Returns true if anything has been draw to any of the views.
			 *
			 * @note	Render thread only.
			 */
			bool RenderScene(RenderBeastScene& scene, const FrameInfo& frameInfo);

			/**
			 * Renders all views in the provided view group. Returns true if anything has been draw to any of the views.
			 *
			 * @param	commandBuffer			Command buffer into which to record the render commands.
			 * @param	scene					Owner scene of the view being drawn.
			 * @param	viewGroup				Group of views to render. Usually there's one view per group, but e.g. when rendering cubemaps it can be 6.
			 * @param	frameInfo				Information about the current frame.
			 * @param	forceRender				Forces the overlay to render even if the views and extensions don't request a render.
			 *
			 * @note	Render thread only.
			 */
			bool RenderViews(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, RendererViewGroup& viewGroup, const FrameInfo& frameInfo, bool forceRender);

			/**
			 * Renders all objects visible by the provided view.
			 *
			 * @param	commandBuffer			Command buffer into which to record the render commands.
			 * @param	scene					Owner scene of the view being drawn.
			 * @param	viewGroup				Group of views that the view belong to. Usually there's one view per group, but e.g. when rendering cubemaps it can be 6.
			 * @param	view					View being drawn.
			 * @param	frameInfo				Information about the current frame.
			 *
			 * @note	Render thread only.
			 */
			void RenderView(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo);

			/**
			 * Renders all overlay callbacks of the provided view. Returns true if anything has been rendered in any of the views.
			 *
			 * @param	commandBuffer			Command buffer into which to record the render commands.
			 * @param	scene					Owner scene of the view being drawn.
			 * @param	view					View being drawn.
			 * @param	frameInfo				Information about the current frame.
			 * @param	forceRender				Forces the overlay to render even if the view and extensions don't request a render.
			 *
			 * @note	Render thread only.
			 */
			bool RenderOverlay(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, RendererView& view, const FrameInfo& frameInfo, bool forceRender);

			/**	Creates data used by the renderer on the render thread. */
			void InitializeOnRenderThread(const LoadedRendererTextures& rendererTextures);

			/**	Destroys data used by the renderer on the render thread. */
			void DestroyOnRenderThread() override;

			/** Called right after a renderer scene has been created. */
			void NotifySceneCreated(const TShared<RenderBeastScene>& scene);

			/** Called just before a renderer scene is destroyed. */
			void NotifySceneDestroyed(const RenderBeastScene* scene);

			// Render thread only fields
			RenderBeastFeatureSet mFeatureSet = RenderBeastFeatureSet::Desktop;
			bool mIsFrameCaptureRequested = false;

			// Per-object parameter set layouts and dynamic offset indices
			RenderableParameterSetInfo mRenderableParameterSetInfo;
			DecalParameterSetInfo mDecalParameterSetInfo;
			GpuParticlesParameterSetInfo mGpuParticlesParameterSetInfo;
			TInlineArray<UniformBufferPools::PoolConfiguration, 4> mTypeConfigurations;

			// Scene data
			Vector<RenderBeastScene*> mScenes;

			TShared<RenderBeastOptions> mRenderThreadOptions;

			// Helpers to avoid memory allocations
			RendererViewGroup* mMainViewGroup = nullptr;

			// Main thread only fields
			TShared<RenderBeastOptions> mOptions;
			bool mOptionsDirty = true;

			// Transient
			Vector<RendererExtension*> mOverlayExtensions;
		};

		/**	Provides easy access to the RenderBeast renderer. */
		TShared<RenderBeast> GetRenderBeast();

		/** @} */
	} // namespace render
} // namespace b3d
