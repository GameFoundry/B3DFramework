//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DRendererView.h"
#include "Components/B3DCamera.h"
#include "Components/B3DRenderable.h"
#include "Renderer/B3DRendererUtility.h"
#include "Material/B3DMaterial.h"
#include "Material/B3DShader.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "RenderState/B3DLightRenderState.h"
#include "B3DRenderBeastScene.h"
#include "B3DRenderBeast.h"
#include "RenderState/B3DDecalRenderState.h"
#include "Animation/B3DAnimationScene.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DRenderTarget.h"
#include "Image/B3DTexture.h"

namespace b3d {
namespace render {

PerCameraUniformDefinition gPerCameraUniformDefinition;
SkyboxUniformDefinition gSkyboxUniformDefinition;

void SkyboxMaterial::Initialize()
{
	if(mGpuParameterSet->HasSampledTexture("gSkyTex"))
		mGpuParameterSet->GetSampledTextureParameter("gSkyTex", mSkyTextureParameter);

	mGpuParameterSet->TryGetUniformBufferParameter("Params", mUniformBufferParameter);
}

void SkyboxMaterial::Bind(GpuCommandBuffer& commandBuffer, const GpuBufferSuballocation& perCamera, const TShared<Texture>& texture, const Color& solidColor)
{
	mGpuParameterSet->SetUniformBuffer("PerCamera", perCamera);

	GpuBufferMappedScope uniforms = gSkyboxUniformDefinition.AllocateTransient().Map();
	gSkyboxUniformDefinition.gClearColor.Set(uniforms, solidColor);

	mUniformBufferParameter.Set(uniforms);
	mSkyTextureParameter.Set(texture);

	RendererMaterial::Bind(commandBuffer);
}

SkyboxMaterial* SkyboxMaterial::GetVariation(bool color)
{
	if(color)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

RendererViewInformation::RendererViewInformation()
	: EncodeDepth(false)
{
}

RendererViewProperties::RendererViewProperties(const RendererViewCreateInformation& src)
	: RendererViewInformation(src), FrameIdx(0), Target(src.Target)
{
	ProjTransformNoAa = src.ProjTransform;
	ViewProjTransform = src.ProjTransform * src.ViewTransform;
}

RendererView::RendererView()
	: mCamera(nullptr), mRenderSettingsHash(0), mViewIdx(-1)
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	mPerCameraBufferPool.Initialize(*gpuDevice, GpuBufferCreateInformation::CreateUniform(gPerCameraUniformDefinition.GetSize()), 4);
}

RendererView::RendererView(const RendererViewCreateInformation& desc)
	: mProperties(desc), mCamera(desc.SceneCamera), mRenderSettingsHash(0), mViewIdx(-1)
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	mPerCameraBufferPool.Initialize(*gpuDevice, GpuBufferCreateInformation::CreateUniform(gPerCameraUniformDefinition.GetSize()), 4);
	mProperties.PrevViewProjTransform = mProperties.ViewProjTransform;

	SetStateReductionMode(desc.StateReduction);
}

RendererView::~RendererView()
{
	if(mPerCameraBuffer.IsValid())
		mPerCameraBufferPool.Release(mPerCameraBuffer);
}

void RendererView::SetStateReductionMode(StateReduction reductionMode)
{
	mDeferredOpaqueQueue = B3DMakeShared<RenderQueue>(reductionMode);
	mForwardOpaqueQueue = B3DMakeShared<RenderQueue>(reductionMode);

	StateReduction transparentStateReduction = reductionMode;
	if(transparentStateReduction == StateReduction::Material)
		transparentStateReduction = StateReduction::Distance; // Transparent object MUST be sorted by distance

	mTransparentQueue = B3DMakeShared<RenderQueue>(transparentStateReduction);
	mDecalQueue = B3DMakeShared<RenderQueue>(StateReduction::Material);
}

void RendererView::SetRenderSettings(const TShared<RenderSettings>& settings)
{
	if(mRenderSettings == nullptr)
		mRenderSettings = B3DMakeShared<RenderSettings>();

	if(settings != nullptr)
		*mRenderSettings = *settings;

	mRenderSettingsHash++;

	// Update compositor hierarchy (Note: Needs to be called even when viewport size (or other information) changes,
	// but we're currently calling it here as all such calls are followed by setRenderSettings.
	TArray<StringID> primaryNodes;
	primaryNodes.Reserve(32);

	// Primary rendering (base pass, lighting)
	primaryNodes.Add(RCNodeParticleSort::GetNodeId());
	primaryNodes.Add(RCNodeBasePass::GetNodeId());
	primaryNodes.Add(RCNodeSSAO::GetNodeId());
	primaryNodes.Add(RCNodeDeferredDirectLighting::GetNodeId());
	primaryNodes.Add(RCNodeIndirectDiffuseLighting::GetNodeId());
	primaryNodes.Add(RCNodeSSR::GetNodeId());
	primaryNodes.Add(RCNodeDeferredIndirectSpecularLighting::GetNodeId());
	primaryNodes.Add(RCNodeSkybox::GetNodeId());
	primaryNodes.Add(RCNodeParticleSimulate::GetNodeId());	
	primaryNodes.Add(RCNodeClusteredForward::GetNodeId());

	// Screen space
	if (mProperties.RunPostProcessing)
	{
		primaryNodes.Add(RCNodeTemporalAA::GetNodeId());
		primaryNodes.Add(RCNodeBokehDOF::GetNodeId());
		primaryNodes.Add(RCNodeMotionBlur::GetNodeId());

		if (mRenderSettings->Bloom.Enabled || mRenderSettings->ScreenSpaceLensFlare.Enabled)
			primaryNodes.Add(RCNodeBloom::GetNodeId());

		if (mRenderSettings->ScreenSpaceLensFlare.Enabled)
			primaryNodes.Add(RCNodeScreenSpaceLensFlare::GetNodeId());

		primaryNodes.Add(RCNodeTonemapping::GetNodeId());
		primaryNodes.Add(RCNodeGaussianDOF::GetNodeId());
		primaryNodes.Add(RCNodeFXAA::GetNodeId());
		primaryNodes.Add(RCNodeChromaticAberration::GetNodeId());
		primaryNodes.Add(RCNodeFilmGrain::GetNodeId());
	}

	primaryNodes.Add(RCNodeFinalResolve::GetNodeId());

	mCompositor.Build(*this, primaryNodes);
}

void RendererView::SetTransform(const Vector3& origin, const Vector3& direction, const Matrix4& view, const Matrix4& proj, const ConvexVolume& worldFrustum)
{
	mProperties.ViewOrigin = origin;
	mProperties.ViewDirection = direction;
	mProperties.ViewTransform = view;
	mProperties.ProjTransform = proj;
	mProperties.ProjTransformNoAa = proj;
	mProperties.CullFrustum = worldFrustum;
	mProperties.ViewProjTransform = proj * view;
	mProperties.TemporalJitter = Vector2::kZero;
}

void RendererView::SetView(const RendererViewCreateInformation& desc)
{
	mCamera = desc.SceneCamera;
	mProperties = desc;
	mProperties.ProjTransformNoAa = desc.ProjTransform;
	mProperties.ViewProjTransform = desc.ProjTransform * desc.ViewTransform;
	mProperties.PrevViewProjTransform = Matrix4::kIdentity;
	mProperties.Target = desc.Target;
	mProperties.TemporalJitter = Vector2::kZero;

	SetStateReductionMode(desc.StateReduction);
}

void RendererView::BeginFrame(const FrameInfo& frameInfo)
{
	// Check if render target resized and update the view properties accordingly
	// Note: Normally we rely on the renderer notify* methods to let us know of changes to camera/viewport, but since
	// render target resize can often originate from the render thread, this avoids the back and forth between
	// main <-> render thread, and the frame delay that comes with it
	bool perViewBufferDirty = false;
	if(mCamera)
	{
		const TShared<Viewport>& viewport = mCamera->GetViewport();
		if(viewport)
		{
			u32 newTargetWidth = 0;
			u32 newTargetHeight = 0;
			if(mProperties.Target.Target != nullptr)
			{
				newTargetWidth = mProperties.Target.Target->GetProperties().Width;
				newTargetHeight = mProperties.Target.Target->GetProperties().Height;
			}

			if(newTargetWidth != mProperties.Target.TargetWidth ||
			   newTargetHeight != mProperties.Target.TargetHeight)
			{
				mProperties.Target.ViewRect = viewport->GetPixelArea();
				mProperties.Target.TargetWidth = newTargetWidth;
				mProperties.Target.TargetHeight = newTargetHeight;

				perViewBufferDirty = true;
			}
		}
	}

	// Update projection matrix jitter if temporal AA is enabled
	if(mRenderSettings->TemporalAa.Enabled)
	{
		u32 positionCount = mRenderSettings->TemporalAa.JitteredPositionCount;
		positionCount = Math::Clamp(positionCount, 4U, 128U);

		u32 positionIndex = mTemporalPositionIdx % positionCount;

		if(positionCount == 4)
		{
			// Using a 4x MSAA pattern: http://msdn.microsoft.com/en-us/library/windows/desktop/ff476218(v=vs.85).aspx
			Vector2 samples[] = {
				{ -2.0f / 16.0f, -6.0f / 16.0f },
				{ 6.0f / 16.0f, -2.0f / 16.0f },
				{ 2.0f / 16.0f, 6.0f / 16.0f },
				{ -6.0f / 16.0f, 2.0f / 16.0f }
			};

			mProperties.TemporalJitter = samples[positionIndex];
		}
		else
		{
			constexpr float EPSILON = 1e-6f;

			float u1 = Math::HaltonSequence<float>(positionIndex + 1, 2);
			float u2 = Math::HaltonSequence<float>(positionIndex + 1, 3);

			float scale = (2.0f - mRenderSettings->TemporalAa.Sharpness) * 0.3f;

			float angle = 2.0f * Math::kPi * u2;
			float radius = scale * Math::SquareRoot(-2.0f * Math::Log(Math::Max(u1, EPSILON)));

			mProperties.TemporalJitter = Vector2(radius * Math::Cos(angle), radius * Math::Sin(angle));
		}

		Vector2 viewSize = Vector2((float)mProperties.Target.TargetWidth, (float)mProperties.Target.TargetHeight);
		Vector2 subsampleJitter = mProperties.TemporalJitter / viewSize;
		Matrix4 subSampleTranslate = Matrix4::Translation(Vector3(subsampleJitter.X, subsampleJitter.Y, 0.0f));

		mProperties.ProjTransform = subSampleTranslate * mProperties.ProjTransformNoAa;
		mProperties.ViewProjTransform = mProperties.ProjTransform * mProperties.ViewTransform;

		mTemporalPositionIdx++;
		perViewBufferDirty = true;
	}

	// Note: We're updating the buffer every frame because certain properties depend on previous view/projection data. Ideally we store those in a separate buffer
	// we can update
	(void)(perViewBufferDirty);
	//if(perViewBufferDirty)
		UpdatePerViewBuffer();

	mFrameTimings = frameInfo.Timings;
	mAsyncAnim = frameInfo.IsUsingAsynchronousAnimation;

	// Account for auto-exposure taking multiple frames
	if(mRedrawThisFrame)
	{
		// Note: Doing this here instead of _notifyNeedsRedraw because we need an up-to-date frame index
		if(mRenderSettings->EnableHdr && mRenderSettings->EnableAutoExposure)
			mWaitingOnAutoExposureFrame = mFrameTimings.FrameIndex;
		else
			mWaitingOnAutoExposureFrame = std::numeric_limits<u64>::max();
	}
}

void RendererView::EndFrame()
{
	// Save view-projection matrix to use for temporal filtering
	mProperties.PrevViewProjTransform = mProperties.ViewProjTransform;

	// Advance per-view frame index. This is used primarily by temporal rendering effects, and pausing the frame index
	// allows you to freeze the current rendering as is, without temporal artifacts.
	mProperties.FrameIdx++;

	mDeferredOpaqueQueue->Clear();
	mForwardOpaqueQueue->Clear();
	mTransparentQueue->Clear();
	mDecalQueue->Clear();

	if(mRedrawForFrames > 0)
		mRedrawForFrames--;

	if(mRedrawForSeconds > 0.0f)
		mRedrawForSeconds -= mFrameTimings.TimeDelta;

	mRedrawThisFrame = false;
}

void RendererView::NotifyNeedsRedraw()
{
	mRedrawThisFrame = true;

	// If doing async animation there is a one frame delay
	mRedrawForFrames = mAsyncAnim ? 2 : 1;

	// This will be set once we get the new luminance data from the GPU
	mRedrawForSeconds = 0.0f;
}

bool RendererView::ShouldDraw() const
{
	if(!mProperties.OnDemand)
		return true;

	if(mRenderSettings->EnableHdr && mRenderSettings->EnableAutoExposure)
	{
		constexpr float kAutoExposureTolerance = 0.01f;

		// The view was redrawn but we still haven't received the eye adaptation results from the GPU, so
		// we keep redrawing until we do
		if(mWaitingOnAutoExposureFrame != std::numeric_limits<u64>::max())
			return true;

		// Need to render until the auto-exposure reaches the target exposure
		float eyeAdaptationDiff = Math::Abs(mCurrentEyeAdaptation - mPreviousEyeAdaptation);
		if(eyeAdaptationDiff > kAutoExposureTolerance)
			return true;
	}

	return mRedrawForFrames > 0 || mRedrawForSeconds > 0.0f;
}

bool RendererView::RequiresVelocityWrites() const
{
	return mRenderSettings->TemporalAa.Enabled || mRenderSettings->EnableVelocityBuffer;
}

void RendererView::UpdateAsyncOperations()
{
	// Find most recent available frame
	auto lastFinishedIter = mLuminanceUpdates.end();
	for(auto iter = mLuminanceUpdates.begin(); iter != mLuminanceUpdates.end(); ++iter)
	{
		if(!iter->ReadbackAsyncOp.HasCompleted())
			break;

		lastFinishedIter = iter;
	}

	if(lastFinishedIter != mLuminanceUpdates.end())
	{
		// Get new luminance value
		mPreviousEyeAdaptation = mCurrentEyeAdaptation;

		const TShared<PixelData> pixelData = lastFinishedIter->ReadbackAsyncOp.GetReturnValue();
		mCurrentEyeAdaptation = pixelData->GetColorAt(0, 0).R;

		// We've received information about eye adaptation, use that to determine if redrawing
		// is required (technically we're drawing a few frames extra, as this information is always
		// a few frames too late).
		if(lastFinishedIter->FrameIdx == mWaitingOnAutoExposureFrame)
			mWaitingOnAutoExposureFrame = std::numeric_limits<u64>::max();

		mLuminanceUpdates.erase(mLuminanceUpdates.begin(), lastFinishedIter + 1);
	}
}

RendererViewRedrawReason RendererView::GetRedrawReason() const
{
	if(!mProperties.OnDemand)
		return RendererViewRedrawReason::PerFrame;

	if(mRedrawThisFrame)
		return RendererViewRedrawReason::OnDemandThisFrame;

	return RendererViewRedrawReason::OnDemandLingering;
}

float RendererView::GetCurrentExposure() const
{
	if(mRenderSettings->EnableAutoExposure)
		return mPreviousEyeAdaptation;

	return Math::RaiseToPower(2.0f, mRenderSettings->ExposureScale);
}

void RendererView::NotifyLuminanceUpdated(u64 frameIdx, TShared<GpuCommandBuffer> cb, TShared<PooledRenderTexture> texture) const
{
	if(cb == nullptr)
	{
		B3D_LOG(Error, LogRenderer, "Cannot queue luminance update. (Null command buffer provided.)");
		return;
	}

	TAsyncOp<TShared<PixelData>> readbackAsyncOp = TextureUtility::ReadAsync(texture->Texture, *cb);
	mLuminanceUpdates.emplace_back(frameIdx, std::move(readbackAsyncOp), std::move(texture));
}

void RendererView::DetermineVisible(const TChunkedArray<RenderableRenderState*>& renderables, const TChunkedArray<CullInfo>& cullInfos, Vector<bool>* visibility)
{
	mVisibility.Renderables.clear();
	mVisibility.Renderables.resize(renderables.size(), false);

	if(!ShouldDraw3D())
		return;

	CalculateVisibility(cullInfos, mVisibility.Renderables);

	if(visibility != nullptr)
	{
		for(u32 i = 0; i < (u32)renderables.size(); i++)
		{
			bool visible = (*visibility)[i];

			(*visibility)[i] = visible || mVisibility.Renderables[i];
		}
	}
}

void RendererView::DetermineVisible(const TChunkedArray<ParticleRenderState>& particleSystems, const TChunkedArray<CullInfo>& cullInfos, Vector<bool>* visibility)
{
	mVisibility.ParticleSystems.clear();
	mVisibility.ParticleSystems.resize(particleSystems.size(), false);

	if(!ShouldDraw3D())
		return;

	CalculateVisibility(cullInfos, mVisibility.ParticleSystems);

	if(visibility != nullptr)
	{
		for(u32 i = 0; i < (u32)particleSystems.size(); i++)
		{
			bool visible = (*visibility)[i];

			(*visibility)[i] = visible || mVisibility.ParticleSystems[i];
		}
	}
}

void RendererView::DetermineVisible(const TChunkedArray<DecalRenderState>& decals, const TChunkedArray<CullInfo>& cullInfos, Vector<bool>* visibility)
{
	mVisibility.Decals.clear();
	mVisibility.Decals.resize(decals.size(), false);

	if(!ShouldDraw3D())
		return;

	CalculateVisibility(cullInfos, mVisibility.Decals);

	if(visibility != nullptr)
	{
		for(u32 i = 0; i < (u32)decals.size(); i++)
		{
			bool visible = (*visibility)[i];

			(*visibility)[i] = visible || mVisibility.Decals[i];
		}
	}
}

void RendererView::DetermineVisible(TArrayView<const Sphere> bounds, LightType lightType, Vector<bool>* outVisibility)
{
	const u32 lightCount = (u32)bounds.size();

	// Special case for directional lights, they're always visible
	if(lightType == LightType::Directional)
	{
		if(outVisibility)
			outVisibility->assign(lightCount, true);

		return;
	}

	Vector<bool>* perViewVisibility;
	if(lightType == LightType::Radial)
	{
		mVisibility.RadialLights.clear();
		mVisibility.RadialLights.resize(lightCount, false);

		perViewVisibility = &mVisibility.RadialLights;
	}
	else // Spot
	{
		mVisibility.SpotLights.clear();
		mVisibility.SpotLights.resize(lightCount, false);

		perViewVisibility = &mVisibility.SpotLights;
	}

	if(!ShouldDraw3D())
		return;

	CalculateVisibility(bounds, *perViewVisibility);

	if(outVisibility != nullptr)
	{
		for(u32 i = 0; i < lightCount; i++)
		{
			bool visible = (*outVisibility)[i];

			(*outVisibility)[i] = visible || (*perViewVisibility)[i];
		}
	}
}

template<typename CullInfoContainer>
static void CalculateVisibilityCullInfo(const RendererViewProperties& properties, const RenderSettings& renderSettings, const CullInfoContainer& cullInfos, Vector<bool>& visibility)
{
	u64 cameraLayers = properties.VisibleLayers;
	const ConvexVolume& worldFrustum = properties.CullFrustum;
	const Vector3& worldCameraPosition = properties.ViewOrigin;
	float baseCullDistance = renderSettings.CullDistance;

	for(u32 i = 0; i < (u32)cullInfos.size(); i++)
	{
		if((cullInfos[i].Layer & cameraLayers) == 0)
			continue;

		// Do distance culling
		const Sphere& boundingSphere = cullInfos[i].Bounds.GetSphere();
		const Vector3& worldRenderablePosition = boundingSphere.Center;

		float distanceToCameraSq = worldCameraPosition.SquaredDistance(worldRenderablePosition);
		float correctedCullDistance = cullInfos[i].CullDistanceFactor * baseCullDistance;
		float maxDistanceToCamera = correctedCullDistance + boundingSphere.Radius;

		if(distanceToCameraSq > maxDistanceToCamera * maxDistanceToCamera)
			continue;

		// Do frustum culling
		// Note: This is bound to be a bottleneck at some point. When it is ensure that intersect methods use vector
		// operations, as it is trivial to update them. Also consider spatial partitioning.
		if(worldFrustum.Intersects(boundingSphere))
		{
			// More precise with the box
			const AABox& boundingBox = cullInfos[i].Bounds.GetBox();

			if(worldFrustum.Intersects(boundingBox))
				visibility[i] = true;
		}
	}
}

void RendererView::CalculateVisibility(TArrayView<const CullInfo> cullInfos, Vector<bool>& visibility) const
{
	CalculateVisibilityCullInfo(mProperties, *mRenderSettings, cullInfos, visibility);
}

void RendererView::CalculateVisibility(const TChunkedArray<CullInfo>& cullInfos, Vector<bool>& visibility) const
{
	CalculateVisibilityCullInfo(mProperties, *mRenderSettings, cullInfos, visibility);
}

template<typename SphereContainer>
static void CalculateVisibilitySphere(const ConvexVolume& worldFrustum, const SphereContainer& bounds, Vector<bool>& visibility)
{
	for(u32 i = 0; i < (u32)bounds.size(); i++)
	{
		if(worldFrustum.Intersects(bounds[i]))
			visibility[i] = true;
	}
}

void RendererView::CalculateVisibility(TArrayView<const Sphere> bounds, Vector<bool>& visibility) const
{
	CalculateVisibilitySphere(mProperties.CullFrustum, bounds, visibility);
}

void RendererView::CalculateVisibility(const TChunkedArray<Sphere>& bounds, Vector<bool>& visibility) const
{
	CalculateVisibilitySphere(mProperties.CullFrustum, bounds, visibility);
}

void RendererView::CalculateVisibility(const Vector<AABox>& bounds, Vector<bool>& visibility) const
{
	const ConvexVolume& worldFrustum = mProperties.CullFrustum;

	for(u32 i = 0; i < (u32)bounds.size(); i++)
	{
		if(worldFrustum.Intersects(bounds[i]))
			visibility[i] = true;
	}
}

void RendererView::QueueDrawCommands(const RenderBeastScene& scene)
{
	B3D_ENSURE(mDeferredOpaqueQueue->GetSortedEntries().empty());
	B3D_ENSURE(mForwardOpaqueQueue->GetSortedEntries().empty());
	B3D_ENSURE(mDecalQueue->GetSortedEntries().empty());
	B3D_ENSURE(mTransparentQueue->GetSortedEntries().empty());

	// Queue renderables
	for(u32 i = 0; i < scene.GetRenderableCount(); i++)
	{
		if(!mVisibility.Renderables[i])
			continue;

		const AABox& boundingBox = scene.GetRenderableCullInfo(i).Bounds.GetBox();
		const float distanceToCamera = (mProperties.ViewOrigin - boundingBox.GetCenter()).Length();

		bool needsVelocity = RequiresVelocityWrites();
		for(auto& drawCommand : scene.GetRenderable(i)->DrawCommands)
		{
			u32 variationIndex;
			if(needsVelocity)
			{
				variationIndex = drawCommand.WriteVelocityVariationIndex != (u32)-1
					? drawCommand.WriteVelocityVariationIndex
					: drawCommand.DefaultVariationIndex;
			}
			else
				variationIndex = drawCommand.DefaultVariationIndex;

			ShaderFlags shaderFlags = drawCommand.Material->GetShader()->GetFlags();

			// Note: I could keep renderables in multiple separate arrays, so I don't need to do the check here
			if(shaderFlags.IsSet(ShaderFlag::Transparent))
				mTransparentQueue->Add(&drawCommand, distanceToCamera, variationIndex);
			else if(shaderFlags.IsSet(ShaderFlag::Forward))
				mForwardOpaqueQueue->Add(&drawCommand, distanceToCamera, variationIndex);
			else
				mDeferredOpaqueQueue->Add(&drawCommand, distanceToCamera, variationIndex);
		}
	}

	// Queue particle systems
	for(u32 i = 0; i < scene.GetParticleSystemCount(); i++)
	{
		if(!mVisibility.ParticleSystems[i])
			continue;

		const ParticlesDrawCommand& drawCommand = scene.GetParticleRenderState(i).DrawCommand;
		if(!drawCommand.IsValid())
			continue;

		const AABox& boundingBox = scene.GetParticleSystemCullInfo(i).Bounds.GetBox();
		const float distanceToCamera = (mProperties.ViewOrigin - boundingBox.GetCenter()).Length();

		ShaderFlags shaderFlags = drawCommand.Material->GetShader()->GetFlags();

		if(shaderFlags.IsSet(ShaderFlag::Transparent))
			mTransparentQueue->Add(&drawCommand, distanceToCamera, drawCommand.DefaultVariationIndex);
		else if(shaderFlags.IsSet(ShaderFlag::Forward))
			mForwardOpaqueQueue->Add(&drawCommand, distanceToCamera, drawCommand.DefaultVariationIndex);
		else
			mDeferredOpaqueQueue->Add(&drawCommand, distanceToCamera, drawCommand.DefaultVariationIndex);
	}

	// Queue decals
	const bool isMSAA = mProperties.Target.NumSamples > 1;
	for(u32 i = 0; i < (u32)scene.GetDecalCount(); i++)
	{
		if(!mVisibility.Decals[i])
			continue;

		const DecalDrawCommand& drawCommand = scene.GetDecalRenderState(i).DrawCommand;

		// Note: I could keep renderables in multiple separate arrays, so I don't need to do the check here
		ShaderFlags shaderFlags = drawCommand.Material->GetShader()->GetFlags();

		// Decals are only supported using deferred rendering
		if(shaderFlags.IsSetAny(ShaderFlag::Transparent | ShaderFlag::Forward))
			continue;

		const AABox& boundingBox = scene.GetDecalCullInfo(i).Bounds.GetBox();
		const float distanceToCamera = (mProperties.ViewOrigin - boundingBox.GetCenter()).Length();

		// Check if viewer is inside the decal volume

		// Extend the bounds slighty to cover the case when the viewer is outside, but the near plane is intersecting
		// the decal bounds. We need to be conservative since the material for rendering outside will not properly
		// render the inside of the decal volume.
		const bool isInside = boundingBox.Contains(mProperties.ViewOrigin, mProperties.NearPlane * 3.0f);
		const u32* variationIndices = drawCommand.VariationIndices[(i32)isInside];

		// No MSAA evaluation, or same value for all samples (no divergence between samples)
		mDecalQueue->Add(&drawCommand, distanceToCamera, variationIndices[(i32)(isMSAA ? MSAAMode::Single : MSAAMode::None)]);

		// Evaluates all MSAA samples for pixels that are marked as divergent
		if(isMSAA)
			mDecalQueue->Add(&drawCommand, distanceToCamera, variationIndices[(i32)MSAAMode::Full]);
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

	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuDeviceCapabilities& gpuDeviceCapabilities = gpuDevice->GetCapabilities();

	float depthRange = gpuDeviceCapabilities.MaxDepth - gpuDeviceCapabilities.MinDepth;
	float minDepth = gpuDeviceCapabilities.MinDepth;

	float a = projMatrix[2][2];
	float b = projMatrix[2][3];
	float c = projMatrix[3][2];

	Vector2 output;

	if(c != 0.0f)
	{
		output.X = b / (depthRange * c);
		output.Y = minDepth / depthRange - a / (depthRange * c);
	}
	else // Ortographic, assuming viewing towards negative Z
	{
		output.X = b / -depthRange;
		output.Y = minDepth / depthRange - a / -depthRange;
	}

	return output;
}

Vector2 RendererView::GetNdczToViewZ(const Matrix4& projMatrix)
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

	if(c != 0.0f)
	{
		output.X = b / c;
		output.Y = -a / c;
	}
	else // Ortographic, assuming viewing towards negative Z
	{
		output.X = -b;
		output.Y = a;
	}

	return output;
}

Vector2 RendererView::GetNdczToDeviceZ()
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuDeviceCapabilities& gpuDeviceCapabilities = gpuDevice->GetCapabilities();

	Vector2 ndcZToDeviceZ;
	ndcZToDeviceZ.X = 1.0f / (gpuDeviceCapabilities.MaxDepth - gpuDeviceCapabilities.MinDepth);
	ndcZToDeviceZ.Y = -gpuDeviceCapabilities.MinDepth;

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
			(float)(1.0 / a), 0.0f, 0.0f, (float)(-s / a),
			0.0f, (float)(1.0 / b), 0.0f, (float)(-t / b),
			0.0f, 0.0f, 0.0f, -1.0f,
			0.0f, 0.0f, (float)(1.0 / d), (float)(c / d));
	}
	else
	{
		return mat.Inverse();
	}
}

void RendererView::UpdatePerViewBuffer()
{
	// Release the previous buffer allocation (if any) and allocate a new one
	if(mPerCameraBuffer.IsValid())
		mPerCameraBufferPool.Release(mPerCameraBuffer);

	mPerCameraBuffer = mPerCameraBufferPool.Allocate();
	GpuBufferMappedScope uniforms = mPerCameraBuffer.Map();

	Matrix4 viewProj = mProperties.ProjTransform * mProperties.ViewTransform;
	Matrix4 invProj = InvertProjectionMatrix(mProperties.ProjTransform);
	Matrix4 invView = mProperties.ViewTransform.InverseAffine();
	Matrix4 invViewProj = invView * invProj;

	gPerCameraUniformDefinition.gMatProj.Set(uniforms, mProperties.ProjTransform);
	gPerCameraUniformDefinition.gMatView.Set(uniforms, mProperties.ViewTransform);
	gPerCameraUniformDefinition.gMatViewProj.Set(uniforms, viewProj);
	gPerCameraUniformDefinition.gMatInvViewProj.Set(uniforms, invViewProj);
	gPerCameraUniformDefinition.gMatInvProj.Set(uniforms, invProj);
	gPerCameraUniformDefinition.gMatPrevViewProj.Set(uniforms, mProperties.PrevViewProjTransform);

	// Construct a special inverse view-projection matrix that had projection entries that effect z and w eliminated.
	// Used to transform a vector(clip_x, clip_y, view_z, view_w), where clip_x/clip_y are in clip space, and
	// view_z/view_w in view space, into world space.

	// Only projects z/w coordinates (cancels out with the inverse matrix below)
	Matrix4 projZ = Matrix4::kIdentity;
	projZ[2][2] = mProperties.ProjTransform[2][2];
	projZ[2][3] = mProperties.ProjTransform[2][3];
	projZ[3][2] = mProperties.ProjTransform[3][2];
	projZ[3][3] = 0.0f;

	Matrix4 NDCToPrevNDC = mProperties.PrevViewProjTransform * invViewProj;

	gPerCameraUniformDefinition.gMatScreenToWorld.Set(uniforms, invViewProj * projZ);
	gPerCameraUniformDefinition.gNDCToPrevNDC.Set(uniforms, NDCToPrevNDC);
	gPerCameraUniformDefinition.gViewDir.Set(uniforms, mProperties.ViewDirection);
	gPerCameraUniformDefinition.gViewOrigin.Set(uniforms, mProperties.ViewOrigin);
	gPerCameraUniformDefinition.gDeviceZToWorldZ.Set(uniforms, GetDeviceZToViewZ(mProperties.ProjTransform));
	gPerCameraUniformDefinition.gNDCZToWorldZ.Set(uniforms, GetNdczToViewZ(mProperties.ProjTransform));
	gPerCameraUniformDefinition.gNDCZToDeviceZ.Set(uniforms, GetNdczToDeviceZ());

	Vector2 nearFar(mProperties.NearPlane, mProperties.FarPlane);
	gPerCameraUniformDefinition.gNearFar.Set(uniforms, nearFar);

	const Area2I& viewRect = mProperties.Target.ViewRect;

	Vector4I viewportRect;
	viewportRect[0] = viewRect.X;
	viewportRect[1] = viewRect.Y;
	viewportRect[2] = viewRect.Width;
	viewportRect[3] = viewRect.Height;

	gPerCameraUniformDefinition.gViewportRectangle.Set(uniforms, viewportRect);

	Vector4 ndcToUV = GetNdcToUv();
	gPerCameraUniformDefinition.gClipToUVScaleOffset.Set(uniforms, ndcToUV);

	Vector4 uvToNDC(
		1.0f / ndcToUV.X,
		1.0f / ndcToUV.Y,
		-ndcToUV.Z / ndcToUV.X,
		-ndcToUV.W / ndcToUV.Y);
	gPerCameraUniformDefinition.gUVToClipScaleOffset.Set(uniforms, uvToNDC);

	if(!mRenderSettings->EnableLighting)
		gPerCameraUniformDefinition.gAmbientFactor.Set(uniforms, 100.0f);
	else
		gPerCameraUniformDefinition.gAmbientFactor.Set(uniforms, 0.0f);
}

Vector4 RendererView::GetNdcToUv() const
{
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	const GpuDeviceCapabilities& caps = gpuDevice->GetCapabilities();
	const Area2I& viewRect = mProperties.Target.ViewRect;

	float halfWidth = viewRect.Width * 0.5f;
	float halfHeight = viewRect.Height * 0.5f;

	float rtWidth = mProperties.Target.TargetWidth != 0 ? (float)mProperties.Target.TargetWidth : 20.0f;
	float rtHeight = mProperties.Target.TargetHeight != 0 ? (float)mProperties.Target.TargetHeight : 20.0f;

	Vector4 ndcToUV;
	ndcToUV.X = halfWidth / rtWidth;
	ndcToUV.Y = -halfHeight / rtHeight;
	ndcToUV.Z = viewRect.X / rtWidth + (halfWidth + caps.HorizontalTexelOffset) / rtWidth;
	ndcToUV.W = viewRect.Y / rtHeight + (halfHeight + caps.VerticalTexelOffset) / rtHeight;

	// Either of these flips the Y axis, but if they're both true they cancel out
	if((caps.Conventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ (caps.Conventions.NdcYAxis == GpuBackendConventions::Axis::Down))
		ndcToUV.Y = -ndcToUV.Y;

	return ndcToUV;
}

void RendererView::UpdateLightGrid(GpuCommandBuffer& commandBuffer, const VisibleLightData& visibleLightData, const VisibleReflectionProbeData& visibleReflProbeData)
{
	mLightGrid.UpdateGrid(commandBuffer, *this, visibleLightData, visibleReflProbeData, !mRenderSettings->EnableLighting);
}

RendererViewGroup::RendererViewGroup(RendererView** views, u32 numViews, bool mainPass, u32 shadowMapSize)
	: mIsMainPass(mainPass), mShadowRenderer(shadowMapSize)
{
	SetViews(views, numViews);
}

void RendererViewGroup::SetViews(RendererView** views, u32 numViews)
{
	mViews.clear();

	for(u32 i = 0; i < numViews; i++)
	{
		mViews.push_back(views[i]);
		views[i]->SetViewIdxInternal(i);
	}
}

void RendererViewGroup::DetermineVisibility(GpuCommandBuffer& commandBuffer, const RenderBeastScene& scene)
{
	const auto viewCount = (u32)mViews.size();

	// Early exit if no views render scene geometry
	bool anyViewsNeed3DDrawing = false;
	for(u32 viewIndex = 0; viewIndex < viewCount; viewIndex++)
	{
		if(mViews[viewIndex]->ShouldDraw3D())
		{
			anyViewsNeed3DDrawing = true;
			break;
		}
	}

	if(!anyViewsNeed3DDrawing)
		return;

	// Calculate renderable visibility per view
	mVisibility.Renderables.resize(scene.GetRenderableCount(), false);
	mVisibility.Renderables.assign(scene.GetRenderableCount(), false);

	mVisibility.ParticleSystems.resize(scene.GetParticleSystemCount(), false);
	mVisibility.ParticleSystems.assign(scene.GetParticleSystemCount(), false);

	mVisibility.Decals.resize(scene.GetDecalCount(), false);
	mVisibility.Decals.assign(scene.GetDecalCount(), false);

	for(u32 i = 0; i < viewCount; i++)
	{
		mViews[i]->DetermineVisible(scene.GetRenderables(), scene.GetRenderableCullInfos(), &mVisibility.Renderables);
		mViews[i]->DetermineVisible(scene.GetParticleRenderStates(), scene.GetParticleSystemCullInfos(), &mVisibility.ParticleSystems);
		mViews[i]->DetermineVisible(scene.GetDecals(), scene.GetDecalCullInfos(), &mVisibility.Decals);
	}

	// Generate render queues per camera
	for(u32 i = 0; i < viewCount; i++)
	{
		if(mViews[i]->ShouldDraw3D())
			mViews[i]->QueueDrawCommands(scene);
	}

	// Calculate light visibility for all views
	const auto radialLightCount = (u32)scene.GetRadialLights().size();
	mVisibility.RadialLights.resize(radialLightCount, false);
	mVisibility.RadialLights.assign(radialLightCount, false);

	const auto spotLightCount = (u32)scene.GetSpotLights().size();
	mVisibility.SpotLights.resize(spotLightCount, false);
	mVisibility.SpotLights.assign(spotLightCount, false);

	for(u32 viewIndex = 0; viewIndex < viewCount; viewIndex++)
	{
		if(!mViews[viewIndex]->ShouldDraw3D())
			continue;

		mViews[viewIndex]->DetermineVisible(scene.GetRadialLightWorldBounds(), LightType::Radial, &mVisibility.RadialLights);

		mViews[viewIndex]->DetermineVisible(scene.GetSpotLightWorldBounds(), LightType::Spot, &mVisibility.SpotLights);
	}

	// Calculate refl. probe visibility for all views
	const auto reflectionProbeCount = scene.GetReflectionProbeCount();
	mVisibility.ReflProbes.resize(reflectionProbeCount, false);
	mVisibility.ReflProbes.assign(reflectionProbeCount, false);

	// Note: Per-view visibility for refl. probes currently isn't calculated
	for(u32 viewIndex = 0; viewIndex < viewCount; viewIndex++)
	{
		const auto& viewProps = mViews[viewIndex]->GetProperties();

		// Don't recursively render reflection probes when generating reflection probe maps
		if(viewProps.CapturingReflections)
			continue;

		mViews[viewIndex]->CalculateVisibility(scene.GetReflectionProbeWorldBounds(), mVisibility.ReflProbes);
	}

	// Organize light and refl. probe visibility information in a more GPU friendly manner

	// Note: I'm determining light and refl. probe visibility for the entire group. It might be more performance
	// efficient to do it per view. Additionally I'm using a single GPU buffer to hold their information, which is
	// then updated when each view group is rendered. It might be better to keep one buffer reserved per-view.
	mVisibleLightData.Update(scene, *this);
	mVisibleReflProbeData.Update(scene, *this);

	const bool supportsClusteredForward = GetRenderBeast()->GetFeatureSet() == RenderBeastFeatureSet::Desktop;
	if(supportsClusteredForward)
	{
		for(u32 i = 0; i < viewCount; i++)
		{
			if(!mViews[i]->ShouldDraw3D())
				continue;

			mViews[i]->UpdateLightGrid(commandBuffer, mVisibleLightData, mVisibleReflProbeData);
		}
	}
}

void RendererView::RequestScreenCapture(TAsyncOp<TShared<PixelData>> asyncOp)
{
	mRequestedScreenCaptures.push_back(std::move(asyncOp));
}

void RendererView::ResolveSceneCaptures(GpuCommandBuffer& commandBuffer, const TShared<RenderTarget>& target) const
{
	if(mRequestedScreenCaptures.empty())
		return;

	if(target == nullptr)
	{
		for(auto& entry : mRequestedScreenCaptures)
			entry.CompleteOperation(nullptr);

		mRequestedScreenCaptures.clear();
		return;
	}

	Vector<TAsyncOp<TShared<PixelData>>> captureOps = std::move(mRequestedScreenCaptures);
	mRequestedScreenCaptures.clear();

	TAsyncOp<TShared<PixelData>> readOp = target->ReadAsync(commandBuffer);

	auto fnOnReadOpCompleted = [captureOps = std::move(captureOps), readOp]() mutable
	{
		for(auto& entry : captureOps)
			entry.CompleteOperation(readOp.GetReturnValue());
	};

	readOp.DoWhenComplete(std::move(fnOnReadOpCompleted));
}
}} // namespace b3d::render
