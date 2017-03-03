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

namespace bs 
{ 
	struct RendererAnimationData;

	namespace ct
	{
	class LightGrid;

	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Semantics that may be used for signaling the renderer for what is a certain shader parameter used for. */
	static StringID RPS_GBufferA = "GBufferA";
	static StringID RPS_GBufferB = "GBufferB";
	static StringID RPS_GBufferC = "GBufferC";
	static StringID RPS_GBufferDepth = "GBufferDepth";
	static StringID RPS_BoneMatrices = "BoneMatrices";

	/**
	 * Default renderer for Banshee. Performs frustum culling, sorting and renders all scene objects while applying
	 * lighting, shadowing, special effects and post-processing.
	 *
	 * @note	Sim thread unless otherwise noted.
	 */
	class RenderBeast : public Renderer
	{
		/**	Renderer information specific to a single render target. */
		struct RendererRenderTarget
		{
			SPtr<RenderTarget> target;
			Vector<const Camera*> cameras;
		};

		/** Renderer information for a single material. */
		struct RendererMaterial
		{
			Vector<SPtr<GpuParamsSet>> params;
			UINT32 matVersion;
		};

		/** Contains information global to an entire frame. */
		struct FrameInfo
		{
			FrameInfo(float timeDelta, const RendererAnimationData& animData)
				:timeDelta(timeDelta), animData(animData)
			{ }

			float timeDelta;
			const RendererAnimationData& animData;
		};

		/** Information about an active reflection probe. */
		struct ReflProbeInfo
		{
			ReflectionProbe* probe;
			UINT32 arrayIdx;
			SPtr<Texture> texture;
			bool customTexture : 1;
			bool textureDirty : 1;
			bool arrayDirty : 1;
			bool errorFlagged : 1;
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

	private:
		/** @copydoc Renderer::notifyCameraAdded */
		void notifyCameraAdded(const Camera* camera) override;

		/** @copydoc Renderer::notifyCameraUpdated */
		void notifyCameraUpdated(const Camera* camera, UINT32 updateFlag) override;

		/** @copydocRenderer::notifyCameraRemoved */
		void notifyCameraRemoved(const Camera* camera) override;

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

		/** 
		 * Updates (or adds) renderer specific data for the specified camera. Should be called whenever camera properties
		 * change. 
		 *
		 * @param[in]	camera		Camera whose data to update.
		 * @param[in]	forceRemove	If true, the camera data will be removed instead of updated.
		 * @return					Renderer camera object that represents the camera. Null if camera was removed.
		 */
		RendererCamera* updateCameraData(const Camera* camera, bool forceRemove = false);

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
		 * Renders all provided views.
		 * 
		 * @note	Core thread only. 
		 */
		void renderViews(RendererCamera** views, UINT32 numViews, const FrameInfo& frameInfo);

		/**
		 * Renders all objects visible by the provided view.
		 *			
		 * @note	Core thread only.
		 */
		void renderView(RendererCamera* viewInfo, float frameDelta);

		/**
		 * Renders all overlay callbacks of the provided view.
		 * 					
		 * @note	Core thread only.
		 */
		void renderOverlay(RendererCamera* viewInfo);

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

		/** 
		 * Captures the scene at the specified location into a cubemap. 
		 * 
		 * @param[in]	cubemap		Cubemap to store the results in.
		 * @param[in]	position	Position to capture the scene at.
		 * @param[in]	hdr			If true scene will be captured in a format that supports high dynamic range.
		 * @param[in]	frameInfo	Global information about the the frame currently being rendered.
		 */
		void captureSceneCubeMap(const SPtr<Texture>& cubemap, const Vector3& position, bool hdr, const FrameInfo& frameInfo);

		/**	Creates data used by the renderer on the core thread. */
		void initializeCore();

		/**	Destroys data used by the renderer on the core thread. */
		void destroyCore();

		/** Updates reflection probes, rendering ones that are dirty and updating the global probe cubemap array. */
		void updateReflectionProbes(const FrameInfo& frameInfo);

		/**
		 * Checks all sampler overrides in case material sampler states changed, and updates them.
		 *
		 * @param[in]	force	If true, all sampler overrides will be updated, regardless of a change in the material
		 *						was detected or not.
		 */
		void refreshSamplerOverrides(bool force = false);

		// Core thread only fields
		Vector<RendererRenderTarget> mRenderTargets;
		UnorderedMap<const Camera*, RendererCamera*> mCameras;
		UnorderedMap<SamplerOverrideKey, MaterialSamplerOverrides*> mSamplerOverrides;

		Vector<RendererObject*> mRenderables;
		Vector<CullInfo> mRenderableCullInfos;
		Vector<bool> mRenderableVisibility; // Transient

		Vector<RendererLight> mDirectionalLights;
		Vector<RendererLight> mRadialLights;
		Vector<RendererLight> mSpotLights;
		Vector<Sphere> mPointLightWorldBounds;
		Vector<Sphere> mSpotLightWorldBounds;

		Vector<ReflProbeInfo> mReflProbes;
		Vector<Sphere> mReflProbeWorldBounds;
		Vector<bool> mCubemapArrayUsedSlots;
		SPtr<Texture> mCubemapArrayTex;

		SPtr<RenderBeastOptions> mCoreOptions;

		DefaultMaterial* mDefaultMaterial;
		ITiledDeferredLightingMat* mTiledDeferredLightingMats[4];
		FlatFramebufferToTextureMat* mFlatFramebufferToTextureMat;
		SkyboxMat<false>* mSkyboxMat;
		SkyboxMat<true>* mSkyboxSolidColorMat;

		GPULightData* mGPULightData;
		LightGrid* mLightGrid;

		ObjectRenderer* mObjectRenderer;

		// Sim thread only fields
		SPtr<RenderBeastOptions> mOptions;
		bool mOptionsDirty;

		// Helpers to avoid memory allocations
		Vector<LightData> mLightDataTemp;
		Vector<bool> mLightVisibilityTemp;
	};

	/** @} */
}}