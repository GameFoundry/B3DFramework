//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsPostProcessing.h"
#include "BsObjectRendering.h"
#include "BsRenderQueue.h"
#include "BsRendererObject.h"
#include "BsBounds.h"

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
		void bind(const SPtr<GpuParamBlockBufferCore>& perCamera);

		/** Updates the skybox texture used by the material. */
		void setParams(const SPtr<TextureCore>& texture);
	private:
		GpuParamTexture mSkyTextureParam;
	};

	/** Contains information about a Camera, used by the Renderer. */
	class RendererCamera
	{
	public:
		RendererCamera();
		RendererCamera(const CameraCore* camera, StateReduction reductionMode);

		/** Updates the internal camera data, usually called after source camera changes. */
		void update(StateReduction reductionMode);

		/** Updates the internal camera post-processing data. */
		void updatePP();

		/** 
		 * Prepares camera render targets for rendering. When done call endRendering().
		 *
		 * @param[in]	useRenderTargets	If using the internal render targets containing the GBuffer (retrieved via
		 *									getRenderTargets()) while rendering you must set this to true.
		 */
		void beginRendering(bool useRenderTargets);

		/** Ends rendering and frees any acquired resources. */
		void endRendering();

		/** Returns the camera's renderTargets. Only valid if called in-between beginRendering() and endRendering() calls. */
		SPtr<RenderTargets> getRenderTargets() const { return mRenderTargets; }

		/** 
		 * Returns a render queue containing all opaque objects. Make sure to call determineVisible() beforehand if camera 
		 * or object transforms changed since the last time it was called.
		 */
		const SPtr<RenderQueue>& getOpaqueQueue() const { return mOpaqueQueue; }
		
		/** 
		 * Returns a render queue containing all transparent objects. Make sure to call determineVisible() beforehand if 
		 * camera or object transforms changed since the last time it was called.
		 */
		const SPtr<RenderQueue>& getTransparentQueue() const { return mTransparentQueue; }

		/**
		 * Populates camera render queues by determining visible renderable objects. 
		 *
		 * @param[in]	renderables			A set of renderable objects to iterate over and determine visibility for.
		 * @param[in]	renderableBounds	A set of world bounds for the provided renderable objects. Must be the same size
		 *									as the @p renderables array.
		 * @param[in]	visibility			Output parameter that will have the true bit set for any visible renderable
		 *									object. If the bit for an object is already set to true, the method will never
		 *									change it to false which allows the same bitfield to be provided to multiple
		 *									renderer cameras. Must be the same size as the @p renderables array.
		 *									
		 *									As a side-effect, per-camera visibility data is also calculated and can be
		 *									retrieved by calling getVisibilityMask().
		 */
		void determineVisible(const Vector<RendererObject*>& renderables, const Vector<Bounds>& renderableBounds, 
			Vector<bool>& visibility);

		/** Returns the visibility mask calculated with the last call to determineVisible(). */
		const Vector<bool> getVisibilityMask() const { return mVisibility; }

		/** 
		 * Returns a structure containing information about post-processing effects. This structure will be modified and
		 * maintained by the post-processing system.
		 */
		PostProcessInfo& getPPInfo() { return mPostProcessInfo; }

		/** Updates the GPU buffer containing per-camera information, with the latest data. */
		void updatePerCameraBuffer();

		/** Returns a buffer that stores per-camera parameters. */
		SPtr<GpuParamBlockBufferCore> getPerCameraBuffer() const { return mParamBuffer; }

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

		const CameraCore* mCamera;
		SPtr<RenderQueue> mOpaqueQueue;
		SPtr<RenderQueue> mTransparentQueue;

		SPtr<RenderTargets> mRenderTargets;
		PostProcessInfo mPostProcessInfo;
		bool mUsingRenderTargets;

		SPtr<GpuParamBlockBufferCore> mParamBuffer;
		Vector<bool> mVisibility;
	};

	/** @} */
}}