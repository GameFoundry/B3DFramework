//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererCamera.h"
#include "BsCamera.h"
#include "BsRenderable.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsRenderTargets.h"

namespace BansheeEngine
{
	RendererCamera::RendererCamera()
		:mCamera(nullptr), mUsingRenderTargets(false)
	{ }

	RendererCamera::RendererCamera(const CameraCore* camera, StateReduction reductionMode)
		:mCamera(camera), mUsingRenderTargets(false)
	{
		update(reductionMode);
	}

	void RendererCamera::update(StateReduction reductionMode)
	{
		mOpaqueQueue = bs_shared_ptr_new<RenderQueue>(reductionMode);

		StateReduction transparentStateReduction = reductionMode;
		if (transparentStateReduction == StateReduction::Material)
			transparentStateReduction = StateReduction::Distance; // Transparent object MUST be sorted by distance

		mTransparentQueue = bs_shared_ptr_new<RenderQueue>(transparentStateReduction);
		updatePP();
	}

	void RendererCamera::updatePP()
	{
		if (mPostProcessInfo.settings == nullptr)
			mPostProcessInfo.settings = bs_shared_ptr_new<StandardPostProcessSettings>();

		SPtr<StandardPostProcessSettings> ppSettings = std::static_pointer_cast<StandardPostProcessSettings>(mCamera->getPostProcessSettings());
		if (ppSettings != nullptr)
			*mPostProcessInfo.settings = *ppSettings;
		else
			*mPostProcessInfo.settings = StandardPostProcessSettings();

		mPostProcessInfo.settingDirty = true;
	}

	void RendererCamera::beginRendering(bool useGBuffer)
	{
		if (useGBuffer)
		{
			SPtr<ViewportCore> viewport = mCamera->getViewport();
			bool useHDR = mCamera->getFlags().isSet(CameraFlag::HDR);
			UINT32 msaaCount = mCamera->getMSAACount();

			// Render scene objects to g-buffer
			bool createGBuffer = mRenderTargets == nullptr ||
				mRenderTargets->getHDR() != useHDR ||
				mRenderTargets->getNumSamples() != msaaCount;

			if (createGBuffer)
				mRenderTargets = RenderTargets::create(viewport, useHDR, msaaCount);

			mRenderTargets->allocate();
			mUsingRenderTargets = true;
		}
	}

	void RendererCamera::endRendering()
	{
		mOpaqueQueue->clear();
		mTransparentQueue->clear();

		if(mUsingRenderTargets)
		{
			mRenderTargets->release();
			mUsingRenderTargets = false;
		}
	}

	void RendererCamera::determineVisible(Vector<RendererObject>& renderables, const Vector<Bounds>& renderableBounds)
	{
		bool isOverlayCamera = mCamera->getFlags().isSet(CameraFlag::Overlay);
		if (isOverlayCamera)
			return;

		UINT64 cameraLayers = mCamera->getLayers();
		ConvexVolume worldFrustum = mCamera->getWorldFrustum();

		// Update per-object param buffers and queue render elements
		for (auto& renderableData : renderables)
		{
			RenderableCore* renderable = renderableData.renderable;
			UINT32 rendererId = renderable->getRendererId();

			if ((renderable->getLayer() & cameraLayers) == 0)
				continue;

			// Do frustum culling
			// Note: This is bound to be a bottleneck at some point. When it is ensure that intersect methods use vector
			// operations, as it is trivial to update them. Also consider spatial partitioning.
			const Sphere& boundingSphere = renderableBounds[rendererId].getSphere();
			if (worldFrustum.intersects(boundingSphere))
			{
				// More precise with the box
				const AABox& boundingBox = renderableBounds[rendererId].getBox();

				if (worldFrustum.intersects(boundingBox))
				{
					float distanceToCamera = (mCamera->getPosition() - boundingBox.getCenter()).length();

					for (auto& renderElem : renderableData.elements)
					{
						bool isTransparent = (renderElem.material->getShader()->getFlags() & (UINT32)ShaderFlags::Transparent) != 0;

						if (isTransparent)
							mTransparentQueue->add(&renderElem, distanceToCamera);
						else
							mOpaqueQueue->add(&renderElem, distanceToCamera);
					}

				}
			}
		}

		mOpaqueQueue->sort();
		mTransparentQueue->sort();
	}

	Vector2 RendererCamera::getDeviceZTransform(const Matrix4& projMatrix)
	{
		// Returns a set of values that will transform depth buffer values (e.g. [0, 1] in DX, [-1, 1] in GL) to a distance
		// in world space. This involes applying the inverse projection transform to the depth value. When you multiply
		// a vector with the projection matrix you get [clipX, clipY, Az + B, C * z], where we don't care about clipX/clipY.
		// A is [2, 2], B is [2, 3] and C is [3, 2] elements of the projection matrix (only ones that matter for our depth 
		// value). The hardware will also automatically divide the z value with w to get the depth, therefore the final 
		// formula is:
		// depth = (Az + B) / (C * z)

		// To get the z coordinate back we simply do the opposite: 
		// z = B / (depth * C - A)

		// However some APIs will also do a transformation on the depth values before storing them to the texture 
		// (e.g. OpenGL will transform from [-1, 1] to [0, 1]). And we need to reverse that as well. Therefore the final 
		// formula is:
		// z = B / ((depth * (maxDepth - minDepth) + minDepth) * C - A)

		// Are we reorganize it because it needs to fit the "(1.0f / (depth + y)) * x" format used in the shader:
		// z = 1.0f / (depth + minDepth/(maxDepth - minDepth) - A/((maxDepth - minDepth) * C)) * B/((maxDepth - minDepth) * C)

		RenderAPICore& rapi = RenderAPICore::instance();
		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();

		float depthRange = rapiInfo.getMaximumDepthInputValue() - rapiInfo.getMinimumDepthInputValue();
		float minDepth = rapiInfo.getMinimumDepthInputValue();

		float a = projMatrix[2][2];
		float b = projMatrix[2][3];
		float c = projMatrix[3][2];

		Vector2 output;
		output.x = b / (depthRange * c);
		output.y = minDepth / depthRange - a / (depthRange * c);

		return output;
	}

	CameraShaderData RendererCamera::getShaderData()
	{
		CameraShaderData data;
		data.proj = mCamera->getProjectionMatrixRS();
		data.view = mCamera->getViewMatrix();
		data.viewProj = data.proj * data.view;
		data.invProj = data.proj.inverse();
		data.invViewProj = data.viewProj.inverse(); // Note: Calculate inverses separately (better precision possibly)

		// Construct a special inverse view-projection matrix that had projection entries that affect z and w eliminated.
		// Used to transform a vector(clip_x, clip_y, view_z, view_w), where clip_x/clip_y are in clip space, and 
		// view_z/view_w in view space, into world space.

		// Only projects z/w coordinates
		Matrix4 projZ = Matrix4::IDENTITY;
		projZ[2][2] = data.proj[2][2];
		projZ[2][3] = data.proj[2][3];
		projZ[3][2] = data.proj[3][2];
		projZ[3][3] = 0.0f;

		data.screenToWorld = data.invViewProj * projZ;
		data.viewDir = mCamera->getForward();
		data.viewOrigin = mCamera->getPosition();
		data.deviceZToWorldZ = getDeviceZTransform(data.proj);

		SPtr<ViewportCore> viewport = mCamera->getViewport();
		SPtr<RenderTargetCore> rt = viewport->getTarget();

		float halfWidth = viewport->getWidth() * 0.5f;
		float halfHeight = viewport->getHeight() * 0.5f;

		float rtWidth = (float)rt->getProperties().getWidth();
		float rtHeight = (float)rt->getProperties().getHeight();

		RenderAPICore& rapi = RenderAPICore::instance();
		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();

		data.clipToUVScaleOffset.x = halfWidth / rtWidth;
		data.clipToUVScaleOffset.y = -halfHeight / rtHeight;
		data.clipToUVScaleOffset.z = viewport->getX() / rtWidth + (halfWidth + rapiInfo.getHorizontalTexelOffset()) / rtWidth;
		data.clipToUVScaleOffset.w = viewport->getY() / rtHeight + (halfHeight + rapiInfo.getVerticalTexelOffset()) / rtHeight;

		if (!rapiInfo.getNDCYAxisDown())
			data.clipToUVScaleOffset.y = -data.clipToUVScaleOffset.y;

		return data;
	}
}