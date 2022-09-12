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
#include "BsRendererDecal.h"
#include "Animation/BsAnimationManager.h"
#include "RenderAPI/BsCommandBuffer.h"

namespace bs { namespace ct
{
	PerCameraParamDef gPerCameraParamDef;
	SkyboxParamDef gSkyboxParamDef;

	SkyboxMat::SkyboxMat()
	{
		if(mParams->HasTexture(GPT_FRAGMENT_PROGRAM, "gSkyTex"))
			mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSkyTex", mSkyTextureParam);

		mParamBuffer = gSkyboxParamDef.CreateBuffer();

		if(mParams->HasParamBlock(GPT_FRAGMENT_PROGRAM, "Params"))
			mParams->SetParamBlockBuffer("Params", mParamBuffer);
	}

	void SkyboxMat::Bind(const SPtr<GpuParamBlockBuffer>& perCamera, const SPtr<Texture>& texture, const Color& solidColor)
	{
		mParams->SetParamBlockBuffer("PerCamera", perCamera);

		mSkyTextureParam.Set(texture);

		gSkyboxParamDef.gClearColor.Set(mParamBuffer, solidColor);
		mParamBuffer->FlushToGPU();

		RendererMaterial::bind();
	}

	SkyboxMat* SkyboxMat::getVariation(bool color)
	{
		if (color)
			return Get(getVariation<true>());

		return Get(getVariation<false>());
	}

	RendererViewData::RendererViewData()
		:encodeDepth(false)
	{
		
	}

	RendererViewProperties::RendererViewProperties(const RENDERER_VIEW_DESC& src)
		:RendererViewData(src), frameIdx(0), target(src.target)
	{
		projTransformNoAA = src.projTransform;
		viewProjTransform = src.projTransform * src.viewTransform;
	}

	RendererView::RendererView()
		: MCamera(nullptr), mRenderSettingsHash(0), mViewIdx(-1)
	{
		mParamBuffer = gPerCameraParamDef.CreateBuffer();
	}

	RendererView::RendererView(const RENDERER_VIEW_DESC& desc)
		: MProperties(desc), mCamera(desc.sceneCamera), mRenderSettingsHash(0), mViewIdx(-1)
	{
		mParamBuffer = gPerCameraParamDef.CreateBuffer();
		mProperties.prevViewProjTransform = mProperties.viewProjTransform;

		setStateReductionMode(desc.stateReduction);
	}

	void RendererView::SetStateReductionMode(StateReduction reductionMode)
	{
		mDeferredOpaqueQueue = bs_shared_ptr_new<RenderQueue>(reductionMode);
		mForwardOpaqueQueue = bs_shared_ptr_new<RenderQueue>(reductionMode);

		StateReduction transparentStateReduction = reductionMode;
		if (transparentStateReduction == StateReduction::Material)
			transparentStateReduction = StateReduction::Distance; // Transparent object MUST be sorted by distance

		mTransparentQueue = bs_shared_ptr_new<RenderQueue>(transparentStateReduction);
		mDecalQueue = bs_shared_ptr_new<RenderQueue>(StateReduction::Material);
	}

	void RendererView::SetRenderSettings(const SPtr<RenderSettings>& settings)
	{
		if (mRenderSettings == nullptr)
			mRenderSettings = bs_shared_ptr_new<RenderSettings>();

		if (settings != nullptr)
			*mRenderSettings = *settings;

		mRenderSettingsHash++;

		// Update compositor hierarchy (Note: Needs to be called even when viewport size (or other information) changes,
		// but we're currently calling it here as all such calls are followed by setRenderSettings.
		mCompositor.Build(*this, RCNodeFinalResolve::getNodeId());
	}

	void RendererView::setTransform(const Vector3& origin, const Vector3& direction, const Matrix4& view,
									  const Matrix4& proj, const ConvexVolume& worldFrustum)
	{
		mProperties.viewOrigin = origin;
		mProperties.viewDirection = direction;
		mProperties.viewTransform = view;
		mProperties.projTransform = proj;
		mProperties.projTransformNoAA = proj;
		mProperties.cullFrustum = worldFrustum;
		mProperties.viewProjTransform = proj * view;
		mProperties.temporalJitter = Vector2::ZERO;
	}

	void RendererView::SetView(const RENDERER_VIEW_DESC& desc)
	{
		mCamera = desc.sceneCamera;
		mProperties = desc;
		mProperties.projTransformNoAA = desc.projTransform;
		mProperties.viewProjTransform = desc.projTransform * desc.viewTransform;
		mProperties.prevViewProjTransform = Matrix4::IDENTITY;
		mProperties.target = desc.target;
		mProperties.temporalJitter = Vector2::ZERO;

		setStateReductionMode(desc.stateReduction);
	}

	void RendererView::BeginFrame(const FrameInfo& frameInfo)
	{
		// Check if render target resized and update the view properties accordingly
		// Note: Normally we rely on the renderer notify* methods to let us know of changes to camera/viewport, but since
		// render target resize can often originate from the core thread, this avoids the back and forth between
		// main <-> core thread, and the frame delay that comes with it
		bool perViewBufferDirty = false;
		if(mCamera)
		{
			const SPtr<Viewport>& viewport = mCamera->GetViewport();
			if(viewport)
			{
				UINT32 newTargetWidth = 0;
				UINT32 newTargetHeight = 0;
				if (mProperties.target.target != nullptr)
				{
					newTargetWidth = mProperties.target.target->GetProperties().width;
					newTargetHeight = mProperties.target.target->GetProperties().height;
				}

				if(newTargetWidth != mProperties.target.targetWidth ||
					newTargetHeight != mProperties.target.targetHeight)
				{
					mProperties.target.viewRect = viewport->GetPixelArea();
					mProperties.target.targetWidth = newTargetWidth;
					mProperties.target.targetHeight = newTargetHeight;
					
					perViewBufferDirty = true;
				}
			}
		}

		// Update projection matrix jitter if temporal AA is enabled
		if(mRenderSettings->temporalAA.enabled)
		{
			UINT32 positionCount = mRenderSettings->temporalAA.jitteredPositionCount;
			positionCount = Math::clamp(positionCount, 4U, 128U);
			
			UINT32 positionIndex = mTemporalPositionIdx % positionCount;
			
			if (positionCount == 4)
			{
				// Using a 4x MSAA pattern: http://msdn.microsoft.com/en-us/library/windows/desktop/ff476218(v=vs.85).aspx
				Vector2 samples[] =
				{
					{ -2.0f / 16.0f, -6.0f / 16.0f },
					{  6.0f / 16.0f, -2.0f / 16.0f },
					{  2.0f / 16.0f,  6.0f / 16.0f },
					{ -6.0f / 16.0f,  2.0f / 16.0f }
				};

				mProperties.temporalJitter = samples[positionIndex];
			}
			else
			{
				constexpr float EPSILON = 1e-6f;
				
				float u1 = Math::haltonSequence<float>(positionIndex + 1, 2);
				float u2 = Math::haltonSequence<float>(positionIndex + 1, 3);

				float scale = (2.0f - mRenderSettings->temporalAA.sharpness) * 0.3f;

				float angle = 2.0f * Math::PI * u2;
				float radius = scale * Math::sqrt(-2.0f * Math::log(Math::max(u1, EPSILON)));

				mProperties.temporalJitter = Vector2(radius * Math::cos(angle), radius * Math::sin(angle));
			}

			Vector2 viewSize = Vector2((float)mProperties.target.targetWidth, (float)mProperties.target.targetHeight);
			Vector2 subsampleJitter = mProperties.temporalJitter / viewSize;
			Matrix4 subSampleTranslate = Matrix4::translation(Vector3(subsampleJitter.x, subsampleJitter.y, 0.0f));
			
			mProperties.projTransform = subSampleTranslate * mProperties.projTransformNoAA;
			mProperties.viewProjTransform = mProperties.projTransform * mProperties.viewTransform;

			mTemporalPositionIdx++;
			perViewBufferDirty = true;
		}

		if (perViewBufferDirty)
			updatePerViewBuffer();

		// Note: inverse view-projection can be cached, it doesn't change every frame
		Matrix4 viewProj = mProperties.projTransform * mProperties.viewTransform;
		Matrix4 invViewProj = viewProj.Inverse();
		Matrix4 NDCToPrevNDC = mProperties.prevViewProjTransform * invViewProj;
		
		gPerCameraParamDef.gNDCToPrevNDC.Set(mParamBuffer, NDCToPrevNDC);

		mFrameTimings = frameInfo.timings;
		mAsyncAnim = frameInfo.perFrameData.animation ? frameInfo.perFrameData.animation->async : false;

		// Account for auto-exposure taking multiple frames
		if (mRedrawThisFrame)
		{
			// Note: Doing this here instead of _notifyNeedsRedraw because we need an up-to-date frame index
			if (mRenderSettings->enableHDR && mRenderSettings->enableAutoExposure)
				mWaitingOnAutoExposureFrame = mFrameTimings.frameIdx;
			else
				mWaitingOnAutoExposureFrame = std::numeric_limits<UINT64>::max();
		}
	}

	void RendererView::EndFrame()
	{
		// Save view-projection matrix to use for temporal filtering
		mProperties.prevViewProjTransform = mProperties.viewProjTransform;

		// Advance per-view frame index. This is used primarily by temporal rendering effects, and pausing the frame index
		// allows you to freeze the current rendering as is, without temporal artifacts.
		mProperties.frameIdx++;

		mDeferredOpaqueQueue->Clear();
		mForwardOpaqueQueue->Clear();
		mTransparentQueue->Clear();
		mDecalQueue->Clear();

		if (mRedrawForFrames > 0)
			mRedrawForFrames--;

		if (mRedrawForSeconds > 0.0f)
			mRedrawForSeconds -= mFrameTimings.timeDelta;

		mRedrawThisFrame = false;
	}

	void RendererView::_notifyNeedsRedraw()
	{
		mRedrawThisFrame = true;
		
		// If doing async animation there is a one frame delay
		mRedrawForFrames = mAsyncAnim ? 2 : 1;

		// This will be set once we get the new luminance data from the GPU
		mRedrawForSeconds = 0.0f;
	}

	bool RendererView::ShouldDraw() const
	{
		if (!mProperties.onDemand)
			return true;

		if(mRenderSettings->enableHDR && mRenderSettings->enableAutoExposure)
		{
			constexpr float AUTO_EXPOSURE_TOLERANCE = 0.01f;
			
			// The view was redrawn but we still haven't received the eye adaptation results from the GPU, so
			// we keep redrawing until we do
			if (mWaitingOnAutoExposureFrame != std::numeric_limits<UINT64>::max())
				return true;
			
			// Need to render until the auto-exposure reaches the target exposure
			float eyeAdaptationDiff = Math::abs(mCurrentEyeAdaptation - mPreviousEyeAdaptation);
			if (eyeAdaptationDiff > AUTO_EXPOSURE_TOLERANCE)
				return true;
		}

		return mRedrawForFrames > 0 || mRedrawForSeconds > 0.0f;
	}

	bool RendererView::RequiresVelocityWrites() const
	{
		return mRenderSettings->temporalAA.enabled || mRenderSettings->enableVelocityBuffer;
	}

	void RendererView::UpdateAsyncOperations()
	{
		// Find most recent available frame
		auto lastFinishedIter = mLuminanceUpdates.end();
		for(auto iter = mLuminanceUpdates.begin(); iter != mLuminanceUpdates.end(); ++iter)
		{
			if (iter->commandBuffer->GetState() == CommandBufferState::Executing)
				break;

			lastFinishedIter = iter;
		}

		if (lastFinishedIter != mLuminanceUpdates.end())
		{
			// Get new luminance value
			mPreviousEyeAdaptation = mCurrentEyeAdaptation;

			PixelData data = lastFinishedIter->outputTexture->texture->Lock(GBL_READ_ONLY);
			mCurrentEyeAdaptation = data.GetColorAt(0, 0).r;
			lastFinishedIter->outputTexture->texture->Unlock();

			// We've received information about eye adaptation, use that to determine if redrawing
			// is required (technically we're drawing a few frames extra, as this information is always
			// a few frames too late).
			if (lastFinishedIter->frameIdx == mWaitingOnAutoExposureFrame)
				mWaitingOnAutoExposureFrame = std::numeric_limits<UINT64>::max();

			mLuminanceUpdates.erase(mLuminanceUpdates.begin(), lastFinishedIter + 1);
		}
	}
	
	RendererViewRedrawReason RendererView::GetRedrawReason() const
	{
		if (!mProperties.onDemand)
			return RendererViewRedrawReason::PerFrame;

		if (mRedrawThisFrame)
			return RendererViewRedrawReason::OnDemandThisFrame;

		return RendererViewRedrawReason::OnDemandLingering;
	}

	float RendererView::GetCurrentExposure() const
	{
		if (mRenderSettings->enableAutoExposure)
			return mPreviousEyeAdaptation;

		return Math::Pow(2.0f, mRenderSettings->exposureScale);
	}

	void RendererView::_notifyLuminanceUpdated(UINT64 frameIdx, SPtr<CommandBuffer> cb, SPtr<PooledRenderTexture> texture) const
	{
		mLuminanceUpdates.emplace_back(frameIdx, cb, texture);
	}

	void RendererView::determineVisible(const Vector<RendererRenderable*>& renderables, const Vector<CullInfo>& cullInfos,
		Vector<bool>* visibility)
	{
		mVisibility.renderables.Clear();
		mVisibility.renderables.Resize(renderables.size(), false);

		if (!shouldDraw3D())
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
		mVisibility.particleSystems.Clear();
		mVisibility.particleSystems.Resize(particleSystems.size(), false);

		if (!shouldDraw3D())
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
		mVisibility.decals.Clear();
		mVisibility.decals.Resize(decals.size(), false);

		if (!shouldDraw3D())
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
				visibility->Assign(lights.size(), true);

			return;
		}

		Vector<bool>* perViewVisibility;
		if(lightType == LightType::Radial)
		{
			mVisibility.radialLights.Clear();
			mVisibility.radialLights.Resize(lights.size(), false);

			perViewVisibility = &mVisibility.radialLights;
		}
		else // Spot
		{
			mVisibility.spotLights.Clear();
			mVisibility.spotLights.Resize(lights.size(), false);

			perViewVisibility = &mVisibility.spotLights;
		}

		if (!shouldDraw3D())
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

	void RendererView::CalculateVisibility(const Vector<CullInfo>& cullInfos, Vector<bool>& visibility) const
	{
		UINT64 cameraLayers = mProperties.visibleLayers;
		const ConvexVolume& worldFrustum = mProperties.cullFrustum;
		const Vector3& worldCameraPosition = mProperties.viewOrigin;
		float baseCullDistance = mRenderSettings->cullDistance;

		for (UINT32 i = 0; i < (UINT32)cullInfos.size(); i++)
		{
			if ((cullInfos[i].layer & cameraLayers) == 0)
				continue;

			// Do distance culling
			const Sphere& boundingSphere = cullInfos[i].bounds.GetSphere();
			const Vector3& worldRenderablePosition = boundingSphere.GetCenter();

			float distanceToCameraSq = worldCameraPosition.SquaredDistance(worldRenderablePosition);
			float correctedCullDistance = cullInfos[i].cullDistanceFactor * baseCullDistance;
			float maxDistanceToCamera = correctedCullDistance + boundingSphere.GetRadius();

			if (distanceToCameraSq > maxDistanceToCamera * maxDistanceToCamera)
				continue;

			// Do frustum culling
			// Note: This is bound to be a bottleneck at some point. When it is ensure that intersect methods use vector
			// operations, as it is trivial to update them. Also consider spatial partitioning.
			if (worldFrustum.Intersects(boundingSphere))
			{
				// More precise with the box
				const AABox& boundingBox = cullInfos[i].bounds.GetBox();

				if (worldFrustum.Intersects(boundingBox))
					visibility[i] = true;
			}
		}
	}

	void RendererView::CalculateVisibility(const Vector<Sphere>& bounds, Vector<bool>& visibility) const
	{
		const ConvexVolume& worldFrustum = mProperties.cullFrustum;

		for (UINT32 i = 0; i < (UINT32)bounds.size(); i++)
		{
			if (worldFrustum.Intersects(bounds[i]))
				visibility[i] = true;
		}
	}

	void RendererView::CalculateVisibility(const Vector<AABox>& bounds, Vector<bool>& visibility) const
	{
		const ConvexVolume& worldFrustum = mProperties.cullFrustum;

		for (UINT32 i = 0; i < (UINT32)bounds.size(); i++)
		{
			if (worldFrustum.Intersects(bounds[i]))
				visibility[i] = true;
		}
	}

	void RendererView::QueueRenderElements(const SceneInfo& sceneInfo)
	{
		// Queue renderables
		for(UINT32 i = 0; i < (UINT32)sceneInfo.renderables.size(); i++)
		{
			if (!mVisibility.renderables[i])
				continue;

			const AABox& boundingBox = sceneInfo.renderableCullInfos[i].bounds.GetBox();
			const float distanceToCamera = (mProperties.viewOrigin - boundingBox.GetCenter()).length();

			bool needsVelocity = requiresVelocityWrites();
			for (auto& renderElem : sceneInfo.renderables[i]->elements)
			{
				UINT32 techniqueIdx;
				if (needsVelocity)
				{
					techniqueIdx = renderElem.writeVelocityTechniqueIdx != (UINT32)-1
						? renderElem.writeVelocityTechniqueIdx
						: renderElem.defaultTechniqueIdx;
				}
				else
					techniqueIdx = renderElem.defaultTechniqueIdx;

				ShaderFlags shaderFlags = renderElem.material->GetShader()->getFlags();

				// Note: I could keep renderables in multiple separate arrays, so I don't need to do the check here
				if (shaderFlags.IsSet(ShaderFlag::Transparent))
					mTransparentQueue->Add(&renderElem, distanceToCamera, techniqueIdx);
				else if (shaderFlags.IsSet(ShaderFlag::Forward))
					mForwardOpaqueQueue->Add(&renderElem, distanceToCamera, techniqueIdx);
				else
					mDeferredOpaqueQueue->Add(&renderElem, distanceToCamera, techniqueIdx);
			}
		}

		// Queue particle systems
		for(UINT32 i = 0; i < (UINT32)sceneInfo.particleSystems.size(); i++)
		{
			if (!mVisibility.particleSystems[i])
				continue;

			const ParticlesRenderElement& renderElem = sceneInfo.particleSystems[i].renderElement;
			if (!renderElem.IsValid())
				continue;

			const AABox& boundingBox = sceneInfo.particleSystemCullInfos[i].bounds.GetBox();
			const float distanceToCamera = (mProperties.viewOrigin - boundingBox.GetCenter()).length();

			ShaderFlags shaderFlags = renderElem.material->GetShader()->getFlags();

			if (shaderFlags.IsSet(ShaderFlag::Transparent))
				mTransparentQueue->Add(&renderElem, distanceToCamera, renderElem.defaultTechniqueIdx);
			else if (shaderFlags.IsSet(ShaderFlag::Forward))
				mForwardOpaqueQueue->Add(&renderElem, distanceToCamera, renderElem.defaultTechniqueIdx);
			else
				mDeferredOpaqueQueue->Add(&renderElem, distanceToCamera, renderElem.defaultTechniqueIdx);
		}

		// Queue decals
		const bool isMSAA = mProperties.target.numSamples > 1;
		for(UINT32 i = 0; i < (UINT32)sceneInfo.decals.size(); i++)
		{
			if (!mVisibility.decals[i])
				continue;

			const DecalRenderElement& renderElem = sceneInfo.decals[i].renderElement;

			// Note: I could keep renderables in multiple separate arrays, so I don't need to do the check here
			ShaderFlags shaderFlags = renderElem.material->GetShader()->getFlags();

			// Decals are only supported using deferred rendering
			if (shaderFlags.IsSetAny(ShaderFlag::Transparent | ShaderFlag::Forward))
				continue;

			const AABox& boundingBox = sceneInfo.decalCullInfos[i].bounds.GetBox();
			const float distanceToCamera = (mProperties.viewOrigin - boundingBox.GetCenter()).length();

			// Check if viewer is inside the decal volume

			// Extend the bounds slighty to cover the case when the viewer is outside, but the near plane is intersecting
			// the decal bounds. We need to be conservative since the material for rendering outside will not properly
			// render the inside of the decal volume.
			const bool isInside = boundingBox.Contains(mProperties.viewOrigin, mProperties.nearPlane * 3.0f);
			const UINT32* techniqueIndices = renderElem.techniqueIndices[(INT32)isInside];

			// No MSAA evaluation, or same value for all samples (no divergence between samples)
			mDecalQueue->add(&renderElem, distanceToCamera,
				techniqueIndices[(INT32)(isMSAA ? MSAAMode::Single : MSAAMode::None)]);

			// Evaluates all MSAA samples for pixels that are marked as divergent
			if(isMSAA)
				mDecalQueue->Add(&renderElem, distanceToCamera, techniqueIndices[(INT32)MSAAMode::Full]);
		}

		mForwardOpaqueQueue->Sort();
		mDeferredOpaqueQueue->Sort();
		mTransparentQueue->Sort();
		mDecalQueue->Sort();
	}

	Vector2 RendererView::GetDeviceZToViewZ(const Matrix4& projMatrix)
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

		const RenderAPICapabilities& caps = gCaps();

		float depthRange = caps.maxDepth - caps.minDepth;
		float minDepth = caps.minDepth;

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

	Vector2 RendererView::GetNDCZToViewZ(const Matrix4& projMatrix)
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

	Vector2 RendererView::GetNDCZToDeviceZ()
	{
		const RenderAPICapabilities& caps = gCaps();

		Vector2 ndcZToDeviceZ;
		ndcZToDeviceZ.x = 1.0f / (caps.maxDepth - caps.minDepth);
		ndcZToDeviceZ.y = -caps.minDepth;

		return ndcZToDeviceZ;
	}

	Matrix4 InvertProjectionMatrix(const Matrix4& mat)
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
			return mat.Inverse();
		}
	}

	void RendererView::UpdatePerViewBuffer()
	{
		Matrix4 viewProj = mProperties.projTransform * mProperties.viewTransform;
		Matrix4 invProj = invertProjectionMatrix(mProperties.projTransform);
		Matrix4 invView = mProperties.viewTransform.InverseAffine();
		Matrix4 invViewProj = invView * invProj;

		gPerCameraParamDef.gMatProj.Set(mParamBuffer, mProperties.projTransform);
		gPerCameraParamDef.gMatView.Set(mParamBuffer, mProperties.viewTransform);
		gPerCameraParamDef.gMatViewProj.Set(mParamBuffer, viewProj);
		gPerCameraParamDef.gMatInvViewProj.Set(mParamBuffer, invViewProj);
		gPerCameraParamDef.gMatInvProj.Set(mParamBuffer, invProj);
		gPerCameraParamDef.gMatPrevViewProj.Set(mParamBuffer, mProperties.prevViewProjTransform);

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
		
		gPerCameraParamDef.gMatScreenToWorld.Set(mParamBuffer, invViewProj * projZ);
		gPerCameraParamDef.gNDCToPrevNDC.Set(mParamBuffer, NDCToPrevNDC);
		gPerCameraParamDef.gViewDir.Set(mParamBuffer, mProperties.viewDirection);
		gPerCameraParamDef.gViewOrigin.Set(mParamBuffer, mProperties.viewOrigin);
		gPerCameraParamDef.gDeviceZToWorldZ.Set(mParamBuffer, getDeviceZToViewZ(mProperties.projTransform));
		gPerCameraParamDef.gNDCZToWorldZ.Set(mParamBuffer, getNDCZToViewZ(mProperties.projTransform));
		gPerCameraParamDef.gNDCZToDeviceZ.Set(mParamBuffer, getNDCZToDeviceZ());

		Vector2 NearFar(mProperties.nearPlane, mProperties.farPlane);
		gPerCameraParamDef.gNearFar.Set(mParamBuffer, nearFar);

		const Rect2I& viewRect = mProperties.target.viewRect;

		Vector4I viewportRect;
		viewportRect[0] = viewRect.x;
		viewportRect[1] = viewRect.y;
		viewportRect[2] = viewRect.width;
		viewportRect[3] = viewRect.height;

		gPerCameraParamDef.gViewportRectangle.Set(mParamBuffer, viewportRect);

		Vector4 ndcToUV = getNDCToUV();
		gPerCameraParamDef.gClipToUVScaleOffset.Set(mParamBuffer, ndcToUV);

		Vector4 uvToNDC(
			1.0f / ndcToUV.x,
			1.0f / ndcToUV.y,
			-ndcToUV.z / ndcToUV.x,
			-ndcToUV.w / ndcToUV.y);
		gPerCameraParamDef.gUVToClipScaleOffset.Set(mParamBuffer, uvToNDC);

		if (!mRenderSettings->enableLighting)
			gPerCameraParamDef.gAmbientFactor.Set(mParamBuffer, 100.0f);
		else
			gPerCameraParamDef.gAmbientFactor.Set(mParamBuffer, 0.0f);
	}

	Vector4 RendererView::GetNDCToUV() const
	{
		const RenderAPICapabilities& caps = gCaps();
		const Rect2I& viewRect = mProperties.target.viewRect;
		
		float halfWidth = viewRect.width * 0.5f;
		float halfHeight = viewRect.height * 0.5f;

		float rtWidth = mProperties.target.targetWidth != 0 ? (float)mProperties.target.targetWidth : 20.0f;
		float rtHeight = mProperties.target.targetHeight != 0 ? (float)mProperties.target.targetHeight : 20.0f;

		Vector4 ndcToUV;
		ndcToUV.x = halfWidth / rtWidth;
		ndcToUV.y = -halfHeight / rtHeight;
		ndcToUV.z = viewRect.x / rtWidth + (halfWidth + caps.horizontalTexelOffset) / rtWidth;
		ndcToUV.w = viewRect.y / rtHeight + (halfHeight + caps.verticalTexelOffset) / rtHeight;

		// Either of these flips the Y axis, but if they're both true they cancel out
		if ((caps.conventions.uvYAxis == Conventions::Axis::Up) ^ (caps.conventions.ndcYAxis == Conventions::Axis::Down))
			ndcToUV.y = -ndcToUV.y;

		return ndcToUV;
	}

	void RendererView::updateLightGrid(const VisibleLightData& visibleLightData,
		const VisibleReflProbeData& visibleReflProbeData)
	{
		mLightGrid.UpdateGrid(*this, visibleLightData, visibleReflProbeData, !mRenderSettings->enableLighting);
	}

	RendererViewGroup::RendererViewGroup(RendererView** views, UINT32 numViews, bool mainPass, UINT32 shadowMapSize)
		: MIsMainPass(mainPass), mShadowRenderer(shadowMapSize)
	{
		setViews(views, numViews);
	}

	void RendererViewGroup::SetViews(RendererView** views, UINT32 numViews)
	{
		mViews.Clear();

		for (UINT32 i = 0; i < numViews; i++)
		{
			mViews.push_back(views[i]);
			views[i]->_setViewIdx(i);
		}
	}

	void RendererViewGroup::DetermineVisibility(const SceneInfo& sceneInfo)
	{
		const auto numViews = (UINT32)mViews.size();

		// Early exit if no views render scene geometry
		bool anyViewsNeed3DDrawing = false;
		for (UINT32 i = 0; i < numViews; i++)
		{
			if (mViews[i]->ShouldDraw3D())
			{
				anyViewsNeed3DDrawing = true;
				break;
			}
		}

		if (!anyViewsNeed3DDrawing)
			return;

		// Calculate renderable visibility per view
		mVisibility.renderables.Resize(sceneInfo.renderables.size(), false);
		mVisibility.renderables.Assign(sceneInfo.renderables.size(), false);

		mVisibility.particleSystems.Resize(sceneInfo.particleSystems.size(), false);
		mVisibility.particleSystems.Assign(sceneInfo.particleSystems.size(), false);

		mVisibility.decals.Resize(sceneInfo.decals.size(), false);
		mVisibility.decals.Assign(sceneInfo.decals.size(), false);

		for(UINT32 i = 0; i < numViews; i++)
		{
			mViews[i]->DetermineVisible(sceneInfo.renderables, sceneInfo.renderableCullInfos, &mVisibility.renderables);
			mViews[i]->DetermineVisible(sceneInfo.particleSystems, sceneInfo.particleSystemCullInfos, &mVisibility.particleSystems);
			mViews[i]->DetermineVisible(sceneInfo.decals, sceneInfo.decalCullInfos, &mVisibility.decals);
		}
		
		// Generate render queues per camera
		for (UINT32 i = 0; i < numViews; i++)
		{
			if(mViews[i]->ShouldDraw3D())
				mViews[i]->QueueRenderElements(sceneInfo);
		}

		// Calculate light visibility for all views
		const auto numRadialLights = (UINT32)sceneInfo.radialLights.size();
		mVisibility.radialLights.Resize(numRadialLights, false);
		mVisibility.radialLights.Assign(numRadialLights, false);

		const auto numSpotLights = (UINT32)sceneInfo.spotLights.size();
		mVisibility.spotLights.Resize(numSpotLights, false);
		mVisibility.spotLights.Assign(numSpotLights, false);

		for (UINT32 i = 0; i < numViews; i++)
		{
			if (!mViews[i]->ShouldDraw3D())
				continue;

			mViews[i]->determineVisible(sceneInfo.radialLights, sceneInfo.radialLightWorldBounds, LightType::Radial,
				&mVisibility.radialLights);

			mViews[i]->determineVisible(sceneInfo.spotLights, sceneInfo.spotLightWorldBounds, LightType::Spot,
				&mVisibility.spotLights);
		}

		// Calculate refl. probe visibility for all views
		const auto numProbes = (UINT32)sceneInfo.reflProbes.size();
		mVisibility.reflProbes.Resize(numProbes, false);
		mVisibility.reflProbes.Assign(numProbes, false);

		// Note: Per-view visibility for refl. probes currently isn't calculated
		for (UINT32 i = 0; i < numViews; i++)
		{
			const auto& viewProps = mViews[i]->GetProperties();

			// Don't recursively render reflection probes when generating reflection probe maps
			if (viewProps.capturingReflections)
				continue;

			mViews[i]->CalculateVisibility(sceneInfo.reflProbeWorldBounds, mVisibility.reflProbes);
		}

		// Organize light and refl. probe visibility information in a more GPU friendly manner

		// Note: I'm determining light and refl. probe visibility for the entire group. It might be more performance
		// efficient to do it per view. Additionally I'm using a single GPU buffer to hold their information, which is
		// then updated when each view group is rendered. It might be better to keep one buffer reserved per-view.
		mVisibleLightData.Update(sceneInfo, *this);
		mVisibleReflProbeData.Update(sceneInfo, *this);

		const bool supportsClusteredForward = gRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
		if(supportsClusteredForward)
		{
			for (UINT32 i = 0; i < numViews; i++)
			{
				if (!mViews[i]->ShouldDraw3D())
					continue;

				mViews[i]->UpdateLightGrid(mVisibleLightData, mVisibleReflProbeData);
			}
		}
	}
}}
