//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRenderer.h"
#include "BsBounds.h"
#include "BsRenderableElement.h"
#include "BsSamplerOverrides.h"
#include "BsRendererMaterial.h"
#include "BsLightRendering.h"
#include "BsPostProcessing.h"

namespace BansheeEngine
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	class BeastRenderableElement;

	/** Semantics that may be used for signaling the renderer for what is a certain shader parameter used for. */
	static StringID RPS_GBufferA = "GBufferA";
	static StringID RPS_GBufferB = "GBufferB";
	static StringID RPS_GBufferDepth = "GBufferDepth";

	/** Basic shader that is used when no other is available. */
	class DefaultMaterial : public RendererMaterial<DefaultMaterial> { RMAT_DEF("Default.bsl"); };

	/**	Data used by the renderer when rendering renderable handlers. */
	struct RenderableData
	{
		RenderableCore* renderable;
		Vector<BeastRenderableElement> elements;
		RenderableHandler* controller;
	};

	/**	Data bound to the shader when rendering a specific renderable. */
	struct RenderableShaderData
	{
		Matrix4 worldTransform;
		Matrix4 invWorldTransform;
		Matrix4 worldNoScaleTransform;
		Matrix4 invWorldNoScaleTransform;
		float worldDeterminantSign;
	};

	/**	Data bound to the shader when rendering a with a specific camera. */
	struct CameraShaderData
	{
		Vector3 viewDir;
		Vector3 viewOrigin;
		Matrix4 view;
		Matrix4 proj;
		Matrix4 viewProj;
		Matrix4 invProj;
		Matrix4 invViewProj;
		Matrix4 screenToWorld;
		Vector2 deviceZToWorldZ;
		Vector4 clipToUVScaleOffset;
	};

	/**
	 * @copydoc	RenderableElement
	 *
	 * Contains additional data specific to RenderBeast renderer.
	 */
	class BS_BSRND_EXPORT BeastRenderableElement : public RenderableElement
	{
	public:
		/**
		 * Optional overrides for material sampler states. Used when renderer wants to override certain sampling properties
		 * on a global scale (for example filtering most commonly).
		 */
		MaterialSamplerOverrides* samplerOverrides;

		/**	Identifier of the owner renderable. */
		UINT32 renderableId;
	};

	/**
	 * Default renderer for Banshee. Performs frustum culling, sorting and renders objects in custom ways determine by
	 * renderable handlers.
	 *
	 * @note	Sim thread unless otherwise noted.
	 */
	class BS_BSRND_EXPORT RenderBeast : public Renderer
	{
		/**	Render data for a single render target. */
		struct RenderTargetData
		{
			SPtr<RenderTargetCore> target;
			Vector<const CameraCore*> cameras;
		};

		/**	Data used by the renderer for a camera. */
		struct CameraData
		{
			SPtr<RenderQueue> opaqueQueue;
			SPtr<RenderQueue> transparentQueue;

			SPtr<RenderTargets> target;
			PostProcessInfo postProcessInfo;
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
		virtual const StringID& getName() const override;

		/** @copydoc Renderer::renderAll */
		virtual void renderAll() override;

		/**	Sets options used for controlling the rendering. */
		void setOptions(const SPtr<CoreRendererOptions>& options) override;

		/**	Returns current set of options used for controlling the rendering. */
		SPtr<CoreRendererOptions> getOptions() const override;

		/** @copydoc Renderer::initialize */
		virtual void initialize() override;

		/** @copydoc Renderer::destroy */
		virtual void destroy() override;

	private:
		/** @copydoc Renderer::_notifyCameraAdded */
		void notifyCameraAdded(const CameraCore* camera) override;

		/** @copydocRenderer::_notifyCameraRemoved */
		void notifyCameraRemoved(const CameraCore* camera) override;

		/** @copydoc Renderer::_notifyLightAdded */
		void notifyLightAdded(LightCore* light) override;

		/** @copydoc Renderer::_notifyLightUpdated */
		void notifyLightUpdated(LightCore* light) override;

		/** @copydoc Renderer::_notifyLightRemoved */
		void notifyLightRemoved(LightCore* light) override;

		/** @copydoc Renderer::_notifyRenderableAdded */
		void notifyRenderableAdded(RenderableCore* renderable) override;

		/** @copydoc Renderer::_notifyRenderableUpdated */
		void notifyRenderableUpdated(RenderableCore* renderable) override;

		/** @copydoc Renderer::_notifyRenderableRemoved */
		void notifyRenderableRemoved(RenderableCore* renderable) override;

		/**
		 * Updates the render options on the core thread.
		 *
		 * @note	Core thread only.
		 */
		void syncRenderOptions(const RenderBeastOptions& options);

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
		 * Populates camera render queues by determining visible renderable object.
		 *
		 * @param[in]	camera	The camera to determine visibility for.
		 */
		void determineVisible(const CameraCore& camera);

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
		 * Extracts the necessary values from the projection matrix that allow you to transform device Z value into
		 * world Z value.
		 * 			
		 * @param[in]	projMatrix	Projection matrix that was used to create the device Z value to transform.
		 * @return					Returns two values that can be used to transform device z to world z using this formula:
		 * 							z = (deviceZ + y) * x.
		 */
		static Vector2 getDeviceZTransform(const Matrix4& projMatrix);

		/**
		 * Populates the provided camera shader data object with data from the provided camera. The object can then be used
		 * for populating per-camera parameter buffers.
		 * 			
		 * @note	Core thread.
		 */
		static CameraShaderData getCameraShaderData(const CameraCore& camera);

		/**
		 * Activates the specified pass on the pipeline.
		 *
		 * @param[in]	pass	Pass to activate.
		 * 
		 * @note	Core thread.
		 */
		static void setPass(const SPtr<PassCore>& pass);

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
		UnorderedMap<const CameraCore*, CameraData> mCameraData;
		UnorderedMap<SPtr<MaterialCore>, MaterialSamplerOverrides*> mSamplerOverrides;

		Vector<RenderableData> mRenderables;
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

		// Sim thread only fields
		StaticRenderableHandler* mStaticHandler;
		SPtr<RenderBeastOptions> mOptions;
		bool mOptionsDirty;
	};

	/** @} */
}