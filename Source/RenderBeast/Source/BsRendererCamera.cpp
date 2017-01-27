//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererCamera.h"
#include "BsCamera.h"
#include "BsRenderable.h"
#include "BsMaterial.h"
#include "BsShader.h"
#include "BsRenderTargets.h"
#include "BsRendererUtility.h"
#include "BsGpuParamsSet.h"

namespace bs { namespace ct
{
	PerCameraParamDef gPerCameraParamDef;

	SkyboxMat::SkyboxMat()
	{
		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSkyTex", mSkyTextureParam);
	}

	void SkyboxMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void SkyboxMat::bind(const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		mParamsSet->setParamBlockBuffer("PerCamera", perCamera, true);

		gRendererUtility().setPass(mMaterial, 0);
	}

	void SkyboxMat::setParams(const SPtr<Texture>& texture)
	{
		mSkyTextureParam.set(texture, TextureSurface(1, 1, 0, 0));
		gRendererUtility().setPassParams(mParamsSet);
	}

	RendererCamera::RendererCamera()
		: mUsingGBuffer(false)
	{
		mParamBuffer = gPerCameraParamDef.createBuffer();
	}

	RendererCamera::RendererCamera(const RENDERER_VIEW_DESC& desc)
		: mViewDesc(desc), mUsingGBuffer(false)
	{
		mParamBuffer = gPerCameraParamDef.createBuffer();

		setStateReductionMode(desc.stateReduction);
	}

	void RendererCamera::setStateReductionMode(StateReduction reductionMode)
	{
		mOpaqueQueue = bs_shared_ptr_new<RenderQueue>(reductionMode);

		StateReduction transparentStateReduction = reductionMode;
		if (transparentStateReduction == StateReduction::Material)
			transparentStateReduction = StateReduction::Distance; // Transparent object MUST be sorted by distance

		mTransparentQueue = bs_shared_ptr_new<RenderQueue>(transparentStateReduction);
	}

	void RendererCamera::setPostProcessSettings(const SPtr<PostProcessSettings>& ppSettings)
	{
		if (mPostProcessInfo.settings == nullptr)
			mPostProcessInfo.settings = bs_shared_ptr_new<StandardPostProcessSettings>();

		SPtr<StandardPostProcessSettings> stdPPSettings = std::static_pointer_cast<StandardPostProcessSettings>(ppSettings);
		if (stdPPSettings != nullptr)
			*mPostProcessInfo.settings = *stdPPSettings;
		else
			*mPostProcessInfo.settings = StandardPostProcessSettings();

		mPostProcessInfo.settingDirty = true;
	}

	void RendererCamera::setTransform(const Vector3& origin, const Vector3& direction, const Matrix4& view, 
									  const Matrix4& proj, const ConvexVolume& worldFrustum)
	{
		mViewDesc.viewOrigin = origin;
		mViewDesc.viewDirection = direction;
		mViewDesc.viewTransform = view;
		mViewDesc.projTransform = proj;
		mViewDesc.cullFrustum = worldFrustum;
	}

	void RendererCamera::setView(const RENDERER_VIEW_DESC& desc)
	{
		mViewDesc = desc;

		setStateReductionMode(desc.stateReduction);
	}

	void RendererCamera::beginRendering(bool useGBuffer)
	{
		if (useGBuffer)
		{
			// Render scene objects to g-buffer
			bool createGBuffer = mRenderTargets == nullptr ||
				mRenderTargets->getHDR() != mViewDesc.isHDR ||
				mRenderTargets->getNumSamples() != mViewDesc.target.numSamples;

			if (createGBuffer)
				mRenderTargets = RenderTargets::create(mViewDesc.target, mViewDesc.isHDR);

			mRenderTargets->allocate();
			mUsingGBuffer = true;
		}
	}

	void RendererCamera::endRendering()
	{
		mOpaqueQueue->clear();
		mTransparentQueue->clear();

		if(mUsingGBuffer)
		{
			mRenderTargets->release();
			mUsingGBuffer = false;
		}
	}

	void RendererCamera::determineVisible(const Vector<RendererObject*>& renderables, const Vector<CullInfo>& cullInfos,
		Vector<bool>* visibility)
	{
		mVisibility.renderables.clear();
		mVisibility.renderables.resize(renderables.size(), false);

		if (mViewDesc.isOverlay)
			return;

		calculateVisibility(cullInfos, mVisibility.renderables);

		// Update per-object param buffers and queue render elements
		for(UINT32 i = 0; i < (UINT32)cullInfos.size(); i++)
		{
			if (!mVisibility.renderables[i])
				continue;

			const AABox& boundingBox = cullInfos[i].bounds.getBox();
			float distanceToCamera = (mViewDesc.viewOrigin - boundingBox.getCenter()).length();

			for (auto& renderElem : renderables[i]->elements)
			{
				// Note: I could keep opaque and transparent renderables in two separate arrays, so I don't need to do the
				// check here
				bool isTransparent = (renderElem.material->getShader()->getFlags() & (UINT32)ShaderFlags::Transparent) != 0;

				if (isTransparent)
					mTransparentQueue->add(&renderElem, distanceToCamera);
				else
					mOpaqueQueue->add(&renderElem, distanceToCamera);
			}
		}

		if(visibility != nullptr)
		{
			for (UINT32 i = 0; i < (UINT32)renderables.size(); i++)
			{
				bool visible = (*visibility)[i];

				(*visibility)[i] = visible || mVisibility.renderables[i];
			}
		}

		mOpaqueQueue->sort();
		mTransparentQueue->sort();
	}

	void RendererCamera::calculateVisibility(const Vector<CullInfo>& cullInfos, Vector<bool>& visibility) const
	{
		UINT64 cameraLayers = mViewDesc.visibleLayers;
		const ConvexVolume& worldFrustum = mViewDesc.cullFrustum;

		for (UINT32 i = 0; i < (UINT32)cullInfos.size(); i++)
		{
			if ((cullInfos[i].layer & cameraLayers) == 0)
				continue;

			// Do frustum culling
			// Note: This is bound to be a bottleneck at some point. When it is ensure that intersect methods use vector
			// operations, as it is trivial to update them. Also consider spatial partitioning.
			const Sphere& boundingSphere = cullInfos[i].bounds.getSphere();
			if (worldFrustum.intersects(boundingSphere))
			{
				// More precise with the box
				const AABox& boundingBox = cullInfos[i].bounds.getBox();

				if (worldFrustum.intersects(boundingBox))
					visibility[i] = true;
			}
		}
	}

	Vector2 RendererCamera::getDeviceZTransform(const Matrix4& projMatrix) const
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

		RenderAPI& rapi = RenderAPI::instance();
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

	void RendererCamera::updatePerViewBuffer()
	{
		Matrix4 viewProj = mViewDesc.projTransform * mViewDesc.viewTransform;
		Matrix4 invViewProj = viewProj.inverse();

		gPerCameraParamDef.gMatProj.set(mParamBuffer, mViewDesc.projTransform);
		gPerCameraParamDef.gMatView.set(mParamBuffer, mViewDesc.viewTransform);
		gPerCameraParamDef.gMatViewProj.set(mParamBuffer, viewProj);
		gPerCameraParamDef.gMatInvViewProj.set(mParamBuffer, invViewProj); // Note: Calculate inverses separately (better precision possibly)
		gPerCameraParamDef.gMatInvProj.set(mParamBuffer, mViewDesc.projTransform.inverse());

		// Construct a special inverse view-projection matrix that had projection entries that affect z and w eliminated.
		// Used to transform a vector(clip_x, clip_y, view_z, view_w), where clip_x/clip_y are in clip space, and 
		// view_z/view_w in view space, into world space.

		// Only projects z/w coordinates
		Matrix4 projZ = Matrix4::IDENTITY;
		projZ[2][2] = mViewDesc.projTransform[2][2];
		projZ[2][3] = mViewDesc.projTransform[2][3];
		projZ[3][2] = mViewDesc.projTransform[3][2];
		projZ[3][3] = 0.0f;

		gPerCameraParamDef.gMatScreenToWorld.set(mParamBuffer, invViewProj * projZ);
		gPerCameraParamDef.gViewDir.set(mParamBuffer, mViewDesc.viewDirection);
		gPerCameraParamDef.gViewOrigin.set(mParamBuffer, mViewDesc.viewOrigin);
		gPerCameraParamDef.gDeviceZToWorldZ.set(mParamBuffer, getDeviceZTransform(mViewDesc.projTransform));

		const Rect2I& viewRect = mViewDesc.target.viewRect;

		float halfWidth = viewRect.width * 0.5f;
		float halfHeight = viewRect.height * 0.5f;

		float rtWidth = mViewDesc.target.targetWidth != 0 ? (float)mViewDesc.target.targetWidth : 20.0f;
		float rtHeight = mViewDesc.target.targetHeight != 0 ? (float)mViewDesc.target.targetHeight : 20.0f;

		RenderAPI& rapi = RenderAPI::instance();
		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();

		Vector4 clipToUVScaleOffset;
		clipToUVScaleOffset.x = halfWidth / rtWidth;
		clipToUVScaleOffset.y = -halfHeight / rtHeight;
		clipToUVScaleOffset.z = viewRect.x / rtWidth + (halfWidth + rapiInfo.getHorizontalTexelOffset()) / rtWidth;
		clipToUVScaleOffset.w = viewRect.y / rtHeight + (halfHeight + rapiInfo.getVerticalTexelOffset()) / rtHeight;

		// Either of these flips the Y axis, but if they're both true they cancel out
		if (rapiInfo.getUVYAxisUp() ^ rapiInfo.getNDCYAxisDown())
			clipToUVScaleOffset.y = -clipToUVScaleOffset.y;

		gPerCameraParamDef.gClipToUVScaleOffset.set(mParamBuffer, clipToUVScaleOffset);
	}
}}