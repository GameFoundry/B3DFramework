//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsPostProcessing.h"
#include "BsObjectRendering.h"
#include "BsRenderQueue.h"
#include "BsRendererObject.h"
#include "BsBounds.h"

namespace BansheeEngine
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

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

		/** Populates camera render queues by determining visible renderable objects. */
		void determineVisible(Vector<RendererObject>& renderables, const Vector<Bounds>& renderableBounds);

		/** 
		 * Returns a structure containing information about post-processing effects. This structure will be modified and
		 * maintained by the post-processing system.
		 */
		PostProcessInfo& getPPInfo() { return mPostProcessInfo; }

		/** Returns an object with camera's information, used for populating per-camera parameter buffers. */
		CameraShaderData getShaderData();

	private:
		/**
		 * Extracts the necessary values from the projection matrix that allow you to transform device Z value into
		 * world Z value.
		 * 			
		 * @param[in]	projMatrix	Projection matrix that was used to create the device Z value to transform.
		 * @return					Returns two values that can be used to transform device z to world z using this formula:
		 * 							z = (deviceZ + y) * x.
		 */
		Vector2 getDeviceZTransform(const Matrix4& projMatrix);

		const CameraCore* mCamera;
		SPtr<RenderQueue> mOpaqueQueue;
		SPtr<RenderQueue> mTransparentQueue;

		SPtr<RenderTargets> mRenderTargets;
		PostProcessInfo mPostProcessInfo;
		bool mUsingRenderTargets;
	};

	/** @} */
}