//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererScene.h"
#include "Allocators/B3DFrameAllocator.h"
#include "Components/B3DDecal.h"
#include "Components/B3DLight.h"
#include "Components/B3DParticleSystem.h"
#include "Components/B3DReflectionProbe.h"
#include "Components/B3DRenderable.h"
#include "CoreObject/B3DRenderThread.h"
#include "ECS/B3DRegistry.h"
#include "Scene/B3DSceneInstance.h"
#include "B3DRenderer.h"

namespace b3d
{
	RendererScene::~RendererScene()
	{
		mLightRemovedHandle.Disconnect();
		mRenderableRemovedHandle.Disconnect();
		mDecalRemovedHandle.Disconnect();
		mParticleSystemRemovedHandle.Disconnect();
		mReflectionProbeRemovedHandle.Disconnect();
	}

	SPtr<RendererScene> RendererScene::Create()
	{
		RendererScene* rendererScene = new (B3DAllocate<RendererScene>()) RendererScene();
		SPtr<RendererScene> rendererSceneShared = B3DMakeSharedFromExisting(rendererScene);
		rendererSceneShared->SetShared(rendererSceneShared);
		rendererSceneShared->Initialize();

		return rendererSceneShared;
	}

	void RendererScene::Initialize()
	{
		CoreObject::Initialize();

		SPtr<render::RendererScene> renderProxy = B3DGetRenderProxy(this);
		mDecalStorage = renderProxy->GetDecalStorage();
		mRenderableStorage = renderProxy->GetRenderableStorage();
		mLightStorage = renderProxy->GetLightStorage();
		mParticleSystemStorage = renderProxy->GetParticleSystemStorage();
		mReflectionProbeStorage = renderProxy->GetReflectionProbeStorage();
	}

	void RendererScene::SetOwner(const SPtr<SceneInstance>& scene)
	{
		mOwner = scene;
		ecs::Registry& registry = scene->GetECSRegistry();

		mLightRemovedHandle = registry.OnComponentRemoved<ecs::Light>().Connect(
			[this, &registry](ecs::Entity entity)
			{
				DeallocateLightId(registry, entity);
				registry.RemoveComponents<ecs::LightDirty>(entity);
				registry.RemoveComponents<ecs::LightTransformDirty>(entity);
			});

		mRenderableRemovedHandle = registry.OnComponentRemoved<ecs::Renderable>().Connect(
			[this, &registry](ecs::Entity entity)
			{
				DeallocateRenderableId(registry, entity);
				registry.RemoveComponents<ecs::RenderableDirty>(entity);
				registry.RemoveComponents<ecs::RenderableTransformDirty>(entity);
			});

		mDecalRemovedHandle = registry.OnComponentRemoved<ecs::Decal>().Connect(
			[this, &registry](ecs::Entity entity)
			{
				DeallocateDecalId(registry, entity);
				registry.RemoveComponents<ecs::DecalDirty>(entity);
				registry.RemoveComponents<ecs::DecalTransformDirty>(entity);
			});

		mParticleSystemRemovedHandle = registry.OnComponentRemoved<ecs::ParticleSystem>().Connect(
			[this, &registry](ecs::Entity entity)
			{
				DeallocateParticleSystemId(registry, entity);
				registry.RemoveComponents<ecs::ParticleSystemDirty>(entity);
				registry.RemoveComponents<ecs::ParticleSystemTransformDirty>(entity);
			});

		mReflectionProbeRemovedHandle = registry.OnComponentRemoved<ecs::ReflectionProbe>().Connect(
			[this, &registry](ecs::Entity entity)
			{
				ecs::ReflectionProbe& fragment = registry.GetComponents<ecs::ReflectionProbe>(entity);
				if(fragment.PendingTask != nullptr)
				{
					fragment.PendingTask->Cancel();
					fragment.PendingTask = nullptr;
				}

				DeallocateReflectionProbeId(registry, entity);
				registry.RemoveComponents<ecs::ReflectionProbeDirty>(entity);
				registry.RemoveComponents<ecs::ReflectionProbeTransformDirty>(entity);
			});
	}

	RendererId RendererScene::AllocateRenderableId(ecs::Registry& registry, ecs::Entity entity)
	{
		RendererId objectId = mRenderableStorage->AllocateRendererId();
		registry.AddComponent<ecs::RenderableId>(entity, ecs::RenderableId{objectId});

		return objectId;
	}

	void RendererScene::DeallocateRenderableId(ecs::Registry& registry, ecs::Entity entity)
	{
		if(!registry.HasAllOf<ecs::RenderableId>(entity))
			return;

		RendererId objectId = registry.GetComponents<ecs::RenderableId>(entity).Id;
		if(objectId != kInvalidRendererId)
			mRenderableStorage->DeallocateRendererId(objectId);

		registry.RemoveComponents<ecs::RenderableId>(entity);
	}

	RendererId RendererScene::AllocateLightId(ecs::Registry& registry, ecs::Entity entity)
	{
		RendererId objectId = mLightStorage->AllocateRendererId();
		registry.AddComponent<ecs::LightId>(entity, ecs::LightId{objectId});

		return objectId;
	}

	void RendererScene::DeallocateLightId(ecs::Registry& registry, ecs::Entity entity)
	{
		if(!registry.HasAllOf<ecs::LightId>(entity))
			return;

		RendererId objectId = registry.GetComponents<ecs::LightId>(entity).Id;
		if(objectId != kInvalidRendererId)
			mLightStorage->DeallocateRendererId(objectId);

		registry.RemoveComponents<ecs::LightId>(entity);
	}

	RendererId RendererScene::AllocateDecalId(ecs::Registry& registry, ecs::Entity entity)
	{
		RendererId objectId = mDecalStorage->AllocateRendererId();
		registry.AddComponent<ecs::DecalId>(entity, ecs::DecalId{objectId});

		return objectId;
	}

	void RendererScene::DeallocateDecalId(ecs::Registry& registry, ecs::Entity entity)
	{
		if(!registry.HasAllOf<ecs::DecalId>(entity))
			return;

		RendererId objectId = registry.GetComponents<ecs::DecalId>(entity).Id;
		if(objectId != kInvalidRendererId)
			mDecalStorage->DeallocateRendererId(objectId);

		registry.RemoveComponents<ecs::DecalId>(entity);
	}

	RendererId RendererScene::AllocateParticleSystemId(ecs::Registry& registry, ecs::Entity entity)
	{
		RendererId objectId = mParticleSystemStorage->AllocateRendererId();
		registry.AddComponent<ecs::ParticleSystemId>(entity, ecs::ParticleSystemId{objectId});

		return objectId;
	}

	void RendererScene::DeallocateParticleSystemId(ecs::Registry& registry, ecs::Entity entity)
	{
		if(!registry.HasAllOf<ecs::ParticleSystemId>(entity))
			return;

		RendererId objectId = registry.GetComponents<ecs::ParticleSystemId>(entity).Id;
		if(objectId != kInvalidRendererId)
			mParticleSystemStorage->DeallocateRendererId(objectId);

		registry.RemoveComponents<ecs::ParticleSystemId>(entity);
	}

	RendererId RendererScene::AllocateReflectionProbeId(ecs::Registry& registry, ecs::Entity entity)
	{
		RendererId objectId = mReflectionProbeStorage->AllocateRendererId();
		registry.AddComponent<ecs::ReflectionProbeId>(entity, ecs::ReflectionProbeId{objectId});

		return objectId;
	}

	void RendererScene::DeallocateReflectionProbeId(ecs::Registry& registry, ecs::Entity entity)
	{
		if(!registry.HasAllOf<ecs::ReflectionProbeId>(entity))
			return;

		RendererId objectId = registry.GetComponents<ecs::ReflectionProbeId>(entity).Id;
		if(objectId != kInvalidRendererId)
			mReflectionProbeStorage->DeallocateRendererId(objectId);

		registry.RemoveComponents<ecs::ReflectionProbeId>(entity);
	}

	SPtr<render::RenderProxy> RendererScene::CreateRenderProxy() const
	{
		return render::GetRenderer()->CreateScene();
	}

	void RendererScene::SyncToRenderThread(ecs::Registry& registry, FrameAllocator& allocator)
	{
		RendererSceneSyncData* syncData = SyncRead(registry, allocator);
		if(syncData == nullptr)
			return;

		SPtr<render::RendererScene> renderScene = B3DGetRenderProxy(this);
		GetRenderThread().PostCommand(
			[renderScene = std::move(renderScene), syncData, &allocator]
			{
				renderScene->SyncWrite(*syncData, allocator);
			},
			"RendererScene::SyncWrite");
	}

	RendererSceneSyncData* RendererScene::SyncRead(ecs::Registry& registry, FrameAllocator& allocator)
	{
		RendererSceneSyncData* batch = nullptr;

		if(mRenderableStorage != nullptr)
		{
			void* renderableBatchData = mRenderableStorage->SyncRead(registry, allocator);
			if(renderableBatchData != nullptr)
			{
				if(batch == nullptr)
					batch = allocator.Construct<RendererSceneSyncData>();

				batch->RenderableBatchData = renderableBatchData;
			}
		}

		if(mLightStorage != nullptr)
		{
			void* lightBatchData = mLightStorage->SyncRead(registry, allocator);
			if(lightBatchData != nullptr)
			{
				if(batch == nullptr)
					batch = allocator.Construct<RendererSceneSyncData>();

				batch->LightBatchData = lightBatchData;
			}
		}

		if(mDecalStorage != nullptr)
		{
			void* decalBatchData = mDecalStorage->SyncRead(registry, allocator);
			if(decalBatchData != nullptr)
			{
				if(batch == nullptr)
					batch = allocator.Construct<RendererSceneSyncData>();

				batch->DecalBatchData = decalBatchData;
			}
		}

		if(mParticleSystemStorage != nullptr)
		{
			void* particleSystemBatchData = mParticleSystemStorage->SyncRead(registry, allocator);
			if(particleSystemBatchData != nullptr)
			{
				if(batch == nullptr)
					batch = allocator.Construct<RendererSceneSyncData>();

				batch->ParticleSystemBatchData = particleSystemBatchData;
			}
		}

		if(mReflectionProbeStorage != nullptr)
		{
			void* reflectionProbeBatchData = mReflectionProbeStorage->SyncRead(registry, allocator);
			if(reflectionProbeBatchData != nullptr)
			{
				if(batch == nullptr)
					batch = allocator.Construct<RendererSceneSyncData>();

				batch->ReflectionProbeBatchData = reflectionProbeBatchData;
			}
		}

		return batch;
	}

	namespace render
	{
		void RendererScene::SyncWrite(RendererSceneSyncData& batchData, FrameAllocator& allocator)
		{
			if(batchData.RenderableBatchData != nullptr)
				mRenderableStorage->SyncWrite(batchData.RenderableBatchData, allocator);

			if(batchData.LightBatchData != nullptr)
				mLightStorage->SyncWrite(batchData.LightBatchData, allocator);

			if(batchData.DecalBatchData != nullptr)
				mDecalStorage->SyncWrite(batchData.DecalBatchData, allocator);

			if(batchData.ParticleSystemBatchData != nullptr)
				mParticleSystemStorage->SyncWrite(batchData.ParticleSystemBatchData, allocator);

			if(batchData.ReflectionProbeBatchData != nullptr)
				mReflectionProbeStorage->SyncWrite(batchData.ReflectionProbeBatchData, allocator);

			allocator.Destruct(&batchData);
		}

		void RendererScene::UpdateCombinedRendererExtensionsIfNeeded(const Set<RendererExtension*, RendererExtension::SortFunction>& globalRendererExtensions, bool forceUpdate)
		{
			if(!forceUpdate && !mCombinedRendererExtensionsDirty)
				return;

			mCombinedRendererExtensions.clear();

			for(const auto& entry : globalRendererExtensions)
				mCombinedRendererExtensions.insert(entry);

			for(const auto& entry : mRendererExtensions)
				mCombinedRendererExtensions.insert(entry);

			mCombinedRendererExtensionsDirty = false;
		}
	}
}
