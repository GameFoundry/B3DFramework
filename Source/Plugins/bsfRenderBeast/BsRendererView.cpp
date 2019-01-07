//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererView.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsRenderable.h"
#include "Renderer/BsRendererUtility.h"
#include "Material/BsMaterial.h"
#include "Material/BsShader.h"
#include "Material/BsGpuParamsSet.h"
#include "BsRendererLight.h"
#include "BsRendererScene.h"
#include "BsRenderBeast.h"
#include <BsRendererDecal.h>

namespace bs { namespace ct
{
	PerCameraParamDef gPerCameraParamDef;
	SkyboxParamDef gSkyboxParamDef;

	SkyboxMat::SkyboxMat()
	{
		if(mParams->hasTexture(GPT_FRAGMENT_PROGRAM, "gSkyTex"))
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSkyTex", mSkyTextureParam);

		mParamBuffer = gSkyboxParamDef.createBuffer();

		if(mParams->hasParamBlock(GPT_FRAGMENT_PROGRAM, "Params"))
			mParams->setParamBlockBuffer("Params", mParamBuffer);
	}

	void SkyboxMat::bind(const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<Texture>& texture, const Color& solidColor)
	{
		mParams->setParamBlockBuffer("PerCamera", perCamera);

		mSkyTextureParam.set(texture);

		gSkyboxParamDef.gClearColor.set(mParamBuffer, solidColor);
		mParamBuffer->flushToGPU();

		RendererMaterial::bind();
	}

	SkyboxMat* SkyboxMat::getVariation(bool color)
	{
		if (color)
			return get(getVariation<true>());

		return get(getVariation<false>());
	}

	RendererViewData::RendererViewData()
		:encodeDepth(false), depthEncodeNear(0.0f), depthEncodeFar(0.0f)
	{
		
	}

	RendererViewProperties::RendererViewProperties(const RENDERER_VIEW_DESC& src)
		:RendererViewData(src), frameIdx(0), target(src.target)
	{
		viewProjTransform = src.projTransform * src.viewTransform;
	}

	RendererView::RendererView()
		: mCamera(nullptr), mRenderSettingsHash(0), mViewIdx(-1)
	{
		mParamBuffer = gPerCameraParamDef.createBuffer();
	}

	RendererView::RendererView(const RENDERER_VIEW_DESC& desc)
		: mProperties(desc), mCamera(desc.sceneCamera), mRenderSettingsHash(0), mViewIdx(-1)
	{
		mParamBuffer = gPerCameraParamDef.createBuffer();
		mProperties.prevViewProjTransform = mProperties.viewProjTransform;

		setStateReductionMode(desc.stateReduction);
	}

	void RendererView::setStateReductionMode(StateReduction reductionMode)
	{
		mDeferredOpaqueQueue = bs_shared_ptr_new<RenderQueue>(reductionMode);
		mForwardOpaqueQueue = bs_shared_ptr_new<RenderQueue>(reductionMode);

		StateReduction transparentStateReduction = reductionMode;
		if (transparentStateReduction == StateReduction::Material)
			transparentStateReduction = StateReduction::Distance; // Transparent object MUST be sorted by distance

		mTransparentQueue = bs_shared_ptr_new<RenderQueue>(transparentStateReduction);
		mDecalQueue = bs_shared_ptr_new<RenderQueue>(StateReduction::Material);
	}

	void RendererView::setRenderSettings(const SPtr<RenderSettings>& settings)
	{
		if (mRenderSettings == nullptr)
			mRenderSettings = bs_shared_ptr_new<RenderSettings>();

		if (settings != nullptr)
			*mRenderSettings = *settings;

		mRenderSettingsHash++;

		// Update compositor hierarchy (Note: Needs to be called even when viewport size (or other information) changes,
		// but we're currently calling it here as all such calls are followed by setRenderSettings.
		mCompositor.build(*this, RCNodeFinalResolve::getNodeId());
	}

	void RendererView::setTransform(const Vector3& origin, const Vector3& direction, const Matrix4& view, 
									  const Matrix4& proj, const ConvexVolume& worldFrustum)
	{
		mProperties.viewOrigin = origin;
		mProperties.viewDirection = direction;
		mProperties.viewTransform = view;
		mProperties.projTransform = proj;
		mProperties.cullFrustum = worldFrustum;
		mProperties.viewProjTransform = proj * view;
	}

	void RendererView::setView(const RENDERER_VIEW_DESC& desc)
	{
		mCamera = desc.sceneCamera;
		mProperties = desc;
		mProperties.viewProjTransform = desc.projTransform * desc.viewTransform;
		mProperties.prevViewProjTransform = Matrix4::IDENTITY;
		mProperties.target = desc.target;

		setStateReductionMode(desc.stateReduction);
	}

	void RendererView::beginFrame()
	{
		// Check if render target resized and update the view properties accordingly
		// Note: Normally we rely on the renderer notify* methods to let us know of changes to camera/viewport, but since
		// render target resize can often originate from the core thread, this avoids the back and forth between 
		// main <-> core thread, and the frame delay that comes with it
		if(mCamera)
		{
			const SPtr<Viewport>& viewport = mCamera->getViewport();
			if(viewport)
			{
				UINT32 newTargetWidth = 0;
				UINT32 newTargetHeight = 0;
				if (mProperties.target.target != nullptr)
				{
					newTargetWidth = mProperties.target.target->getProperties().width;
					newTargetHeight = mProperties.target.target->getProperties().height;
				}

				if(newTargetWidth != mProperties.target.targetWidth ||
					newTargetHeight != mProperties.target.targetHeight)
				{
					mProperties.target.viewRect = viewport->getPixelArea();
					mProperties.target.targetWidth = newTargetWidth;
					mProperties.target.targetHeight = newTargetHeight;
					
					updatePerViewBuffer();
				}
			}
		}

		// Note: inverse view-projection can be cached, it doesn't change every frame
		Matrix4 viewProj = mProperties.projTransform * mProperties.viewTransform;
		Matrix4 invViewProj = viewProj.inverse();
		Matrix4 NDCToPrevNDC = mProperties.prevViewProjTransform * invViewProj;
		
		gPerCameraParamDef.gNDCToPrevNDC.set(mParamBuffer, NDCToPrevNDC);
	}

	void RendererView::endFrame()
	{
		// Save view-projection matrix to use for temporal filtering
		mProperties.prevViewProjTransform = mProperties.viewProjTransform;

		// Advance per-view frame index. This is used primarily by temporal rendering effects, and pausing the frame index
		// allows you to freeze the current rendering as is, without temporal artifacts.
		mProperties.frameIdx++;

		mDeferredOpaqueQueue->clear();
		mForwardOpaqueQueue->clear();
		mTransparentQueue->clear();
		mDecalQueue->clear();
	}

	void RendererView::determineVisible(const Vector<RendererRenderable*>& renderables, const Vector<CullInfo>& cullInfos,
		Vector<bool>* visibility)
	{
		mVisibility.renderables.clear();
		mVisibility.renderables.resize(renderables.size(), false);

		if (mRenderSettings->overlayOnly)
			return;

		calculateVisibility(cullInfos, mVisibility.renderables);

		if(visibility != nullptr)
		{
			for (UINT32 i = 0; i < (UINT32)renderables.size(); i++)
			{
				bool visible = (*visibility)[i];

				(*visibility)[i] = visible || mVisibility.renderables[i];
			}
		}
	}

	void RendererView::determineVisible(const Vector<RendererParticles>& particleSystems, const Vector<CullInfo>& cullInfos, 
		Vector<bool>* visibility)
	{
		mVisibility.particleSystems.clear();
		mVisibility.particleSystems.resize(particleSystems.size(), false);

		if (mRenderSettings->overlayOnly)
			return;

		calculateVisibility(cullInfos, mVisibility.particleSystems);

		if(visibility != nullptr)
		{
			for (UINT32 i = 0; i < (UINT32)particleSystems.size(); i++)
			{
				bool visible = (*visibility)[i];

				(*visibility)[i] = visible || mVisibility.particleSystems[i];
			}
		}
	}

	void RendererView::determineVisible(const Vector<RendererDecal>& decals, const Vector<CullInfo>& cullInfos, 
		Vector<bool>* visibility)
	{
		mVisibility.decals.clear();
		mVisibility.decals.resize(decals.size(), false);

		if (mRenderSettings->overlayOnly)
			return;

		calculateVisibility(cullInfos, mVisibility.decals);

		if(visibility != nullptr)
		{
			for (UINT32 i = 0; i < (UINT32)decals.size(); i++)
			{
				bool visible = (*visibility)[i];

				(*visibility)[i] = visible || mVisibility.decals[i];
			}
		}
	}

	void RendererView::determineVisible(const Vector<RendererLight>& lights, const Vector<Sphere>& bounds, 
		LightType lightType, Vector<bool>* visibility)
	{
		// Special case for directional lights, they're always visible
		if(lightType == LightType::Directional)
		{
			if (visibility)
				visibility->assign(lights.size(), true);

			return;
		}

		Vector<bool>* perViewVisibility;
		if(lightType == LightType::Radial)
		{
			mVisibility.radialLights.clear();
			mVisibility.radialLights.resize(lights.size(), false);

			perViewVisibility = &mVisibility.radialLights;
		}
		else // Spot
		{
			mVisibility.spotLights.clear();
			mVisibility.spotLights.resize(lights.size(), false);

			perViewVisibility = &mVisibility.spotLights;
		}

		if (mRenderSettings->overlayOnly)
			return;

		calculateVisibility(bounds, *perViewVisibility);

		if(visibility != nullptr)
		{
			for (UINT32 i = 0; i < (UINT32)lights.size(); i++)
			{
				bool visible = (*visibility)[i];

				(*visibility)[i] = visible || (*perViewVisibility)[i];
			}
		}
	}

	void RendererView::calculateVisibility(const Vector<CullInfo>& cullInfos, Vector<bool>& visibility) const
	{
		UINT64 cameraLayers = mProperties.visibleLayers;
		const ConvexVolume& worldFrustum = mProperties.cullFrustum;
		const Vector3& worldCameraPosition = mCamera->getTransform().getPosition();
		float baseCullDistance = mRenderSettings->cullDistance;

		for (UINT32 i = 0; i < (UINT32)cullInfos.size(); i++)
		{
			if ((cullInfos[i].layer & cameraLayers) == 0)
				continue;

			// Do distance culling
			const Sphere& boundingSphere = cullInfos[i].bounds.getSphere();
			const Vector3& worldRenderablePosition = boundingSphere.getCenter();

			float distanceToCameraSq = worldCameraPosition.squaredDistance(worldRenderablePosition);
			float correctedCullDistance = cullInfos[i].cullDistanceFactor * baseCullDistance;
			float maxDistanceToCamera = correctedCullDistance + boundingSphere.getRadius();

			if (distanceToCameraSq > maxDistanceToCamera * maxDistanceToCamera)
				continue;

			// Do frustum culling
			// Note: This is bound to be a bottleneck at some point. When it is ensure that intersect methods use vector
			// operations, as it is trivial to update them. Also consider spatial partitioning.
			if (worldFrustum.intersects(boundingSphere))
			{
				// More precise with the box
				const AABox& boundingBox = cullInfos[i].bounds.getBox();

				if (worldFrustum.intersects(boundingBox))
					visibility[i] = true;
			}
		}
	}

	void RendererView::calculateVisibility(const Vector<Sphere>& bounds, Vector<bool>& visibility) const
	{
		const ConvexVolume& worldFrustum = mProperties.cullFrustum;

		for (UINT32 i = 0; i < (UINT32)bounds.size(); i++)
		{
			if (worldFrustum.intersects(bounds[i]))
				visibility[i] = true;
		}
	}

	void RendererView::calculateVisibility(const Vector<AABox>& bounds, Vector<bool>& visibility) const
	{
		const ConvexVolume& worldFrustum = mProperties.cullFrustum;

		for (UINT32 i = 0; i < (UINT32)bounds.size(); i++)
		{
			if (worldFrustum.intersects(bounds[i]))
				visibility[i] = true;
		}
	}

	void RendererView::queueRenderElements(const SceneInfo& sceneInfo)
	{
		if (mRenderSettings->overlayOnly)
			return;

		// Queue renderables
		for(UINT32 i = 0; i < (UINT32)sceneInfo.renderables.size(); i++)
		{
			if (!mVisibility.renderables[i])
				continue;

			const AABox& boundingBox = sceneInfo.renderableCullInfos[i].bounds.getBox();
			const float distanceToCamera = (mProperties.viewOrigin - boundingBox.getCenter()).length();

			for (auto& renderElem : sceneInfo.renderables[i]->elements)
			{
				// Note: I could keep renderables in multiple separate arrays, so I don't need to do the check here
				ShaderFlags shaderFlags = renderElem.material->getShader()->getFlags();

				if (shaderFlags.isSet(ShaderFlag::Transparent))
					mTransparentQueue->add(&renderElem, distanceToCamera, renderElem.techniqueIdx);
				else if (shaderFlags.isSet(ShaderFlag::Forward))
					mForwardOpaqueQueue->add(&renderElem, distanceToCamera, renderElem.techniqueIdx);
				else
					mDeferredOpaqueQueue->add(&renderElem, distanceToCamera, renderElem.techniqueIdx);
			}
		}

		// Queue particle systems
		for(UINT32 i = 0; i < (UINT32)sceneInfo.particleSystems.size(); i++)
		{
			if (!mVisibility.particleSystems[i])
				continue;

			const ParticlesRenderElement& renderElem = sceneInfo.particleSystems[i].renderElement;
			if (!renderElem.isValid())
				continue;

			const AABox& boundingBox = sceneInfo.particleSystemCullInfos[i].bounds.getBox();
			const float distanceToCamera = (mProperties.viewOrigin - boundingBox.getCenter()).length();

			ShaderFlags shaderFlags = renderElem.material->getShader()->getFlags();

			if (shaderFlags.isSet(ShaderFlag::Transparent))
				mTransparentQueue->add(&renderElem, distanceToCamera, renderElem.techniqueIdx);
			else if (shaderFlags.isSet(ShaderFlag::Forward))
				mForwardOpaqueQueue->add(&renderElem, distanceToCamera, renderElem.techniqueIdx);
			else
				mDeferredOpaqueQueue->add(&renderElem, distanceToCamera, renderElem.techniqueIdx);
		}

		// Queue decals
		const bool isMSAA = mProperties.target.numSamples > 1;
		for(UINT32 i = 0; i < (UINT32)sceneInfo.decals.size(); i++)
		{
			if (!mVisibility.decals[i])
				continue;

			const DecalRenderElement& renderElem = sceneInfo.decals[i].renderElement;

			// Note: I could keep renderables in multiple separate arrays, so I don't need to do the check here
			ShaderFlags shaderFlags = renderElem.material->getShader()->getFlags();

			// Decals are only supported using deferred rendering
			if (shaderFlags.isSetAny(ShaderFlag::Transparent | ShaderFlag::Forward))
				continue;

			const AABox& boundingBox = sceneInfo.decalCullInfos[i].bounds.getBox();
			const float distanceToCamera = (mProperties.viewOrigin - boundingBox.getCenter()).length();

			// Check if viewer is inside the decal volume

			// Extend the bounds slighty to cover the case when the viewer is outside, but the near plane is intersecting
			// the decal bounds. We need to be conservative since the material for rendering outside will not properly
			// render the inside of the decal volume.
			const bool isInside = boundingBox.contains(mProperties.viewOrigin, mProperties.nearPlane * 3.0f);
			const UINT32* techniqueIndices = renderElem.techniqueIndices[(INT32)isInside];

			// No MSAA evaluation, or same value for all samples (no divergence between samples)
			mDecalQueue->add(&renderElem, distanceToCamera, 
				techniqueIndices[(INT32)(isMSAA ? MSAAMode::Single : MSAAMode::None)]);

			// Evaluates all MSAA samples for pixels that are marked as divergent
			if(isMSAA)
				mDecalQueue->add(&renderElem, distanceToCamera, techniqueIndices[(INT32)MSAAMode::Full]);
		}

		mForwardOpaqueQueue->sort();
		mDeferredOpaqueQueue->sort();
		mTransparentQueue->sort();
		mDecalQueue->sort();
	}

	Vector2 RendererView::getDeviceZToViewZ(const Matrix4& projMatrix)
	{
		// Returns a set of values that will transform depth buffer values (in range [0, 1]) to a distance
		// in view space. This involes applying the inverse projection transform to the depth value. When you multiply
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

		if (c != 0.0f)
		{
			output.x = b / (depthRange * c);
			output.y = minDepth / depthRange - a / (depthRange * c);
		}
		else // Ortographic, assuming viewing towards negative Z
		{
			output.x = b / -depthRange;
			output.y = minDepth / depthRange - a / -depthRange;
		}

		return output;
	}

	Vector2 RendererView::getNDCZToViewZ(const Matrix4& projMatrix)
	{
		// Returns a set of values that will transform depth buffer values (e.g. [0, 1] in DX, [-1, 1] in GL) to a distance
		// in view space. This involes applying the inverse projection transform to the depth value. When you multiply
		// a vector with the projection matrix you get [clipX, clipY, Az + B, C * z], where we don't care about clipX/clipY.
		// A is [2, 2], B is [2, 3] and C is [3, 2] elements of the projection matrix (only ones that matter for our depth 
		// value). The hardware will also automatically divide the z value with w to get the depth, therefore the final 
		// formula is:
		// depth = (Az + B) / (C * z)

		// To get the z coordinate back we simply do the opposite: 
		// z = B / (depth * C - A)

		// Are we reorganize it because it needs to fit the "(1.0f / (depth + y)) * x" format used in the shader:
		// z = 1.0f / (depth - A/C) * B/C

		float a = projMatrix[2][2];
		float b = projMatrix[2][3];
		float c = projMatrix[3][2];

		Vector2 output;

		if (c != 0.0f)
		{
			output.x = b / c;
			output.y = -a / c;
		}
		else // Ortographic, assuming viewing towards negative Z
		{
			output.x = -b;
			output.y = a;
		}

		return output;
	}

	Vector2 RendererView::getNDCZToDeviceZ()
	{
		RenderAPI& rapi = RenderAPI::instance();
		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();

		Vector2 ndcZToDeviceZ;
		ndcZToDeviceZ.x = 1.0f / (rapiInfo.getMaximumDepthInputValue() - rapiInfo.getMinimumDepthInputValue());
		ndcZToDeviceZ.y = -rapiInfo.getMinimumDepthInputValue();

		return ndcZToDeviceZ;
	}

	Matrix4 invertProjectionMatrix(const Matrix4& mat)
	{
		// Try to solve the most common case using high percision calculations, in order to reduce depth error
		if(mat[0][1] == 0.0f && mat[0][3] == 0.0f &&
		   mat[1][0] == 0.0f && mat[1][3] == 0.0f &&
		   mat[2][0] == 0.0f && mat[2][1] == 0.0f &&
		   mat[3][0] == 0.0f && mat[3][1] == 0.0f &&
		   mat[3][2] == -1.0f && mat[3][3] == 0.0f)
		{
			double a = mat[0][0];
			double b = mat[1][1];
			double c = mat[2][2];
			double d = mat[2][3];
			double s = mat[0][2];
			double t = mat[1][2];

			return Matrix4(
				(float)(1.0/a), 0.0f, 0.0f, (float)(-s/a),
				0.0f, (float)(1.0/b), 0.0f, (float)(-t/b),
				0.0f, 0.0f, 0.0f, -1.0f,
				0.0f, 0.0f, (float)(1.0/d), (float)(c/d)
				);
		}
		else
		{
			return mat.inverse();
		}
	}

	void RendererView::updatePerViewBuffer()
	{
		Matrix4 viewProj = mProperties.projTransform * mProperties.viewTransform;
		Matrix4 invProj = invertProjectionMatrix(mProperties.projTransform);
		Matrix4 invView = mProperties.viewTransform.inverseAffine();
		Matrix4 invViewProj = invView * invProj;

		gPerCameraParamDef.gMatProj.set(mParamBuffer, mProperties.projTransform);
		gPerCameraParamDef.gMatView.set(mParamBuffer, mProperties.viewTransform);
		gPerCameraParamDef.gMatViewProj.set(mParamBuffer, viewProj);
		gPerCameraParamDef.gMatInvViewProj.set(mParamBuffer, invViewProj);
		gPerCameraParamDef.gMatInvProj.set(mParamBuffer, invProj);

		// Construct a special inverse view-projection matrix that had projection entries that effect z and w eliminated.
		// Used to transform a vector(clip_x, clip_y, view_z, view_w), where clip_x/clip_y are in clip space, and 
		// view_z/view_w in view space, into world space.

		// Only projects z/w coordinates (cancels out with the inverse matrix below)
		Matrix4 projZ = Matrix4::IDENTITY;
		projZ[2][2] = mProperties.projTransform[2][2];
		projZ[2][3] = mProperties.projTransform[2][3];
		projZ[3][2] = mProperties.projTransform[3][2];
		projZ[3][3] = 0.0f;

		Matrix4 NDCToPrevNDC = mProperties.prevViewProjTransform * invViewProj;
		
		gPerCameraParamDef.gMatScreenToWorld.set(mParamBuffer, invViewProj * projZ);
		gPerCameraParamDef.gNDCToPrevNDC.set(mParamBuffer, NDCToPrevNDC);
		gPerCameraParamDef.gViewDir.set(mParamBuffer, mProperties.viewDirection);
		gPerCameraParamDef.gViewOrigin.set(mParamBuffer, mProperties.viewOrigin);
		gPerCameraParamDef.gDeviceZToWorldZ.set(mParamBuffer, getDeviceZToViewZ(mProperties.projTransform));
		gPerCameraParamDef.gNDCZToWorldZ.set(mParamBuffer, getNDCZToViewZ(mProperties.projTransform));
		gPerCameraParamDef.gNDCZToDeviceZ.set(mParamBuffer, getNDCZToDeviceZ());

		Vector2 nearFar(mProperties.nearPlane, mProperties.farPlane);
		gPerCameraParamDef.gNearFar.set(mParamBuffer, nearFar);

		const Rect2I& viewRect = mProperties.target.viewRect;

		Vector4I viewportRect;
		viewportRect[0] = viewRect.x;
		viewportRect[1] = viewRect.y;
		viewportRect[2] = viewRect.width;
		viewportRect[3] = viewRect.height;

		gPerCameraParamDef.gViewportRectangle.set(mParamBuffer, viewportRect);

		Vector4 ndcToUV = getNDCToUV();
		gPerCameraParamDef.gClipToUVScaleOffset.set(mParamBuffer, ndcToUV);

		Vector4 uvToNDC(
			1.0f / ndcToUV.x,
			1.0f / ndcToUV.y,
			-ndcToUV.z / ndcToUV.x,
			-ndcToUV.w / ndcToUV.y);
		gPerCameraParamDef.gUVToClipScaleOffset.set(mParamBuffer, uvToNDC);

		if (!mRenderSettings->enableLighting)
			gPerCameraParamDef.gAmbientFactor.set(mParamBuffer, 100.0f);
		else
			gPerCameraParamDef.gAmbientFactor.set(mParamBuffer, 0.0f);
	}

	Vector4 RendererView::getNDCToUV() const
	{
		RenderAPI& rapi = RenderAPI::instance();
		const RenderAPIInfo& rapiInfo = rapi.getAPIInfo();
		const Rect2I& viewRect = mProperties.target.viewRect;
		
		float halfWidth = viewRect.width * 0.5f;
		float halfHeight = viewRect.height * 0.5f;

		float rtWidth = mProperties.target.targetWidth != 0 ? (float)mProperties.target.targetWidth : 20.0f;
		float rtHeight = mProperties.target.targetHeight != 0 ? (float)mProperties.target.targetHeight : 20.0f;

		Vector4 ndcToUV;
		ndcToUV.x = halfWidth / rtWidth;
		ndcToUV.y = -halfHeight / rtHeight;
		ndcToUV.z = viewRect.x / rtWidth + (halfWidth + rapiInfo.getHorizontalTexelOffset()) / rtWidth;
		ndcToUV.w = viewRect.y / rtHeight + (halfHeight + rapiInfo.getVerticalTexelOffset()) / rtHeight;

		// Either of these flips the Y axis, but if they're both true they cancel out
		if (rapiInfo.isFlagSet(RenderAPIFeatureFlag::UVYAxisUp) ^ rapiInfo.isFlagSet(RenderAPIFeatureFlag::NDCYAxisDown))
			ndcToUV.y = -ndcToUV.y;

		return ndcToUV;
	}

	void RendererView::updateLightGrid(const VisibleLightData& visibleLightData, 
		const VisibleReflProbeData& visibleReflProbeData)
	{
		mLightGrid.updateGrid(*this, visibleLightData, visibleReflProbeData, !mRenderSettings->enableLighting);
	}

	RendererViewGroup::RendererViewGroup(RendererView** views, UINT32 numViews, bool mainPass, UINT32 shadowMapSize)
		: mIsMainPass(mainPass), mShadowRenderer(shadowMapSize)
	{
		setViews(views, numViews);
	}

	void RendererViewGroup::setViews(RendererView** views, UINT32 numViews)
	{
		mViews.clear();

		for (UINT32 i = 0; i < numViews; i++)
		{
			mViews.push_back(views[i]);
			views[i]->_setViewIdx(i);
		}
	}

	void RendererViewGroup::determineVisibility(const SceneInfo& sceneInfo)
	{
		const auto numViews = (UINT32)mViews.size();

		// Early exit if no views render scene geometry
		bool allViewsOverlay = false;
		for (UINT32 i = 0; i < numViews; i++)
		{
			if (!mViews[i]->getRenderSettings().overlayOnly)
			{
				allViewsOverlay = false;
				break;
			}
		}

		if (allViewsOverlay)
			return;

		// Calculate renderable visibility per view
		mVisibility.renderables.resize(sceneInfo.renderables.size(), false);
		mVisibility.renderables.assign(sceneInfo.renderables.size(), false);

		mVisibility.particleSystems.resize(sceneInfo.particleSystems.size(), false);
		mVisibility.particleSystems.assign(sceneInfo.particleSystems.size(), false);

		mVisibility.decals.resize(sceneInfo.decals.size(), false);
		mVisibility.decals.assign(sceneInfo.decals.size(), false);

		for(UINT32 i = 0; i < numViews; i++)
		{
			mViews[i]->determineVisible(sceneInfo.renderables, sceneInfo.renderableCullInfos, &mVisibility.renderables);
			mViews[i]->determineVisible(sceneInfo.particleSystems, sceneInfo.particleSystemCullInfos, &mVisibility.particleSystems);
			mViews[i]->determineVisible(sceneInfo.decals, sceneInfo.decalCullInfos, &mVisibility.decals);
		}
		
		// Generate render queues per camera
		for(UINT32 i = 0; i < numViews; i++)
			mViews[i]->queueRenderElements(sceneInfo);

		// Calculate light visibility for all views
		const auto numRadialLights = (UINT32)sceneInfo.radialLights.size();
		mVisibility.radialLights.resize(numRadialLights, false);
		mVisibility.radialLights.assign(numRadialLights, false);

		const auto numSpotLights = (UINT32)sceneInfo.spotLights.size();
		mVisibility.spotLights.resize(numSpotLights, false);
		mVisibility.spotLights.assign(numSpotLights, false);

		for (UINT32 i = 0; i < numViews; i++)
		{
			if (mViews[i]->getRenderSettings().overlayOnly)
				continue;

			mViews[i]->determineVisible(sceneInfo.radialLights, sceneInfo.radialLightWorldBounds, LightType::Radial,
				&mVisibility.radialLights);

			mViews[i]->determineVisible(sceneInfo.spotLights, sceneInfo.spotLightWorldBounds, LightType::Spot,
				&mVisibility.spotLights);
		}

		// Calculate refl. probe visibility for all views
		const auto numProbes = (UINT32)sceneInfo.reflProbes.size();
		mVisibility.reflProbes.resize(numProbes, false);
		mVisibility.reflProbes.assign(numProbes, false);

		// Note: Per-view visibility for refl. probes currently isn't calculated
		for (UINT32 i = 0; i < numViews; i++)
		{
			const auto& viewProps = mViews[i]->getProperties();

			// Don't recursively render reflection probes when generating reflection probe maps
			if (viewProps.capturingReflections)
				continue;

			mViews[i]->calculateVisibility(sceneInfo.reflProbeWorldBounds, mVisibility.reflProbes);
		}

		// Organize light and refl. probe visibility infomation in a more GPU friendly manner

		// Note: I'm determining light and refl. probe visibility for the entire group. It might be more performance
		// efficient to do it per view. Additionally I'm using a single GPU buffer to hold their information, which is
		// then updated when each view group is rendered. It might be better to keep one buffer reserved per-view.
		mVisibleLightData.update(sceneInfo, *this);
		mVisibleReflProbeData.update(sceneInfo, *this);

		const bool supportsClusteredForward = gRenderBeast()->getFeatureSet() == RenderBeastFeatureSet::Desktop;
		if(supportsClusteredForward)
		{
			for (UINT32 i = 0; i < numViews; i++)
			{
				if (mViews[i]->getRenderSettings().overlayOnly)
					continue;

				mViews[i]->updateLightGrid(mVisibleLightData, mVisibleReflProbeData);
			}
		}
	}
}}
