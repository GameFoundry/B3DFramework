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
		const StringID& getName() const override;

		/** @copydoc Renderer::renderAll */
		void renderAll(PerFrameData perFrameData) override;

		/**	Sets options used for controlling the rendering. */
		void setOptions(const SPtr<RendererOptions>& options) override;

		/**	Returns current set of options used for controlling the rendering. */
		SPtr<RendererOptions> getOptions() const override;

		/** Returns the feature set the renderer is operating on. Core thread only. */
		RenderBeastFeatureSet getFeatureSet() const { return mFeatureSet; }

		/** @copydoc Renderer::initialize */
		void initialize() override;

		/** @copydoc Renderer::destroy */
		void destroy() override;

		/** @copydoc Renderer::captureSceneCubeMap */
		void captureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position,
			const CaptureSettings& settings) override;

		/** @copydoc Renderer::getShaderExtensionPointInfo */
		ShaderExtensionPointInfo getShaderExtensionPointInfo(const String& name) override;

		/** @copydoc Renderer::setGlobalShaderOverride */
		void setGlobalShaderOverride(const String& name, const SPtr<bs::Shader>& shader) override;

	private:
		/** @copydoc Renderer::notifyCameraAdded */
		void notifyCameraAdded(Camera* camera) override;

		/** @copydoc Renderer::notifyCameraUpdated */
		void notifyCameraUpdated(Camera* camera, UINT32 updateFlag) override;

		/** @copydoc Renderer::notifyCameraRemoved */
		void notifyCameraRemoved(Camera* camera) override;

		/** @copydoc Renderer::notifyLightAdded */
		void notifyLightAdded(Light* light) override;

		/** @copydoc Renderer::notifyLightUpdated */
		void notifyLightUpdated(Light* light) override;

		/** @copydoc Renderer::notifyLightRemoved */
		void notifyLightRemoved(Light* light) override;

		/** @copydoc Renderer::notifyRenderableAdded */
		void notifyRenderableAdded(Renderable* renderable) override;

		/** @copydoc Renderer::notifyRenderableUpdated */
		void notifyRenderableUpdated(Renderable* renderable) override;

		/** @copydoc Renderer::notifyRenderableRemoved */
		void notifyRenderableRemoved(Renderable* renderable) override;

		/** @copydoc Renderer::notifyReflectionProbeAdded */
		void notifyReflectionProbeAdded(ReflectionProbe* probe) override;

		/** @copydoc Renderer::notifyReflectionProbeUpdated */
		void notifyReflectionProbeUpdated(ReflectionProbe* probe, bool texture) override;

		/** @copydoc Renderer::notifyReflectionProbeRemoved */
		void notifyReflectionProbeRemoved(ReflectionProbe* probe) override;

		/** @copydoc Renderer::notifyLightProbeVolumeAdded */
		void notifyLightProbeVolumeAdded(LightProbeVolume* volume) override;

		/** @copydoc Renderer::notifyLightProbeVolumeUpdated */
		void notifyLightProbeVolumeUpdated(LightProbeVolume* volume) override;

		/** @copydoc Renderer::notifyLightProbeVolumeRemoved */
		void notifyLightProbeVolumeRemoved(LightProbeVolume* volume) override;

		/** @copydoc Renderer::notifySkyboxAdded */
		void notifySkyboxAdded(Skybox* skybox) override;

		/** @copydoc Renderer::notifySkyboxRemoved */
		void notifySkyboxRemoved(Skybox* skybox) override;

		/** @copydoc Renderer::notifyParticleSystemAdded */
		void notifyParticleSystemAdded(ParticleSystem* particleSystem) override;

		/** @copydoc Renderer::notifyParticleSystemUpdated */
		void notifyParticleSystemUpdated(ParticleSystem* particleSystem, bool tfrmOnly) override;

		/** @copydoc Renderer::notifyParticleSystemRemoved */
		void notifyParticleSystemRemoved(ParticleSystem* particleSystem) override;

		/** @copydoc Renderer::notifyDecalAdded */
		void notifyDecalAdded(Decal* decal) override;

		/** @copydoc Renderer::notifyDecalUpdated */
		void notifyDecalUpdated(Decal* decal) override;

		/** @copydoc Renderer::notifyDecalRemoved */
		void notifyDecalRemoved(Decal* decal) override;

		/**
		 * Updates the render options on the core thread.
		 *
		 * @note	Core thread only.
		 */
		void syncOptions(const RenderBeastOptions& options);

		/**
		 * Performs rendering over all camera proxies.
		 *
		 * @param[in]	timings			Information about frame time and frame index.
		 * @param[in]	perFrameData	Per-frame data provided by external systems.
		 *
		 * @note	Core thread only.
		 */
		void renderAllCore(FrameTimings timings, PerFrameData perFrameData);

		/**
		 * Renders all views in the provided view group. Returns true if anything has been draw to any of the views.
		 *
		 * @note	Core thread only.
		 */
		bool renderViews(RendererViewGroup& viewGroup, const FrameInfo& frameInfo);

		/**
		 * Renders all objects visible by the provided view.
		 *			
		 * @note	Core thread only.
		 */
		void renderView(const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo);

		/**
		 * Renders all overlay callbacks of the provided view. Returns true if anything has been rendered in any of the views.
		 * 					
		 * @note	Core thread only.
		 */
		bool renderOverlay(RendererView& view, const FrameInfo& frameInfo);

		/**	Creates data used by the renderer on the core thread. */
		void initializeCore(const LoadedRendererTextures& rendererTextures);

		/**	Destroys data used by the renderer on the core thread. */
		void destroyCore();

		/** Updates the global reflection probe cubemap array with changed probe textures. */
		void updateReflProbeArray();

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
