//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRenderer.h"
#include "BsRendererMaterial.h"
#include "BsLightRendering.h"
#include "BsImageBasedLighting.h"
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

	/** Contains information global to an entire frame. */
	struct FrameInfo
	{
		FrameInfo(float timeDelta, const RendererAnimationData& animData)
			:timeDelta(timeDelta), animData(animData)
		{ }

		float timeDelta;
		const RendererAnimationData& animData;
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

		/** @copydoc Renderer::createPostProcessSettings */
		SPtr<PostProcessSettings> createPostProcessSettings() const override;

		/** 
		 * Captures the scene at the specified location into a cubemap. 
		 * 
		 * @param[in]	cubemap		Cubemap to store the results in.
		 * @param[in]	position	Position to capture the scene at.
		 * @param[in]	hdr			If true scene will be captured in a format that supports high dynamic range.
		 * @param[in]	frameInfo	Global information about the the frame currently being rendered.
		 */
		void captureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position, bool hdr, const FrameInfo& frameInfo);

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
		void notifyReflectionProbeUpdated(ReflectionProbe* probe) override;

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

		/** @copydoc Renderer::notifySkyboxTextureChanged */
		void notifySkyboxTextureChanged(Skybox* skybox) override;

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
		 * @param[in]	time	Current frame time in milliseconds.
		 * @param[in]	delta	Time elapsed since the last frame.
		 *
		 * @note	Core thread only.
		 */
		void renderAllCore(float time, float delta);

		/**
		 * Renders all views in the provided view group.
		 * 
		 * @note	Core thread only. 
		 */
		void renderViews(const RendererViewGroup& viewGroup, const FrameInfo& frameInfo);

		/**
		 * Renders all objects visible by the provided view.
		 *			
		 * @note	Core thread only.
		 */
		void renderView(const RendererViewGroup& viewGroup, RendererView* viewInfo, float frameDelta);

		/**
		 * Renders all overlay callbacks of the provided view.
		 * 					
		 * @note	Core thread only.
		 */
		void renderOverlay(RendererView* viewInfo);

		/** 
		 * Renders a single element of a renderable object. 
		 *
		 * @param[in]	element		Element to render.
		 * @param[in]	passIdx		Index of the material pass to render the element with.
		 * @param[in]	bindPass	If true the material pass will be bound for rendering, if false it is assumed it is
		 *							already bound.
		 * @param[in]	viewProj	View projection matrix of the camera the element is being rendered with.
		 */
		void renderElement(const BeastRenderableElement& element, UINT32 passIdx, bool bindPass, const Matrix4& viewProj);

		/**	Creates data used by the renderer on the core thread. */
		void initializeCore();

		/**	Destroys data used by the renderer on the core thread. */
		void destroyCore();

		/** Updates light probes, rendering & filtering ones that are dirty and updating the global probe cubemap array. */
		void renderReflectionProbes(const FrameInfo& frameInfo);

		/** Renders the skybox filtered reflections and irradiance cubemap, if they require updating. */
		void updateSkybox();

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