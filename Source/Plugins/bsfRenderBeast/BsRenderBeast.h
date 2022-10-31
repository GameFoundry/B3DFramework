//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRendererMaterial.h"
#include "BsRendererView.h"
#include "BsRendererScene.h"

namespace bs
{
	struct EvaluatedAnimationData;

	namespace ct
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

			/** Returns the feature set the renderer is operating on. Core thread only. */
			RenderBeastFeatureSet GetFeatureSet() const { return mFeatureSet; }

			void Initialize() override;
			void Destroy() override;
			void CaptureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position, const CaptureSettings& settings) override;
			ShaderExtensionPointInfo GetShaderExtensionPointInfo(const String& name) override;
			void SetGlobalShaderOverride(const String& name, const SPtr<bs::Shader>& shader) override;

		private:
			void NotifyCameraAdded(Camera* camera) override;
			void NotifyCameraUpdated(Camera* camera, u32 updateFlag) override;
			void NotifyCameraRemoved(Camera* camera) override;
			void NotifyLightAdded(Light* light) override;
			void NotifyLightUpdated(Light* light) override;
			void NotifyLightRemoved(Light* light) override;
			void NotifyRenderableAdded(Renderable* renderable) override;
			void NotifyRenderableUpdated(Renderable* renderable) override;
			void NotifyRenderableRemoved(Renderable* renderable) override;
			void NotifyReflectionProbeAdded(ReflectionProbe* probe) override;
			void NotifyReflectionProbeUpdated(ReflectionProbe* probe, bool texture) override;
			void NotifyReflectionProbeRemoved(ReflectionProbe* probe) override;
			void NotifyLightProbeVolumeAdded(LightProbeVolume* volume) override;
			void NotifyLightProbeVolumeUpdated(LightProbeVolume* volume) override;
			void NotifyLightProbeVolumeRemoved(LightProbeVolume* volume) override;
			void NotifySkyboxAdded(Skybox* skybox) override;
			void NotifySkyboxRemoved(Skybox* skybox) override;
			void NotifyParticleSystemAdded(ParticleSystem* particleSystem) override;
			void NotifyParticleSystemUpdated(ParticleSystem* particleSystem, bool tfrmOnly) override;
			void NotifyParticleSystemRemoved(ParticleSystem* particleSystem) override;
			void NotifyDecalAdded(Decal* decal) override;
			void NotifyDecalUpdated(Decal* decal) override;
			void NotifyDecalRemoved(Decal* decal) override;

			/**
			 * Updates the render options on the core thread.
			 *
			 * @note	Core thread only.
			 */
			void SyncOptions(const RenderBeastOptions& options);

			/**
			 * Performs rendering over all camera proxies.
			 *
			 * @param[in]	timings			Information about frame time and frame index.
			 * @param[in]	perFrameData	Per-frame data provided by external systems.
			 *
			 * @note	Core thread only.
			 */
			void RenderAllCore(FrameTimings timings, PerFrameData perFrameData);

			/**
			 * Renders all views in the provided view group. Returns true if anything has been draw to any of the views.
			 *
			 * @note	Core thread only.
			 */
			bool RenderViews(RendererViewGroup& viewGroup, const FrameInfo& frameInfo);

			/**
			 * Renders all objects visible by the provided view.
			 *
			 * @note	Core thread only.
			 */
			void RenderView(const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo);

			/**
			 * Renders all overlay callbacks of the provided view. Returns true if anything has been rendered in any of the views.
			 *
			 * @note	Core thread only.
			 */
			bool RenderOverlay(RendererView& view, const FrameInfo& frameInfo);

			/**	Creates data used by the renderer on the core thread. */
			void InitializeCore(const LoadedRendererTextures& rendererTextures);

			/**	Destroys data used by the renderer on the core thread. */
			void DestroyCore();

			/** Updates the global reflection probe cubemap array with changed probe textures. */
			void UpdateReflProbeArray();

			// Core thread only fields
			RenderBeastFeatureSet mFeatureSet = RenderBeastFeatureSet::Desktop;

			// Scene data
			SPtr<RendererScene> mScene;

			SPtr<RenderBeastOptions> mCoreOptions;

			// Helpers to avoid memory allocations
			RendererViewGroup* mMainViewGroup = nullptr;

			// Sim thread only fields
			SPtr<RenderBeastOptions> mOptions;
			bool mOptionsDirty = true;

			// Transient
			Vector<RendererExtension*> mOverlayExtensions;
		};

		/**	Provides easy access to the RenderBeast renderer. */
		SPtr<RenderBeast> GetRenderBeast();

		/** @} */
	} // namespace ct
} // namespace bs
