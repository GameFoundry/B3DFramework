//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsPostProcessing.h"
#include "BsObjectRendering.h"
#include "BsRenderQueue.h"
#include "BsRendererObject.h"
#include "BsBounds.h"
#include "BsConvexVolume.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	BS_PARAM_BLOCK_BEGIN(PerCameraParamDef)
		BS_PARAM_BLOCK_ENTRY(Vector3, gViewDir)
		BS_PARAM_BLOCK_ENTRY(Vector3, gViewOrigin)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatViewProj)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatView)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatProj)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvProj)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatInvViewProj)
		BS_PARAM_BLOCK_ENTRY(Matrix4, gMatScreenToWorld)
		BS_PARAM_BLOCK_ENTRY(Vector2, gDeviceZToWorldZ)
		BS_PARAM_BLOCK_ENTRY(Vector4, gClipToUVScaleOffset)
	BS_PARAM_BLOCK_END

	extern PerCameraParamDef gPerCameraParamDef;

	/** Shader that renders a skybox using a cubemap. */
	class SkyboxMat : public RendererMaterial<SkyboxMat>
	{
		RMAT_DEF("Skybox.bsl");

	public:
		SkyboxMat();

		/** Binds the material for rendering and sets up any global parameters. */
		void bind(const SPtr<GpuParamBlockBuffer>& perCamera);

		/** Updates the skybox texture used by the material. */
		void setParams(const SPtr<Texture>& texture);
	private:
		GpuParamTexture mSkyTextureParam;
	};

	/** Set of properties describing the output render target used by a renderer view. */
	struct RENDERER_VIEW_TARGET_DESC
	{
		SPtr<RenderTarget> target;

		Rect2I viewRect;
		Rect2 nrmViewRect;
		UINT32 targetWidth;
		UINT32 targetHeight;
		UINT32 numSamples;

		UINT32 clearFlags;
		Color clearColor;
		float clearDepthValue;
		UINT16 clearStencilValue;
	};

	/** Set of properties used describing a specific view that the renderer can render. */
	struct RENDERER_VIEW_DESC
	{
		RENDERER_VIEW_TARGET_DESC target;

		Matrix4 viewTransform;
		Matrix4 projTransform;
		Vector3 viewDirection;
		Vector3 viewOrigin;
		bool flipView;
		float nearPlane;

		bool isOverlay : 1;
		bool isHDR : 1;
		bool triggerCallbacks : 1;
		bool runPostProcessing : 1;

		UINT64 visibleLayers;
		ConvexVolume cullFrustum;

		StateReduction stateReduction;

		const Camera* sceneCamera;
		SPtr<Texture> skyboxTexture;
	};

	/** Information whether certain scene objects are visible in a view, per object type. */
	struct VisibilityInfo
	{
		Vector<bool> renderables;
	};

	/** Information used for culling an object against a view. */
	struct CullInfo
	{
		CullInfo(const Bounds& bounds, UINT64 layer = -1)
			:bounds(bounds), layer(layer)
		{ }

		Bounds bounds;
		UINT64 layer;
	};

	/** Contains information about a Camera, used by the Renderer. */
	class RendererCamera
	{
	public:
		RendererCamera();
		RendererCamera(const RENDERER_VIEW_DESC& desc);

		/** Sets state reduction mode that determines how do render queues group & sort renderables. */
		void setStateReductionMode(StateReduction reductionMode);

		/** Updates the internal camera post-processing data. */
		void setPostProcessSettings(const SPtr<PostProcessSettings>& ppSettings);

		/** Updates the internal information with a new view transform. */
		void setTransform(const Vector3& origin, const Vector3& direction, const Matrix4& view,
						  const Matrix4& proj, const ConvexVolume& worldFrustum);

		/** Updates all internal information with new view information. */
		void setView(const RENDERER_VIEW_DESC& desc);

		/** Returns the world position of the view. */
		Vector3 getViewOrigin() const { return mViewDesc.viewOrigin; }

		/** Returns a matrix that contains combined projection and view transforms. */
		Matrix4 getViewProjMatrix() const { return mViewDesc.projTransform * mViewDesc.viewTransform; }

		/** Returns the distance to the near clipping plane. */
		float getNearPlane() const { return mViewDesc.nearPlane; }

		/** Returns true if the view requires high dynamic range rendering. */
		bool isHDR() const { return mViewDesc.isHDR; }

		/** Returns true if this view only renders overlay, and not scene objects. */
		bool isOverlay() const { return mViewDesc.isOverlay; }

		/** Returns the texture to use for the skybox (if any). */
		SPtr<Texture> getSkybox() const { return mViewDesc.skyboxTexture; }

		/** Returns the final render target the rendered contents should be output to. */
		SPtr<RenderTarget> getFinalTarget() const { return mViewDesc.target.target; }

		/** Returns normalized coordinates of the viewport area this view renders to. */
		Rect2 getViewportRect() const { return mViewDesc.target.nrmViewRect; }

		/** Returns true if the resulting render target should be flipped vertically. */
		bool getFlipView() const { return mViewDesc.flipView; }

		/** Returns the scene camera this object is based of. This can be null for manually constructed renderer cameras. */
		const Camera* getSceneCamera() const { return mViewDesc.sceneCamera; }

		/** Returns true if external render callbacks should trigger for this view. */
		bool checkTriggerCallbacks() const { return mViewDesc.triggerCallbacks; }

		/** Returns true if post-processing effects should be triggered for this view. */
		bool checkRunPostProcessing() const { return mViewDesc.runPostProcessing; }

		/** 
		 * Prepares render targets for rendering. When done call endRendering().
		 *
		 * @param[in]	useGBuffer			Set to true if you will be rendering to internal render targets containing the
		 *									GBuffer (retrieved via getRenderTargets()).
		 */
		void beginRendering(bool useGBuffer);

		/** Ends rendering and frees any acquired resources. */
		void endRendering();

		/** Returns the view's renderTargets. Only valid if called in-between beginRendering() and endRendering() calls. */
		SPtr<RenderTargets> getRenderTargets() const { return mRenderTargets; }

		/** 
		 * Returns a render queue containing all opaque objects. Make sure to call determineVisible() beforehand if view 
		 * or object transforms changed since the last time it was called.
		 */
		const SPtr<RenderQueue>& getOpaqueQueue() const { return mOpaqueQueue; }
		
		/** 
		 * Returns a render queue containing all transparent objects. Make sure to call determineVisible() beforehand if 
		 * view or object transforms changed since the last time it was called.
		 */
		const SPtr<RenderQueue>& getTransparentQueue() const { return mTransparentQueue; }

		/**
		 * Populates view render queues by determining visible renderable objects. 
		 *
		 * @param[in]	renderables			A set of renderable objects to iterate over and determine visibility for.
		 * @param[in]	cullInfos			A set of world bounds & other information relevant for culling the provided
		 *									renderable objects. Must be the same size as the @p renderables array.
		 * @param[out]	visibility			Output parameter that will have the true bit set for any visible renderable
		 *									object. If the bit for an object is already set to true, the method will never
		 *									change it to false which allows the same bitfield to be provided to multiple
		 *									renderer views. Must be the same size as the @p renderables array.
		 *									
		 *									As a side-effect, per-view visibility data is also calculated and can be
		 *									retrieved by calling getVisibilityMask().
		 */
		void determineVisible(const Vector<RendererObject*>& renderables, const Vector<CullInfo>& cullInfos,
			Vector<bool>* visibility = nullptr);

		/**
		 * Culls the provided set of bounds against the current frustum and outputs a set of visibility flags determining
		 * which entry is or ins't visible by this view. Both inputs must be arrays of the same size.
		 */
		void calculateVisibility(const Vector<CullInfo>& cullInfos, Vector<bool>& visibility) const;

		/** Returns the visibility mask calculated with the last call to determineVisible(). */
		const VisibilityInfo& getVisibilityMasks() const { return mVisibility; }

		/** 
		 * Returns a structure containing information about post-processing effects. This structure will be modified and
		 * maintained by the post-processing system.
		 */
		PostProcessInfo& getPPInfo() { return mPostProcessInfo; }

		/** Updates the GPU buffer containing per-view information, with the latest internal data. */
		void updatePerViewBuffer();

		/** Returns a buffer that stores per-view parameters. */
		SPtr<GpuParamBlockBuffer> getPerViewBuffer() const { return mParamBuffer; }

	private:
		/**
		 * Extracts the necessary values from the projection matrix that allow you to transform device Z value into
		 * world Z value.
		 * 
		 * @param[in]	projMatrix	Projection matrix that was used to create the device Z value to transform.
		 * @return					Returns two values that can be used to transform device z to world z using this formula:
		 * 							z = (deviceZ + y) * x.
		 */
		Vector2 getDeviceZTransform(const Matrix4& projMatrix) const;

		RENDERER_VIEW_DESC mViewDesc;

		SPtr<RenderQueue> mOpaqueQueue;
		SPtr<RenderQueue> mTransparentQueue;

		SPtr<RenderTargets> mRenderTargets;
		PostProcessInfo mPostProcessInfo;
		bool mUsingGBuffer;

		SPtr<GpuParamBlockBuffer> mParamBuffer;
		VisibilityInfo mVisibility;
	};

	/** @} */
}}