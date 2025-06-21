//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererMaterial.h"
#include "BsRendererView.h"
#include "BsRenderBeastScene.h"

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
			FrameInfo(const FrameTimings& timings, PerFrameData perFrameData)
				: Timings(timings), PerFrameData(perFrameData)
			{}

			FrameTimings Timings;
			PerFrameData PerFrameData;
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
				Vector<SPtr<GpuParamsSet>> Params;
				u32 MatVersion;
			};

		public:
			RenderBeast();
			~RenderBeast() = default;

			const StringID& GetName() const override;
			void RenderAll(PerFrameData perFrameData) override;
			void SetOptions(const SPtr<RendererOptions>& options) override;
			SPtr<RendererOptions> GetOptions() const override;

			/** Returns the feature set the renderer is operating on. Render thread only. */
			RenderBeastFeatureSet GetFeatureSet() const { return mFeatureSet; }

			void Initialize(const SPtr<GpuDevice>& gpuDevice) override;
			void Destroy() override;
			void CaptureSceneCubeMap(RendererScene& scene, GpuCommandBuffer& commandBuffer, const SPtr<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings) override;
			void RequestFrameCapture() override { mIsFrameCaptureRequested = true; }
			SPtr<RendererScene> CreateScene() override;

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
			void RenderAllScenes(FrameTimings timings, PerFrameData perFrameData);

			/**
			 * Renders all views in the provided scene. Returns true if anything has been draw to any of the views.
			 *
			 * @note	Render thread only.
			 */
			bool RenderScene(RenderBeastScene& scene, const FrameInfo& frameInfo);

			/**
			 * Renders all views in the provided view group. Returns true if anything has been draw to any of the views.
			 *
			 * @note	Render thread only.
			 */
			bool RenderViews(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, RendererViewGroup& viewGroup, const FrameInfo& frameInfo);

			/**
			 * Renders all objects visible by the provided view.
			 *
			 * @note	Render thread only.
			 */
			void RenderView(GpuCommandBuffer& commandBuffer, RenderBeastScene& scene, const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo);

			/**
			 * Renders all overlay callbacks of the provided view. Returns true if anything has been rendered in any of the views.
			 *
			 * @note	Render thread only.
			 */
			bool RenderOverlay(GpuCommandBuffer& commandBuffer, RendererView& view, const FrameInfo& frameInfo);

			/**	Creates data used by the renderer on the render thread. */
			void InitializeOnRenderThread(const LoadedRendererTextures& rendererTextures);

			/**	Destroys data used by the renderer on the render thread. */
			void DestroyOnRenderThread() override;

			/** Called right after a renderer scene has been created. */
			void NotifySceneCreated(const WeakSPtr<RenderBeastScene>& scene);

			/** Called just before a renderer scene is destroyed. */
			void NotifySceneDestroyed(const RenderBeastScene* scene);

			// Render thread only fields
			RenderBeastFeatureSet mFeatureSet = RenderBeastFeatureSet::Desktop;
			bool mIsFrameCaptureRequested = false;

			// Scene data
			Vector<WeakSPtr<RenderBeastScene>> mScenes;

			SPtr<RenderBeastOptions> mRenderThreadOptions;

			// Helpers to avoid memory allocations
			RendererViewGroup* mMainViewGroup = nullptr;

			// Main thread only fields
			SPtr<RenderBeastOptions> mOptions;
			bool mOptionsDirty = true;

			// Transient
			Vector<RendererExtension*> mOverlayExtensions;
		};

		/**	Provides easy access to the RenderBeast renderer. */
		SPtr<RenderBeast> GetRenderBeast();

		/** @} */
	} // namespace render
} // namespace b3d
