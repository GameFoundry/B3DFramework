//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DReflectionProbe.h"
#include "B3DApplication.h"
#include "CoreObject/B3DCoreObjectSync.h"
#include "ECS/B3DRegistry.h"
#include "Image/B3DTexture.h"
#include "RTTI/B3DReflectionProbeRTTI.h"
#include "Profiling/B3DProfilerGPU.h"
#include "RenderAPI/B3DGpuDevice.h"
#include "Renderer/B3DIBLUtility.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneObjectFragments.h"

using namespace b3d;

// ECS free functions

ecs::ReflectionProbe& ecs::CreateReflectionProbe(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene, const Transform& transform)
{
	ecs::ReflectionProbeECSUtility::CreateFragmentsIfMissing(registry, entity);
	registry.AddComponent<ecs::WorldTransform>(entity, ecs::WorldTransform(transform));
	rendererScene->AllocateReflectionProbeId(registry, entity);
	ecs::ReflectionProbeECSUtility::MarkDirty(registry, entity);

	return registry.GetComponents<ecs::ReflectionProbe>(entity);
}

void ecs::DestroyReflectionProbe(ecs::Registry& registry, ecs::Entity entity)
{
	ecs::ReflectionProbeECSUtility::RemoveFragments(registry, entity);
}

// TReflectionProbeData

template <bool IsRenderProxy>
void TReflectionProbeData<IsRenderProxy>::ComputeBounds(const Transform& transform)
{
	Vector3 position = transform.GetPosition();
	Vector3 scale = transform.GetScale();

	switch(Type)
	{
	case ReflectionProbeType::Sphere:
		Bounds = Sphere(position, Radius * std::max(std::max(scale.X, scale.Y), scale.Z));
		break;
	case ReflectionProbeType::Box:
		Bounds = Sphere(position, (Extents * scale).Length());
		break;
	}
}

template void TReflectionProbeData<true>::ComputeBounds(const Transform& transform);
template void TReflectionProbeData<false>::ComputeBounds(const Transform& transform);

// New ECS-based sync blocks

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::ReflectionProbe, FullSyncPacket, TReflectionProbeData<true>)
		B3D_SYNC_BLOCK_ENTRY(Type)
		B3D_SYNC_BLOCK_ENTRY(Radius)
		B3D_SYNC_BLOCK_ENTRY(Extents)
		B3D_SYNC_BLOCK_ENTRY(TransitionDistance)
		B3D_SYNC_BLOCK_ENTRY(Bounds)
		B3D_SYNC_BLOCK_ENTRY(FilteredTexture)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::ReflectionProbe, TransformSyncPacket, TReflectionProbeData<true>)
		B3D_SYNC_BLOCK_ENTRY(Bounds)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	struct ReflectionProbeFullUpdateChannel : TRendererObjectECSSyncChannel
	<
		ReflectionProbeFullUpdateChannel,
		ecs::ReflectionProbe::FullSyncPacket,
		ecs::ReflectionProbeDirty,
		ecs::ReflectionProbe, ecs::WorldTransform, ecs::ReflectionProbeId
	>
	{
		void Write(ReflectionProbeObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToCreate(&allocator);
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToDestroy(&allocator);

			WritePackets(storage, allocator, [&renderStatesToCreate, &renderStatesToDestroy, &storage](ecs::ReflectionProbe::FullSyncPacket& packet, PackedRendererId rendererId)
			{
				render::ReflectionProbeProxy& proxy = storage.GetReflectionProbeProxy(rendererId);

				bool wasRegistered = proxy.mRendererId != kInvalidPackedRendererId;
				proxy.mRendererId = rendererId;
				packet.ApplySyncData(&proxy.mData);

				proxy.mTransform = packet.TransformData;
				proxy.mData.ComputeBounds(proxy.mTransform);

				if(wasRegistered)
					renderStatesToDestroy.push_back(rendererId);

				renderStatesToCreate.push_back(rendererId);
			});

			if(!renderStatesToDestroy.empty())
				storage.DestroyRenderState(renderStatesToDestroy);

			if(!renderStatesToCreate.empty())
				storage.CreateRenderState(renderStatesToCreate);
		}

		void CreateAndPopulatePacket(ecs::ReflectionProbe& fragment, ecs::WorldTransform& transform, ecs::ReflectionProbeId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
		}
	};

	struct ReflectionProbeTransformUpdateChannel : TRendererObjectECSSyncChannel
	<
		ReflectionProbeTransformUpdateChannel,
		ecs::ReflectionProbe::TransformSyncPacket,
		ecs::ReflectionProbeTransformDirty,
		ecs::ReflectionProbe, ecs::WorldTransform, ecs::ReflectionProbeId
	>
	{
		void Write(ReflectionProbeObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToUpdate(&allocator);

			WritePackets(storage, allocator, [&renderStatesToUpdate, &storage](ecs::ReflectionProbe::TransformSyncPacket& packet, PackedRendererId rendererId)
			{
				render::ReflectionProbeProxy& proxy = storage.GetReflectionProbeProxy(rendererId);
				proxy.mTransform = packet.TransformData;
				proxy.mData.ComputeBounds(proxy.mTransform);

				renderStatesToUpdate.push_back(rendererId);
			});

			if(!renderStatesToUpdate.empty())
				storage.UpdateRenderState(renderStatesToUpdate);
		}

		void CreateAndPopulatePacket(ecs::ReflectionProbe& fragment, ecs::WorldTransform& transform, ecs::ReflectionProbeId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
		}
	};

	using ReflectionProbeSyncBatch = TRendererObjectECSSyncBatch<ReflectionProbeFullUpdateChannel, ReflectionProbeTransformUpdateChannel>;
}

// ecs::ReflectionProbe fragment access

ecs::ReflectionProbe& ReflectionProbe::GetFragment()
{
	return GetECSRegistry()->GetComponents<ecs::ReflectionProbe>(GetECSEntity());
}

const ecs::ReflectionProbe& ReflectionProbe::GetFragment() const
{
	return GetECSRegistry()->GetComponents<ecs::ReflectionProbe>(GetECSEntity());
}

const TReflectionProbeData<false>& ReflectionProbe::GetReflectionProbeData() const
{
	return GetFragment();
}

// b3d::ReflectionProbe setters

void ReflectionProbe::SetType(ReflectionProbeType type)
{
	GetFragment().Type = type;
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

void ReflectionProbe::SetRadius(float radius)
{
	GetFragment().Radius = radius;
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

void ReflectionProbe::SetExtents(const Vector3& extents)
{
	GetFragment().Extents = extents;
	MarkRenderProxyDataDirty();
	UpdateBounds();
}

void ReflectionProbe::SetTransitionDistance(float distance)
{
	GetFragment().TransitionDistance = std::max(1.0f, distance);
}

float ReflectionProbe::GetWorldRadius() const
{
	Vector3 scale = SceneObject()->GetTransform().GetScale();
	return GetFragment().Radius * std::max(std::max(scale.X, scale.Y), scale.Z);
}

Vector3 ReflectionProbe::GetWorldExtents() const
{
	return GetFragment().Extents * SceneObject()->GetTransform().GetScale();
}

void ReflectionProbe::SetCustomTexture(const HTexture& texture)
{
	GetFragment().CustomTexture = texture;

	if(texture != nullptr)
	{
		const SPtr<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
		ReflectionProbeUtility::Filter(*GetECSRegistry(), GetECSEntity(), rendererScene);
	}
}

void ReflectionProbe::Capture()
{
	const SPtr<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ReflectionProbeUtility::Capture(*GetECSRegistry(), GetECSEntity(), rendererScene);
}

void ReflectionProbe::UpdateBounds()
{
	GetFragment().ComputeBounds(SceneObject()->GetTransform());
}

void ReflectionProbe::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if(!SceneObject().IsValid())
		return;
	ecs::ReflectionProbeECSUtility::MarkDirty(*GetECSRegistry(), GetECSEntity(), flag);
}

// ReflectionProbeUtility

void ReflectionProbeUtility::CaptureAndFilter(ecs::Registry& registry, ecs::Entity entity, const SPtr<RendererScene>& rendererScene)
{
	ecs::ReflectionProbe& fragment = registry.GetComponents<ecs::ReflectionProbe>(entity);
	RendererId probeId = registry.HasAllOf<ecs::ReflectionProbeId>(entity)
		? registry.GetComponents<ecs::ReflectionProbeId>(entity).Id : kInvalidRendererId;

	// Cancel any previous in-flight task
	if(fragment.PendingTask != nullptr)
		fragment.PendingTask->Cancel();

	TextureCreateInformation cubemapDesc;
	cubemapDesc.Name = "ReflectionProbe Cubemap";
	cubemapDesc.Type = TEX_TYPE_CUBE_MAP;
	cubemapDesc.Format = PF_RG11B10F;
	cubemapDesc.Width = render::IBLUtility::kReflectionCubemapSize;
	cubemapDesc.Height = render::IBLUtility::kReflectionCubemapSize;
	cubemapDesc.MipMapCount = PixelUtility::GetMipmapCount(cubemapDesc.Width, cubemapDesc.Height, 1, cubemapDesc.Format);
	cubemapDesc.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::RenderTarget;

	fragment.FilteredTexture = Texture::CreateShared(cubemapDesc);

	SPtr<render::Texture> textureRenderProxy = B3DGetRenderProxy(fragment.FilteredTexture);
	const SPtr<render::RendererScene> renderSceneProxy = B3DGetRenderProxy(rendererScene);

	SPtr<render::RendererTask> task;
	if(fragment.CustomTexture == nullptr)
	{
		auto fnRenderReflectionProbe = [textureRenderProxy, renderSceneProxy, probeId](render::GpuCommandBufferPool& commandBufferPool)
		{
			const SPtr<ReflectionProbeObjectStorageBase>& reflProbeStorage = renderSceneProxy->GetReflectionProbeStorage();
			PackedRendererId packedId = reflProbeStorage->GetPackedRendererId(probeId);
			if(packedId == kInvalidPackedRendererId)
				return true;

			render::ReflectionProbeProxy& proxy = reflProbeStorage->GetReflectionProbeProxy(packedId);
			float probeRadius = proxy.GetType() == ReflectionProbeType::Sphere ? proxy.GetRadius() : proxy.GetExtents().Length();
			Vector3 probePosition = proxy.GetWorldTransform().GetPosition();

			const SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("RenderAndFilterReflectionProbe"));
			SPtr<GpuCommandBufferProfiler> commandBufferProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*commandBuffer);

			commandBufferProfiler->BeginSample(*commandBuffer, "RenderAndFilterReflectionProbe");

			render::CaptureSettings settings;
			settings.EncodeDepth = true;
			settings.DepthEncodeNear = probeRadius;
			settings.DepthEncodeFar = probeRadius + 1; // + 1 arbitrary, make it a customizable value?

			render::GetRenderer()->CaptureSceneCubeMap(*renderSceneProxy, *commandBuffer, textureRenderProxy, probePosition, settings);
			render::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, textureRenderProxy, nullptr);

			reflProbeStorage->UpdateFilteredTexture(probeId, textureRenderProxy);

			commandBufferProfiler->EndSample(*commandBuffer);

			GetGpuProfiler().ResolveProfileWhenReady("RenderAndFilterReflectionProbe", commandBufferProfiler);

			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			gpuDevice->SubmitCommandBuffer(commandBuffer);

			return true;
		};

		task = render::RendererTask::Create("ReflProbeRender", fnRenderReflectionProbe);
	}
	else
	{
		SPtr<render::Texture> customTextureRenderProxy = B3DGetRenderProxy(fragment.CustomTexture);
		auto fnFilterReflectionProbe = [customTextureRenderProxy, textureRenderProxy, renderSceneProxy, probeId](render::GpuCommandBufferPool& commandBufferPool)
		{
			const SPtr<render::GpuCommandBuffer> commandBuffer = commandBufferPool.Create(render::GpuCommandBufferCreateInformation::Create("FilterReflectionProbe"));
			SPtr<GpuCommandBufferProfiler> commandBufferProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*commandBuffer);

			commandBufferProfiler->BeginSample(*commandBuffer, "FilterReflectionProbe");

			render::GetIBLUtility().ScaleCubemap(*commandBuffer, customTextureRenderProxy, 0, textureRenderProxy, 0);
			render::GetIBLUtility().FilterCubemapForSpecular(*commandBuffer, textureRenderProxy, nullptr);

			renderSceneProxy->GetReflectionProbeStorage()->UpdateFilteredTexture(probeId, textureRenderProxy);

			commandBufferProfiler->EndSample(*commandBuffer);

			GetGpuProfiler().ResolveProfileWhenReady("FilterReflectionProbe", commandBufferProfiler);

			const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
			gpuDevice->SubmitCommandBuffer(commandBuffer);

			return true;
		};

		task = render::RendererTask::Create("ReflProbeRender", fnFilterReflectionProbe);
	}

	fragment.PendingTask = task;
	render::GetRenderer()->AddTask(task);
}

void ReflectionProbeUtility::Capture(ecs::Registry& registry, ecs::Entity entity,
	const SPtr<RendererScene>& rendererScene)
{
	ecs::ReflectionProbe& fragment = registry.GetComponents<ecs::ReflectionProbe>(entity);
	if(fragment.CustomTexture != nullptr)
		return;

	CaptureAndFilter(registry, entity, rendererScene);
}

void ReflectionProbeUtility::Filter(ecs::Registry& registry, ecs::Entity entity,
	const SPtr<RendererScene>& rendererScene)
{
	ecs::ReflectionProbe& fragment = registry.GetComponents<ecs::ReflectionProbe>(entity);
	if(fragment.CustomTexture == nullptr)
		return;

	CaptureAndFilter(registry, entity, rendererScene);
}

// b3d::ReflectionProbe lifecycle

ReflectionProbe::ReflectionProbe(const HSceneObject& parent)
	: Component(parent)
{
	SetFlag(ComponentFlag::AlwaysRun, true);
	SetName("ReflectionProbe");
}

ReflectionProbe::ReflectionProbe()
	: ReflectionProbe(nullptr)
{ }

void ReflectionProbe::Initialize()
{
	SetShared(B3DStaticGameObjectCast<ReflectionProbe>(mThisHandle).GetShared());
	ecs::ReflectionProbeECSUtility::CreateFragmentsIfMissing(*GetECSRegistry(), GetECSEntity());
	Component::Initialize();
	CoreObject::Initialize();
}

void ReflectionProbe::OnCreated()
{
	UpdateBounds();
}

void ReflectionProbe::OnEnabled()
{
	ecs::Registry* registry = GetECSRegistry();
	ecs::Entity entity = GetECSEntity();
	const SPtr<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::ReflectionProbeECSUtility::RegisterWithRenderer(*registry, entity, rendererScene);

	ecs::ReflectionProbe& fragment = GetFragment();

	// If filtered texture doesn't exist, ensure it is generated
	if(fragment.FilteredTexture == nullptr)
	{
		if(fragment.CustomTexture != nullptr)
			ReflectionProbeUtility::Filter(*registry, entity, rendererScene);
		else
			ReflectionProbeUtility::Capture(*registry, entity, rendererScene);
	}
}

void ReflectionProbe::OnDisabled()
{
	ecs::ReflectionProbe& fragment = GetFragment();
	if(fragment.PendingTask != nullptr)
	{
		fragment.PendingTask->Cancel();
		fragment.PendingTask = nullptr;
	}

	const SPtr<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::ReflectionProbeECSUtility::UnregisterFromRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);
}

void ReflectionProbe::OnDestroyed()
{
	ecs::ReflectionProbeECSUtility::RemoveFragments(*GetECSRegistry(), GetECSEntity());
}

void ReflectionProbe::OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity)
{
	ecs::ReflectionProbeECSUtility::ChangeScene(oldScene, oldEntity, *SceneObject()->GetScene(), GetECSEntity(), GetEnabled());
}

void ReflectionProbe::OnTransformChanged(TransformChangedFlags flags)
{
	UpdateBounds();
	MarkRenderProxyDataDirty(ComponentDirtyFlag::Transform);
}

RTTIType* ReflectionProbe::GetRttiStatic()
{
	return ReflectionProbeRTTI::Instance();
}

RTTIType* ReflectionProbe::GetRtti() const
{
	return ReflectionProbe::GetRttiStatic();
}

namespace b3d::ecs
{
	RTTIType* ReflectionProbe::GetRttiStatic() { return ECSReflectionProbeRTTI::Instance(); }
	RTTIType* ReflectionProbe::GetRtti() const { return ReflectionProbe::GetRttiStatic(); }
}

// ReflectionProbeObjectStorageBase

void* ReflectionProbeObjectStorageBase::SyncRead(ecs::Registry& registry, FrameAllocator& allocator)
{
	return ReflectionProbeSyncBatch::Read(*this, registry, allocator);
}

void ReflectionProbeObjectStorageBase::SyncWrite(void* rawData, FrameAllocator& allocator)
{
	ReflectionProbeSyncBatch::Write(*this, rawData, allocator);
}

void ReflectionProbeObjectStorageBase::UpdateFilteredTexture(RendererId probeId, const SPtr<render::Texture>& texture)
{
	PackedRendererId packedId = GetPackedRendererId(probeId);
	if(packedId == kInvalidPackedRendererId)
		return;

	mReflectionProbeProxies[packedId].mData.FilteredTexture = texture;
	OnFilteredTextureUpdated(packedId);
}

