//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DReflectionProbe.h"
#include "B3DApplication.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "Image/B3DTexture.h"
#include "RTTI/B3DReflectionProbeRTTI.h"
#include "Profiling/B3DProfilerGPU.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Renderer/B3DIBLUtility.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

template<bool IsRenderProxy>
TReflectionProbe<IsRenderProxy>::TReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
	: mType(type), mRadius(radius), mExtents(extents)
{}

template<bool IsRenderProxy>
void TReflectionProbe<IsRenderProxy>::UpdateBounds()
{
	Vector3 position;
	Vector3 scale;

	if constexpr(IsRenderProxy)
	{
		const Transform& transform = static_cast<render::ReflectionProbe*>(this)->GetWorldTransform();
		position = transform.GetPosition();
		scale = transform.GetScale();
	}
	else
	{
		const Transform& transform = static_cast<ReflectionProbe*>(this)->SceneObject()->GetTransform();
		position = transform.GetPosition();
		scale = transform.GetScale();
	}

	switch(mType)
	{
	case ReflectionProbeType::Sphere:
		mBounds = Sphere(position, mRadius * std::max(std::max(scale.X, scale.Y), scale.Z));
		break;
	case ReflectionProbeType::Box:
		mBounds = Sphere(position, (mExtents * scale).Length());
		break;
	}
}

template <bool IsRenderProxy>
void TReflectionProbe<IsRenderProxy>::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if constexpr(!IsRenderProxy)
		CoreObject::MarkRenderProxyDataDirty((u32)flag);
}

template class TReflectionProbe<true>;
template class TReflectionProbe<false>;

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(ReflectionProbe, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mType)
		B3D_SYNC_BLOCK_ENTRY(mRadius)
		B3D_SYNC_BLOCK_ENTRY(mExtents)
		B3D_SYNC_BLOCK_ENTRY(mTransitionDistance)
		B3D_SYNC_BLOCK_ENTRY(mBounds)
		B3D_SYNC_BLOCK_ENTRY(mFilteredTexture)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(bool, mActive)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(SPtr<SceneInstance>, mSceneInstance)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END
}


ReflectionProbe::ReflectionProbe(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("ReflectionProbe");
}

ReflectionProbe::ReflectionProbe()
	: ReflectionProbe(nullptr)
{ }

void ReflectionProbe::SetCustomTexture(const HTexture& texture)
{
	mCustomTexture = texture;
	Filter();
}

float ReflectionProbe::GetRadius() const
{
	Vector3 scale = SceneObject()->GetTransform().GetScale();
	return mRadius * std::max(std::max(scale.X, scale.Y), scale.Z);
}

void ReflectionProbe::Capture()
{
	if(mCustomTexture != nullptr)
		return;

	CaptureAndFilter();
}

void ReflectionProbe::Filter()
{
	if(mCustomTexture == nullptr)
		return;

	CaptureAndFilter();
}

void ReflectionProbe::CaptureAndFilter()
{
	// If previous rendering task exists, cancel it
	if(mRendererTask != nullptr)
		mRendererTask->Cancel();

	TextureCreateInformation cubemapDesc;
	cubemapDesc.Name = "ReflectionProbe Cubemap";
	cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
	cubemapDesc.Format = PF_RG11B10F;
	cubemapDesc.Width = render::IBLUtility::kReflectionCubemapSize;
	cubemapDesc.Height = render::IBLUtility::kReflectionCubemapSize;
	cubemapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
	cubemapDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::RenderTarget;

	mFilteredTexture = Texture::CreateShared(cubemapDesc);

	auto fnRenderComplete = [this]()
	{
		mRendererTask = nullptr;
	};

	SPtr<render::ReflectionProbe> probeRenderProxy = B3DGetRenderProxy(this);
	SPtr<render::Texture> textureRenderProxy = B3DGetRenderProxy(mFilteredTexture);

	const SPtr<render::RendererScene> rendererScene = B3DGetRenderProxy(SceneObject()->GetScene()->GetRendererScene());
	if(mCustomTexture == nullptr)
	{
		auto fnRenderReflectionProbe = [textureRenderProxy, probeRenderProxy, rendererScene](render::GpuCommandBufferPool& commandBufferPool)
		{
			const SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("RenderAndFilterReflectionProbe"));
			SPtr<GpuCommandBufferProfiler> commandBufferProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*commandBuffer);

			commandBufferProfiler->BeginSample(*commandBuffer, "RenderAndFilterReflectionProbe");
			float radius = probeRenderProxy->mType == ReflectionProbeType::Sphere ? probeRenderProxy->mRadius : probeRenderProxy->mExtents.Length();

			render::CaptureSettings settings;
			settings.EncodeDepth = true;
			settings.DepthEncodeNear = radius;
			settings.DepthEncodeFar = radius + 1; // + 1 arbitrary, make it a customizable value?

			render::GetRenderer()->CaptureSceneCubeMap(*rendererScene, *commandBuffer, textureRenderProxy, probeRenderProxy->GetWorldTransform().GetPosition(), settings);
			render::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, textureRenderProxy, nullptr);

			probeRenderProxy->mFilteredTexture = textureRenderProxy;
			rendererScene->UpdateReflectionProbe(probeRenderProxy.get(), true);
			commandBufferProfiler->EndSample(*commandBuffer);

			GetGpuProfiler().ResolveProfileWhenReady("RenderAndFilterReflectionProbe", commandBufferProfiler);

			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			gpuDevice->SubmitCommandBuffer(commandBuffer);

			return true;
		};

		mRendererTask = render::RendererTask::Create("ReflProbeRender", fnRenderReflectionProbe);
	}
	else
	{
		SPtr<render::Texture> customTextureRenderProxy = B3DGetRenderProxy(mCustomTexture);
		auto fnFilterReflectionProbe = [customTextureRenderProxy, textureRenderProxy, probeRenderProxy, rendererScene](render::GpuCommandBufferPool& commandBufferPool)
		{
			const SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("FilterReflectionProbe"));
			SPtr<GpuCommandBufferProfiler> commandBufferProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*commandBuffer);

			commandBufferProfiler->BeginSample(*commandBuffer, "FilterReflectionProbe");

			render::GetIBLUtility().ScaleCubemap(*commandBuffer, customTextureRenderProxy, 0, textureRenderProxy, 0);
			render::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, textureRenderProxy, nullptr);

			probeRenderProxy->mFilteredTexture = textureRenderProxy;
			rendererScene->UpdateReflectionProbe(probeRenderProxy.get(), true);

			commandBufferProfiler->EndSample(*commandBuffer);

			GetGpuProfiler().ResolveProfileWhenReady("FilterReflectionProbe", commandBufferProfiler);

			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			gpuDevice->SubmitCommandBuffer(commandBuffer);

			return true;
		};

		mRendererTask = render::RendererTask::Create("ReflProbeRender", fnFilterReflectionProbe);
	}

	mRendererTask->OnComplete.Connect(fnRenderComplete);
	render::GetRenderer()->AddTask(mRendererTask);
}

void ReflectionProbe::Initialize()
{
	SetShared(B3DStaticGameObjectCast<ReflectionProbe>(mThisHandle).GetShared());

	Component::Initialize();
	CoreObject::Initialize();
}

void ReflectionProbe::OnCreated()
{
	UpdateBounds();
}

void ReflectionProbe::OnEnabled()
{
	// If filtered texture doesn't exist, ensure it is generated
	SPtr<Texture> filteredTexture = GetFilteredTexture();
	if(filteredTexture == nullptr)
	{
		if(GetCustomTexture())
			Filter();
		else
			Capture();
	}

	MarkRenderProxyDataDirty();
}

void ReflectionProbe::OnDisabled()
{
	MarkRenderProxyDataDirty();
}

void ReflectionProbe::OnDestroyed()
{
	if(mRendererTask)
		mRendererTask->Cancel();

	CoreObject::Destroy();
}

SPtr<render::RenderProxy> ReflectionProbe::CreateRenderProxy() const
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

	render::ReflectionProbe* renderProxy = new(B3DAllocate<render::ReflectionProbe>()) render::ReflectionProbe(B3DGetRenderProxy(scene), mType, mRadius, mExtents, B3DGetRenderProxy(mFilteredTexture));
	SPtr<render::ReflectionProbe> renderProxyShared = B3DMakeSharedFromExisting<render::ReflectionProbe>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* ReflectionProbe::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	const HSceneObject& sceneObject = SceneObject();

	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	syncPacket->mActive = GetEnabled();
	syncPacket->mSceneInstance = B3DGetRenderProxy(sceneObject->GetScene());
	syncPacket->mTransform = sceneObject->GetTransform();

	return syncPacket;
}

RTTIType* ReflectionProbe::GetRttiStatic()
{
	return ReflectionProbeRTTI::Instance();
}

RTTIType* ReflectionProbe::GetRtti() const
{
	return ReflectionProbe::GetRttiStatic();
}

namespace b3d { namespace render
{
ReflectionProbe::ReflectionProbe(const SPtr<SceneInstance>& scene, ReflectionProbeType type, float radius, const Vector3& extents, const SPtr<Texture>& filteredTexture)
	: TReflectionProbe(type, radius, extents), mSceneInstance(scene)
{
	mFilteredTexture = filteredTexture;
}

ReflectionProbe::~ReflectionProbe()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->UnregisterReflectionProbe(this);
}

void ReflectionProbe::Initialize()
{
	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->RegisterReflectionProbe(this);

	RenderProxy::Initialize();
}

float ReflectionProbe::GetRadius() const
{
	Vector3 scale = this->mTransform.GetScale();
	return mRadius * std::max(std::max(scale.X, scale.Y), scale.Z);
}

void ReflectionProbe::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	RenderProxySyncPacket* const syncPacket = data.GetSyncPacket<b3d::ReflectionProbe::SyncPacket>();
	if(syncPacket == nullptr)
		return;

	bool previousIsActive = mActive;
	ReflectionProbeType previousType = mType;

	syncPacket->ApplySyncData(this);

	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	if(syncPacket->Flags == (u32)ComponentDirtyFlag::Transform)
	{
		if(mActive)
			rendererScene->UpdateReflectionProbe(this, false);
	}
	else
	{
		if(previousIsActive != mActive)
		{
			if(mActive)
				rendererScene->RegisterReflectionProbe(this);
			else
			{
				ReflectionProbeType newType = mType;
				mType = previousType;
				rendererScene->UnregisterReflectionProbe(this);
				mType = newType;
			}
		}
		else
		{
			ReflectionProbeType newType = mType;
			mType = previousType;
			rendererScene->UnregisterReflectionProbe(this);
			mType = newType;

			rendererScene->RegisterReflectionProbe(this);
		}
	}
}
}}
