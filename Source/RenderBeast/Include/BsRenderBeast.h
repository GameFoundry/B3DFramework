//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRenderer.h"
#include "BsRendererMaterial.h"
#include "BsObjectRendering.h"
#include "BsPostProcessing.h"
#include "BsRendererView.h"
#include "BsRendererObject.h"
#include "BsRendererScene.h"

namespace bs 
{ 
	struct RendererAnimationData;

	namespace ct
	{
	class LightGrid;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Information about current time and frame index. */
	struct FrameTimings
	{
		float time;
		float timeDelta;
		UINT64 frameIdx;
	};

	/** Contains information global to an entire frame. */
	struct FrameInfo
	{
		FrameInfo(const FrameTimings& timings, const RendererAnimationData* animData = nullptr)
			:timeDelta(timings.timeDelta), frameIdx(timings.frameIdx), animData(animData)
		{ }

		float timeDelta;
		UINT64 frameIdx;
		const RendererAnimationData* animData;
	};

	/**
	 * Default renderer for Banshee. Performs frustum culling, sorting and renders all scene objects while applying
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
		~RenderBeast() { }

		/** @copydoc Renderer::getName */
		const StringID& getName() const override;

		/** @copydoc Renderer::renderAll */
		void renderAll() override;

		/**	Sets options used for controlling the rendering. */
		void setOptions(const SPtr<RendererOptions>& options) override;

		/**	Returns current set of options used for controlling the rendering. */
		SPtr<RendererOptions> getOptions() const override;

		/** @copydoc Renderer::initialize */
		void initialize() override;

		/** @copydoc Renderer::destroy */
		void destroy() override;

		/** @copydoc Renderer::captureSceneCubeMap */
		void captureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position, bool hdr) override;

	private:
		/** @copydoc Renderer::notifyCameraAdded */
		void notifyCameraAdded(Camera* camera) override;

		/** @copydoc Renderer::notifyCameraUpdated */
		void notifyCameraUpdated(Camera* camera, UINT32 updateFlag) override;

		/** @copydocRenderer::notifyCameraRemoved */
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

		/**
		 * Updates the render options on the core thread.
		 *
		 * @note	Core thread only.
		 */
		void syncOptions(const RenderBeastOptions& options);

		/**
		 * Performs rendering over all camera proxies.
		 *
		 * @param[in]	timings		Information about frame time and frame index.
		 *
		 * @note	Core thread only.
		 */
		void renderAllCore(FrameTimings timings);

		/**
		 * Renders all views in the provided view group.
		 * 
		 * @note	Core thread only. 
		 */
		void renderViews(RendererViewGroup& viewGroup, const FrameInfo& frameInfo);

		/**
		 * Renders all objects visible by the provided view.
		 *			
		 * @note	Core thread only.
		 */
		void renderView(const RendererViewGroup& viewGroup, RendererView& view, const FrameInfo& frameInfo);

		/**
		 * Renders all overlay callbacks of the provided view.
		 * 					
		 * @note	Core thread only.
		 */
		void renderOverlay(RendererView& view);

		/**	Creates data used by the renderer on the core thread. */
		void initializeCore();

		/**	Destroys data used by the renderer on the core thread. */
		void destroyCore();

		/** Updates the global reflection probe cubemap array with changed probe textures. */
		void updateReflProbeArray();

		// Core thread only fields

		// Scene data
		SPtr<RendererScene> mScene;

		//// Base pass
		ObjectRenderer* mObjectRenderer = nullptr;

		SPtr<RenderBeastOptions> mCoreOptions;

		// Helpers to avoid memory allocations
		RendererViewGroup* mMainViewGroup = nullptr;

		// Sim thread only fields
		SPtr<RenderBeastOptions> mOptions;
		bool mOptionsDirty = true;
	};

	/** @} */
}}