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
			:timings(timings), perFrameData(perFrameData)
		{ }

		FrameTimings timings;
		PerFrameData perFrameData;
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
			Vector<SPtr<GpuParamsSet>> params;
			UINT32 matVersion;
		};

	public:
		RenderBeast();
		~RenderBeast() = default;

		/** @copydoc Renderer::getName */
		const StringID& GetName() const override;

		/** @copydoc Renderer::renderAll */
		void RenderAll(PerFrameData perFrameData) override;

		/**	Sets options used for controlling the rendering. */
		void SetOptions(const SPtr<RendererOptions>& options) ;

		/**	Returns current set of options used for controlling the rendering. */
		SPtr<RendererOptions> GetOptions() const ;

		/** Returns the feature set the renderer is operating on. Core thread only. */
		RenderBeastFeatureSet GetFeatureSet() const { return mFeatureSet; }

		/** @copydoc Renderer::initialize */
		void Initialize() override;

		/** @copydoc Renderer::destroy */
		void Destroy() override;

		/** @copydoc Renderer::captureSceneCubeMap */
		void CaptureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position,
			const CaptureSettings& settings) ;

		/** @copydoc Renderer::getShaderExtensionPointInfo */
		ShaderExtensionPointInfo GetShaderExtensionPointInfo(const String& name) override;

		/** @copydoc Renderer::setGlobalShaderOverride */
		void SetGlobalShaderOverride(const String& name, const SPtr<bs::Shader>& shader) ;

	private:
		/** @copydoc Renderer::notifyCameraAdded */
		void NotifyCameraAdded(Camera* camera) override;

		/** @copydoc Renderer::notifyCameraUpdated */
		void NotifyCameraUpdated(Camera* camera, UINT32 updateFlag) override;

		/** @copydoc Renderer::notifyCameraRemoved */
		void NotifyCameraRemoved(Camera* camera) override;

		/** @copydoc Renderer::notifyLightAdded */
		void NotifyLightAdded(Light* light) override;

		/** @copydoc Renderer::notifyLightUpdated */
		void NotifyLightUpdated(Light* light) override;

		/** @copydoc Renderer::notifyLightRemoved */
		void NotifyLightRemoved(Light* light) override;

		/** @copydoc Renderer::notifyRenderableAdded */
		void NotifyRenderableAdded(Renderable* renderable) override;

		/** @copydoc Renderer::notifyRenderableUpdated */
		void NotifyRenderableUpdated(Renderable* renderable) override;

		/** @copydoc Renderer::notifyRenderableRemoved */
		void NotifyRenderableRemoved(Renderable* renderable) override;

		/** @copydoc Renderer::notifyReflectionProbeAdded */
		void NotifyReflectionProbeAdded(ReflectionProbe* probe) override;

		/** @copydoc Renderer::notifyReflectionProbeUpdated */
		void NotifyReflectionProbeUpdated(ReflectionProbe* probe, bool texture) override;

		/** @copydoc Renderer::notifyReflectionProbeRemoved */
		void NotifyReflectionProbeRemoved(ReflectionProbe* probe) override;

		/** @copydoc Renderer::notifyLightProbeVolumeAdded */
		void NotifyLightProbeVolumeAdded(LightProbeVolume* volume) override;

		/** @copydoc Renderer::notifyLightProbeVolumeUpdated */
		void NotifyLightProbeVolumeUpdated(LightProbeVolume* volume) override;

		/** @copydoc Renderer::notifyLightProbeVolumeRemoved */
		void NotifyLightProbeVolumeRemoved(LightProbeVolume* volume) override;

		/** @copydoc Renderer::notifySkyboxAdded */
		void NotifySkyboxAdded(Skybox* skybox) override;

		/** @copydoc Renderer::notifySkyboxRemoved */
		void NotifySkyboxRemoved(Skybox* skybox) override;

		/** @copydoc Renderer::notifyParticleSystemAdded */
		void NotifyParticleSystemAdded(ParticleSystem* particleSystem) override;

		/** @copydoc Renderer::notifyParticleSystemUpdated */
		void NotifyParticleSystemUpdated(ParticleSystem* particleSystem, bool tfrmOnly) override;

		/** @copydoc Renderer::notifyParticleSystemRemoved */
		void NotifyParticleSystemRemoved(ParticleSystem* particleSystem) override;

		/** @copydoc Renderer::notifyDecalAdded */
		void NotifyDecalAdded(Decal* decal) override;

		/** @copydoc Renderer::notifyDecalUpdated */
		void NotifyDecalUpdated(Decal* decal) override;

		/** @copydoc Renderer::notifyDecalRemoved */
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
	SPtr<RenderBeast> gRenderBeast();

	/** @} */
}}
