//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRenderer.h"
#include "BsBounds.h"
#include "BsSamplerOverrides.h"
#include "BsRendererMaterial.h"
#include "BsLightRendering.h"
#include "BsObjectRendering.h"
#include "BsPostProcessing.h"
#include "BsRendererCamera.h"
#include "BsRendererObject.h"

namespace BansheeEngine
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Semantics that may be used for signaling the renderer for what is a certain shader parameter used for. */
	static StringID RPS_GBufferA = "GBufferA";
	static StringID RPS_GBufferB = "GBufferB";
	static StringID RPS_GBufferDepth = "GBufferDepth";

	/**
	 * Default renderer for Banshee. Performs frustum culling, sorting and renders objects in custom ways determine by
	 * renderable handlers.
	 *
	 * @note	Sim thread unless otherwise noted.
	 */
	class RenderBeast : public Renderer
	{
		/**	Render data for a single render target. */
		struct RenderTargetData
		{
			SPtr<RenderTargetCore> target;
			Vector<const CameraCore*> cameras;
		};

		/**	Data used by the renderer for lights. */
		struct LightData
		{
			LightCore* internal;
		};

	public:
		RenderBeast();
		~RenderBeast() { }

		/** @copydoc Renderer::getName */
		const StringID& getName() const override;

		/** @copydoc Renderer::renderAll */
		void renderAll() override;

		/**	Sets options used for controlling the rendering. */
		void setOptions(const SPtr<CoreRendererOptions>& options) override;

		/**	Returns current set of options used for controlling the rendering. */
		SPtr<CoreRendererOptions> getOptions() const override;

		/** @copydoc Renderer::initialize */
		void initialize() override;

		/** @copydoc Renderer::destroy */
		void destroy() override;

	private:
		/** @copydoc Renderer::notifyCameraAdded */
		void notifyCameraAdded(const CameraCore* camera) override;

		/** @copydoc Renderer::notifyCameraUpdated */
		void notifyCameraUpdated(const CameraCore* camera, UINT32 updateFlag) override;

		/** @copydocRenderer::notifyCameraRemoved */
		void notifyCameraRemoved(const CameraCore* camera) override;

		/** @copydoc Renderer::notifyLightAdded */
		void notifyLightAdded(LightCore* light) override;

		/** @copydoc Renderer::notifyLightUpdated */
		void notifyLightUpdated(LightCore* light) override;

		/** @copydoc Renderer::notifyLightRemoved */
		void notifyLightRemoved(LightCore* light) override;

		/** @copydoc Renderer::notifyRenderableAdded */
		void notifyRenderableAdded(RenderableCore* renderable) override;

		/** @copydoc Renderer::notifyRenderableUpdated */
		void notifyRenderableUpdated(RenderableCore* renderable) override;

		/** @copydoc Renderer::notifyRenderableRemoved */
		void notifyRenderableRemoved(RenderableCore* renderable) override;

		/** 
		 * Updates (or adds) renderer specific data for the specified camera. Should be called whenever camera properties
		 * change. 
		 *
		 * @param[in]	camera		Camera whose data to update.
		 * @param[in]	forceRemove	If true, the camera data will be removed instead of updated.
		 */
		void updateCameraData(const CameraCore* camera, bool forceRemove = false);

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
		 * Renders all objects visible by the provided camera.
		 *
		 * @param[in]	rtData	Render target data containing the camera to render.
		 * @param[in]	camIdx	Index of the camera to render.
		 * @param[in]	delta	Time elapsed since the last frame.
		 * 					
		 * @note	Core thread only.
		 */
		void render(RenderTargetData& rtData, UINT32 camIdx, float delta);

		/**
		 * Renders all overlay callbacks attached to the provided camera.
		 *
		 * @param[in]	rtData	Render target data containing the camera to render.
		 * @param[in]	camIdx	Index of the camera to render.
		 * @param[in]	delta	Time elapsed since the last frame.
		 * 					
		 * @note	Core thread only.
		 */
		void renderOverlay(RenderTargetData& rtData, UINT32 camIdx, float delta);

		/**	Creates data used by the renderer on the core thread. */
		void initializeCore();

		/**	Destroys data used by the renderer on the core thread. */
		void destroyCore();

		/**
		 * Checks all sampler overrides in case material sampler states changed, and updates them.
		 *
		 * @param[in]	force	If true, all sampler overrides will be updated, regardless of a change in the material
		 *						was detected or not.
		 */
		void refreshSamplerOverrides(bool force = false);

		/**
		 * Sets parameters (textures, samplers, buffers) for the currently active pass.
		 *
		 * @param[in]	passParams			Structure containing parameters for all stages of the pass.
		 * @param[in]	samplerOverrides	Optional samplers to use instead of the those in the pass parameters. Number of
		 *									samplers must match number in pass parameters.
		 *
		 * @note	Core thread.
		 */
		static void setPassParams(const SPtr<PassParametersCore>& passParams, const PassSamplerOverrides* samplerOverrides);

		// Core thread only fields
		Vector<RenderTargetData> mRenderTargets;
		UnorderedMap<const CameraCore*, RendererCamera> mCameras;
		UnorderedMap<SPtr<MaterialCore>, MaterialSamplerOverrides*> mSamplerOverrides;

		Vector<RendererObject> mRenderables;
		Vector<RenderableShaderData> mRenderableShaderData;
		Vector<Bounds> mWorldBounds;

		Vector<LightData> mDirectionalLights;
		Vector<LightData> mPointLights;
		Vector<Sphere> mLightWorldBounds;

		SPtr<RenderBeastOptions> mCoreOptions;

		DefaultMaterial* mDefaultMaterial;
		PointLightInMat* mPointLightInMat;
		PointLightOutMat* mPointLightOutMat;
		DirectionalLightMat* mDirLightMat;

		ObjectRenderer* mObjectRenderer;

		// Sim thread only fields
		SPtr<RenderBeastOptions> mOptions;
		bool mOptionsDirty;
	};

	/** @} */
}