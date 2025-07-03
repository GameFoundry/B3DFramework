//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsReflectionProbe.h"

#include "BsCoreApplication.h"
#include "BsRendererScene.h"
#include "Private/RTTI/BsReflectionProbeRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "Scene/BsSceneObject.h"
#include "Image/BsTexture.h"
#include "Renderer/BsRenderer.h"
#include "Utility/BsUUID.h"
#include "Renderer/BsIBLUtility.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Profiling/BsProfilerGPU.h"
#include "RenderAPI/BsGpuDevice.h"
#include "Scene/BsSceneInstance.h"

using namespace b3d;

template<bool IsRenderProxy>
TReflectionProbe<IsRenderProxy>::TReflectionProbe(const SPtr<SceneInstanceType>& scene, ReflectionProbeType type, float radius, const Vector3& extents)
	: Super(scene), mType(type), mRadius(radius), mExtents(extents)
{}

template<bool IsRenderProxy>
float TReflectionProbe<IsRenderProxy>::GetRadius() const
{
	Vector3 scale = this->mTransform.GetScale();
	return mRadius * std::max(std::max(scale.X, scale.Y), scale.Z);
}

template<bool IsRenderProxy>
void TReflectionProbe<IsRenderProxy>::UpdateBounds()
{
	Vector3 position = this->mTransform.GetPosition();
	Vector3 scale = this->mTransform.GetScale();

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

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(ReflectionProbe, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mType)
		B3D_SYNC_BLOCK_ENTRY(mRadius)
		B3D_SYNC_BLOCK_ENTRY(mExtents)
		B3D_SYNC_BLOCK_ENTRY(mTransitionDistance)
		B3D_SYNC_BLOCK_ENTRY(mBounds)
		B3D_SYNC_BLOCK_ENTRY(mFilteredTexture)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END
}

ReflectionProbe::ReflectionProbe(const SPtr<SceneInstance>& scene, ReflectionProbeType type, float radius, const Vector3& extents)
	: TReflectionProbe(scene, type, radius, extents)
{
	// Calling virtual method is okay here because this is the most derived type
	UpdateBounds();
}

ReflectionProbe::~ReflectionProbe()
{
	if(mRendererTask)
		mRendererTask->Cancel();
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
	cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

	mFilteredTexture = Texture::CreateShared(cubemapDesc);

	auto renderComplete = [this]()
	{
		mRendererTask = nullptr;
	};

	SPtr<render::ReflectionProbe> probeRenderProxy = B3DGetRenderProxy(this);
	SPtr<render::Texture> textureRenderProxy = B3DGetRenderProxy(mFilteredTexture);

	const SPtr<render::RendererScene> rendererScene = B3DGetRenderProxy(mSceneInstance->GetRendererScene());
	if(mCustomTexture == nullptr)
	{
		auto renderReflProbe = [textureRenderProxy, probeRenderProxy, rendererScene](render::GpuCommandBufferPool& commandBufferPool)
		{
			const SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("RenderAndFilterReflectionProbe"));
			GetProfilerGPU().BeginSample(*commandBuffer, "RenderAndFilterReflectionProbe");
			float radius = probeRenderProxy->mType == ReflectionProbeType::Sphere ? probeRenderProxy->mRadius : probeRenderProxy->mExtents.Length();

			render::CaptureSettings settings;
			settings.EncodeDepth = true;
			settings.DepthEncodeNear = radius;
			settings.DepthEncodeFar = radius + 1; // + 1 arbitrary, make it a customizable value?

			render::GetRenderer()->CaptureSceneCubeMap(*rendererScene, *commandBuffer, textureRenderProxy, probeRenderProxy->GetTransform().GetPosition(), settings);
			render::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, textureRenderProxy, nullptr);

			probeRenderProxy->mFilteredTexture = textureRenderProxy;
			rendererScene->UpdateReflectionProbe(probeRenderProxy.get(), true);
			GetProfilerGPU().EndSample(*commandBuffer, "RenderAndFilterReflectionProbe");

			const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
			gpuDevice->SubmitCommandBuffer(commandBuffer);

			return true;
		};

		mRendererTask = render::RendererTask::Create("ReflProbeRender", renderReflProbe);
	}
	else
	{
		SPtr<render::Texture> customTextureRenderProxy = B3DGetRenderProxy(mCustomTexture);
		auto filterReflProbe = [customTextureRenderProxy, textureRenderProxy, probeRenderProxy, rendererScene](render::GpuCommandBufferPool& commandBufferPool)
		{
			const SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("FilterReflectionProbe"));
			GetProfilerGPU().BeginSample(*commandBuffer, "FilterReflectionProbe");

			render::GetIBLUtility().ScaleCubemap(*commandBuffer, customTextureRenderProxy, 0, textureRenderProxy, 0);
			render::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, textureRenderProxy, nullptr);

			probeRenderProxy->mFilteredTexture = textureRenderProxy;
			rendererScene->UpdateReflectionProbe(probeRenderProxy.get(), true);
			GetProfilerGPU().EndSample(*commandBuffer, "FilterReflectionProbe");

			const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
			gpuDevice->SubmitCommandBuffer(commandBuffer);

			return true;
		};

		mRendererTask = render::RendererTask::Create("ReflProbeRender", filterReflProbe);
	}

	mRendererTask->OnComplete.Connect(renderComplete);
	render::GetRenderer()->AddTask(mRendererTask);
}

SPtr<ReflectionProbe> ReflectionProbe::CreateSphere(const SPtr<SceneInstance>& scene, float radius)
{
	ReflectionProbe* probe = new(B3DAllocate<ReflectionProbe>()) ReflectionProbe(scene, ReflectionProbeType::Sphere, radius, Vector3::kZero);
	SPtr<ReflectionProbe> probePtr = B3DMakeSharedFromExisting<ReflectionProbe>(probe);
	probePtr->SetShared(probePtr);
	probePtr->Initialize();

	return probePtr;
}

SPtr<ReflectionProbe> ReflectionProbe::CreateBox(const SPtr<SceneInstance>& scene, const Vector3& extents)
{
	ReflectionProbe* probe = new(B3DAllocate<ReflectionProbe>()) ReflectionProbe(scene, ReflectionProbeType::Box, 1.0f, extents);
	SPtr<ReflectionProbe> probePtr = B3DMakeSharedFromExisting<ReflectionProbe>(probe);
	probePtr->SetShared(probePtr);
	probePtr->Initialize();

	return probePtr;
}

SPtr<ReflectionProbe> ReflectionProbe::CreateEmpty()
{
	ReflectionProbe* probe = new(B3DAllocate<ReflectionProbe>()) ReflectionProbe();
	SPtr<ReflectionProbe> probePtr = B3DMakeSharedFromExisting<ReflectionProbe>(probe);
	probePtr->SetShared(probePtr);

	return probePtr;
}

SPtr<render::RenderProxy> ReflectionProbe::CreateRenderProxy() const
{
	SPtr<render::Texture> filteredTexture = B3DGetRenderProxy(mFilteredTexture);

	render::ReflectionProbe* renderProxy = new(B3DAllocate<render::ReflectionProbe>()) render::ReflectionProbe(B3DGetRenderProxy(mSceneInstance), mType, mRadius, mExtents, filteredTexture);
	SPtr<render::ReflectionProbe> renderProxyShared = B3DMakeSharedFromExisting<render::ReflectionProbe>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* ReflectionProbe::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* const syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	if(B3D_ENSURE(syncPacket))
		syncPacket->SceneActorPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

void ReflectionProbe::MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flags)
{
	MarkRenderProxyDataDirty((u32)flags);
}

RTTIType* ReflectionProbe::GetRttiStatic()
{
	return ReflectionProbeRTTI::Instance();
}

RTTIType* ReflectionProbe::GetRtti() const
{
	return ReflectionProbe::GetRttiStatic();
}

template class TReflectionProbe<true>;
template class TReflectionProbe<false>;

namespace b3d { namespace render
{
ReflectionProbe::ReflectionProbe(const SPtr<SceneInstance>& scene, ReflectionProbeType type, float radius, const Vector3& extents, const SPtr<Texture>& filteredTexture)
	: TReflectionProbe(scene, type, radius, extents), mRendererId(0)
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

void ReflectionProbe::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::ReflectionProbe::SyncPacket>();
	if(!syncPacket)
		return;

	bool oldIsActive = mActive;
	ReflectionProbeType oldType = mType;

	syncPacket->ApplySyncData(this);

	UpdateBounds();

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	if(syncPacket->Flags == (u32)ActorDirtyFlag::Transform)
	{
		if(mActive)
			rendererScene->UpdateReflectionProbe(this, false);
	}
	else
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				rendererScene->RegisterReflectionProbe(this);
			else
			{
				ReflectionProbeType newType = mType;
				mType = oldType;
				rendererScene->UnregisterReflectionProbe(this);
				mType = newType;
			}
		}
		else
		{
			ReflectionProbeType newType = mType;
			mType = oldType;
			rendererScene->UnregisterReflectionProbe(this);
			mType = newType;

			rendererScene->RegisterReflectionProbe(this);
		}
	}
}
}}
