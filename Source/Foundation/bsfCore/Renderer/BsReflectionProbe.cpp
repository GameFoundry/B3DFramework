//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsReflectionProbe.h"

#include "BsCoreApplication.h"
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

using namespace bs;

template<bool IsRenderProxy>
TReflectionProbe<IsRenderProxy>::TReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
	: mType(type), mRadius(radius), mExtents(extents)
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

namespace bs
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

ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents)
	: TReflectionProbe(type, radius, extents)
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
	cubemapDesc.Width = ct::IBLUtility::kReflectionCubemapSize;
	cubemapDesc.Height = ct::IBLUtility::kReflectionCubemapSize;
	cubemapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
	cubemapDesc.Usage = TU_STATIC | TU_RENDERTARGET;

	mFilteredTexture = Texture::CreateShared(cubemapDesc);

	auto renderComplete = [this]()
	{
		mRendererTask = nullptr;
	};

	SPtr<ct::ReflectionProbe> probeRenderProxy = B3DGetRenderProxy(this);
	SPtr<ct::Texture> textureRenderProxy = B3DGetRenderProxy(mFilteredTexture);

	if(mCustomTexture == nullptr)
	{
		auto renderReflProbe = [textureRenderProxy, probeRenderProxy](ct::GpuCommandBufferPool& commandBufferPool)
		{
			const SPtr<ct::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(ct::GpuCommandBufferCreateInformation::Create("RenderAndFilterReflectionProbe"));
			GetProfilerGPU().BeginSample(*commandBuffer, "RenderAndFilterReflectionProbe");
			float radius = probeRenderProxy->mType == ReflectionProbeType::Sphere ? probeRenderProxy->mRadius : probeRenderProxy->mExtents.Length();

			ct::CaptureSettings settings;
			settings.EncodeDepth = true;
			settings.DepthEncodeNear = radius;
			settings.DepthEncodeFar = radius + 1; // + 1 arbitrary, make it a customizable value?

			ct::GetRenderer()->CaptureSceneCubeMap(*commandBuffer, textureRenderProxy, probeRenderProxy->GetTransform().GetPosition(), settings);
			ct::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, textureRenderProxy, nullptr);

			probeRenderProxy->mFilteredTexture = textureRenderProxy;
			ct::GetRenderer()->NotifyReflectionProbeUpdated(probeRenderProxy.get(), true);
			GetProfilerGPU().EndSample(*commandBuffer, "RenderAndFilterReflectionProbe");

			const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
			gpuDevice->SubmitCommandBuffer(commandBuffer);

			return true;
		};

		mRendererTask = ct::RendererTask::Create("ReflProbeRender", renderReflProbe);
	}
	else
	{
		SPtr<ct::Texture> customTextureRenderProxy = B3DGetRenderProxy(mCustomTexture);
		auto filterReflProbe = [customTextureRenderProxy, textureRenderProxy, probeRenderProxy](ct::GpuCommandBufferPool& commandBufferPool)
		{
			const SPtr<ct::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(ct::GpuCommandBufferCreateInformation::Create("FilterReflectionProbe"));
			GetProfilerGPU().BeginSample(*commandBuffer, "FilterReflectionProbe");

			ct::GetIBLUtility().ScaleCubemap(*commandBuffer, customTextureRenderProxy, 0, textureRenderProxy, 0);
			ct::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, textureRenderProxy, nullptr);

			probeRenderProxy->mFilteredTexture = textureRenderProxy;
			ct::GetRenderer()->NotifyReflectionProbeUpdated(probeRenderProxy.get(), true);
			GetProfilerGPU().EndSample(*commandBuffer, "FilterReflectionProbe");

			const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
			gpuDevice->SubmitCommandBuffer(commandBuffer);

			return true;
		};

		mRendererTask = ct::RendererTask::Create("ReflProbeRender", filterReflProbe);
	}

	mRendererTask->OnComplete.Connect(renderComplete);
	ct::GetRenderer()->AddTask(mRendererTask);
}

SPtr<ReflectionProbe> ReflectionProbe::CreateSphere(float radius)
{
	ReflectionProbe* probe = new(B3DAllocate<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Sphere, radius, Vector3::kZero);
	SPtr<ReflectionProbe> probePtr = B3DMakeSharedFromExisting<ReflectionProbe>(probe);
	probePtr->SetShared(probePtr);
	probePtr->Initialize();

	return probePtr;
}

SPtr<ReflectionProbe> ReflectionProbe::CreateBox(const Vector3& extents)
{
	ReflectionProbe* probe = new(B3DAllocate<ReflectionProbe>()) ReflectionProbe(ReflectionProbeType::Box, 1.0f, extents);
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

SPtr<ct::RenderProxy> ReflectionProbe::CreateRenderProxy() const
{
	SPtr<ct::Texture> filteredTexture = B3DGetRenderProxy(mFilteredTexture);

	ct::ReflectionProbe* renderProxy = new(B3DAllocate<ct::ReflectionProbe>()) ct::ReflectionProbe(mType, mRadius, mExtents, filteredTexture);
	SPtr<ct::ReflectionProbe> renderProxyShared = B3DMakeSharedFromExisting<ct::ReflectionProbe>(renderProxy);
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

namespace bs { namespace ct
{
ReflectionProbe::ReflectionProbe(ReflectionProbeType type, float radius, const Vector3& extents, const SPtr<Texture>& filteredTexture)
	: TReflectionProbe(type, radius, extents), mRendererId(0)
{
	mFilteredTexture = filteredTexture;
}

ReflectionProbe::~ReflectionProbe()
{
	GetRenderer()->NotifyReflectionProbeRemoved(this);
}

void ReflectionProbe::Initialize()
{
	UpdateBounds();
	GetRenderer()->NotifyReflectionProbeAdded(this);

	RenderProxy::Initialize();
}

void ReflectionProbe::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<bs::ReflectionProbe::SyncPacket>();
	if(!syncPacket)
		return;

	bool oldIsActive = mActive;
	ReflectionProbeType oldType = mType;

	syncPacket->ApplySyncData(this);

	UpdateBounds();

	if(syncPacket->Flags == (u32)ActorDirtyFlag::Transform)
	{
		if(mActive)
			GetRenderer()->NotifyReflectionProbeUpdated(this, false);
	}
	else
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				GetRenderer()->NotifyReflectionProbeAdded(this);
			else
			{
				ReflectionProbeType newType = mType;
				mType = oldType;
				GetRenderer()->NotifyReflectionProbeRemoved(this);
				mType = newType;
			}
		}
		else
		{
			ReflectionProbeType newType = mType;
			mType = oldType;
			GetRenderer()->NotifyReflectionProbeRemoved(this);
			mType = newType;

			GetRenderer()->NotifyReflectionProbeAdded(this);
		}
	}
}
}}
