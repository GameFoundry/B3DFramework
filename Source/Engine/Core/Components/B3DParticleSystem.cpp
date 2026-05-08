//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DParticleSystem.h"

#include "CoreObject/B3DCoreObjectSync.h"
#include "ECS/B3DRegistry.h"
#include "Private/Particles/B3DParticleSet.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneObjectFragments.h"
#include "Utility/B3DTime.h"
#include "RTTI/B3DParticleSystemRTTI.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"
#include "Mesh/B3DMesh.h"
#include "Particles/B3DVectorField.h"
#include "Material/B3DMaterial.h"
#include "Particles/B3DParticleScene.h"
#include "Particles/B3DParticleEvolver.h"

using namespace b3d;

RTTIType* ecs::ParticleSystem::GetRttiStatic()
{
	return ecs::ECSParticleSystemRTTI::Instance();
}

RTTIType* ecs::ParticleSystem::GetRtti() const
{
	return GetRttiStatic();
}

ecs::ParticleSystem& ecs::CreateParticleSystem(ecs::Registry& registry, ecs::Entity entity, const TShared<RendererScene>& rendererScene, const Transform& transform)
{
	ecs::ParticleSystemECSUtility::CreateFragmentsIfMissing(registry, entity);
	registry.AddComponent<ecs::WorldTransform>(entity, ecs::WorldTransform(transform));
	rendererScene->AllocateParticleSystemId(registry, entity);
	ecs::ParticleSystemECSUtility::MarkDirty(registry, entity);

	return registry.GetComponents<ecs::ParticleSystem>(entity);
}

void ecs::DestroyParticleSystem(ecs::Registry& registry, ecs::Entity entity)
{
	ecs::ParticleSystemECSUtility::RemoveFragments(registry, entity);
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(ParticleSystemSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(GpuSimulation)
		B3D_SYNC_BLOCK_ENTRY(SimulationSpace)
		B3D_SYNC_BLOCK_ENTRY(Orientation)
		B3D_SYNC_BLOCK_ENTRY(OrientationPlaneNormal)
		B3D_SYNC_BLOCK_ENTRY(OrientationLockY)
		B3D_SYNC_BLOCK_ENTRY(Duration)
		B3D_SYNC_BLOCK_ENTRY(IsLooping)
		B3D_SYNC_BLOCK_ENTRY(SortMode)
		B3D_SYNC_BLOCK_ENTRY(Material)
		B3D_SYNC_BLOCK_ENTRY(UseAutomaticBounds)
		B3D_SYNC_BLOCK_ENTRY(CustomBounds)
		B3D_SYNC_BLOCK_ENTRY(RenderMode)
		B3D_SYNC_BLOCK_ENTRY(Mesh)
	B3D_SYNC_BLOCK_END
}

RTTIType* ParticleSystemSettings::GetRttiStatic()
{
	return ParticleSystemSettingsRTTI::Instance();
}

RTTIType* ParticleSystemSettings::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(ParticleVectorFieldSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Intensity)
		B3D_SYNC_BLOCK_ENTRY(Tightness)
		B3D_SYNC_BLOCK_ENTRY(Scale)
		B3D_SYNC_BLOCK_ENTRY(Offset)
		B3D_SYNC_BLOCK_ENTRY(Rotation)
		B3D_SYNC_BLOCK_ENTRY(RotationRate)
		B3D_SYNC_BLOCK_ENTRY(TilingX)
		B3D_SYNC_BLOCK_ENTRY(TilingY)
		B3D_SYNC_BLOCK_ENTRY(TilingZ)
		B3D_SYNC_BLOCK_ENTRY(VectorField)
	B3D_SYNC_BLOCK_END
}

RTTIType* ParticleVectorFieldSettings::GetRttiStatic()
{
	return ParticleVectorFieldSettingsRTTI::Instance();
}

RTTIType* ParticleVectorFieldSettings::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(ParticleDepthCollisionSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(Restitution)
		B3D_SYNC_BLOCK_ENTRY(Dampening)
		B3D_SYNC_BLOCK_ENTRY(RadiusScale)
	B3D_SYNC_BLOCK_END
}

RTTIType* ParticleDepthCollisionSettings::GetRttiStatic()
{
	return ParticleDepthCollisionSettingsRTTI::Instance();
}

RTTIType* ParticleDepthCollisionSettings::GetRtti() const
{
	return GetRttiStatic();
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(ParticleGpuSimulationSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(ColorOverLifetime)
		B3D_SYNC_BLOCK_ENTRY(SizeScaleOverLifetime)
		B3D_SYNC_BLOCK_ENTRY(Acceleration)
		B3D_SYNC_BLOCK_ENTRY(Drag)
		B3D_SYNC_BLOCK_ENTRY(DepthCollision)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(VectorField, SyncPacket)
	B3D_SYNC_BLOCK_END
}

RTTIType* ParticleGpuSimulationSettings::GetRttiStatic()
{
	return ParticleGpuSimulationSettingsRTTI::Instance();
}

RTTIType* ParticleGpuSimulationSettings::GetRtti() const
{
	return GetRttiStatic();
}

// New ECS-based sync blocks for particle system data

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::ParticleSystem, FullSyncPacket, TParticleSystemData<true>)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(Settings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(GpuSimulationSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Layer)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(u32, Id)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN_CUSTOM(ecs::ParticleSystem, TransformSyncPacket, TParticleSystemData<true>)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM(Transform, TransformData)
	B3D_SYNC_BLOCK_END

	struct ParticleSystemFullUpdateChannel : TRendererObjectECSSyncChannel
	<
		ParticleSystemFullUpdateChannel,
		ecs::ParticleSystem::FullSyncPacket,
		ecs::ParticleSystemDirty,
		ecs::ParticleSystem, ecs::WorldTransform, ecs::ParticleSystemId
	>
	{
		void Write(ParticleSystemObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToCreate(&allocator);
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToDestroy(&allocator);

			WritePackets(storage, allocator, [&renderStatesToCreate, &renderStatesToDestroy, &storage](ecs::ParticleSystem::FullSyncPacket& packet, PackedRendererId rendererId)
			{
				render::ParticleSystemProxy& proxy = storage.GetParticleSystemProxy(rendererId);

				bool wasRegistered = proxy.mRendererId != kInvalidPackedRendererId;
				proxy.mRendererId = rendererId;
				packet.ApplySyncData(&proxy.mData);

				proxy.mTransform = packet.TransformData;
				proxy.mId = packet.Id;

				if(wasRegistered)
					renderStatesToDestroy.push_back(rendererId);

				renderStatesToCreate.push_back(rendererId);
			});

			if(!renderStatesToDestroy.empty())
				storage.DestroyRenderState(renderStatesToDestroy);

			if(!renderStatesToCreate.empty())
				storage.CreateRenderState(renderStatesToCreate);
		}

		void CreateAndPopulatePacket(ecs::ParticleSystem& fragment, ecs::WorldTransform& transform, ecs::ParticleSystemId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
			packet.Id = fragment.Id;
		}
	};

	struct ParticleSystemTransformUpdateChannel : TRendererObjectECSSyncChannel
	<
		ParticleSystemTransformUpdateChannel,
		ecs::ParticleSystem::TransformSyncPacket,
		ecs::ParticleSystemTransformDirty,
		ecs::ParticleSystem, ecs::WorldTransform, ecs::ParticleSystemId
	>
	{
		void Write(ParticleSystemObjectStorageBase& storage, FrameAllocator& allocator)
		{
			Vector<PackedRendererId, StdFrameAlloc<PackedRendererId>> renderStatesToUpdate(&allocator);

			WritePackets(storage, allocator, [&renderStatesToUpdate, &storage](ecs::ParticleSystem::TransformSyncPacket& packet, PackedRendererId rendererId)
			{
				render::ParticleSystemProxy& proxy = storage.GetParticleSystemProxy(rendererId);
				proxy.mTransform = packet.TransformData;

				renderStatesToUpdate.push_back(rendererId);
			});

			if(!renderStatesToUpdate.empty())
				storage.UpdateRenderState(renderStatesToUpdate);
		}

		void CreateAndPopulatePacket(ecs::ParticleSystem& fragment, ecs::WorldTransform& transform, ecs::ParticleSystemId& id, FrameAllocator& allocator)
		{
			auto& packet = CreatePacket(id.Id, fragment, allocator, 0);
			packet.TransformData = transform;
		}
	};

	using ParticleSystemSyncBatch = TRendererObjectECSSyncBatch<ParticleSystemFullUpdateChannel, ParticleSystemTransformUpdateChannel>;
}

// ParticleSystemObjectStorageBase

void* ParticleSystemObjectStorageBase::SyncRead(ecs::Registry& registry, FrameAllocator& allocator)
{
	return ParticleSystemSyncBatch::Read(*this, registry, allocator);
}

void ParticleSystemObjectStorageBase::SyncWrite(void* batchData, FrameAllocator& allocator)
{
	ParticleSystemSyncBatch::Write(*this, batchData, allocator);
}

ParticleSystem::ParticleSystem(const HSceneObject& parent)
	: Component(parent)
{
	SetName("ParticleSystem");
	SetFlag(ComponentFlag::AlwaysRun, true);
	mNotifyFlags = TCF_Transform;
}

ParticleSystem::ParticleSystem()
	: ParticleSystem(nullptr)
{ }

void ParticleSystem::SetSettings(const ParticleSystemSettings& settings)
{
	ecs::ParticleSimulation& simulation = GetSimulationFragment();
	const ParticleSystemSettings& oldSettings = GetSettings();

	if(settings.UseAutomaticSeed != oldSettings.UseAutomaticSeed)
	{
		if(settings.UseAutomaticSeed)
			simulation.Seed = rand();
		else
			simulation.Seed = settings.ManualSeed;

		simulation.Rng.SetSeed(simulation.Seed);
	}
	else
	{
		if(!settings.UseAutomaticSeed)
		{
			simulation.Seed = settings.ManualSeed;
			simulation.Rng.SetSeed(simulation.Seed);
		}
	}

	if(simulation.Particles && settings.MaxParticles < oldSettings.MaxParticles)
		simulation.Particles->Clear(settings.MaxParticles);

	GetFragment().Settings = settings;
	MarkRenderProxyDataDirty();
	MarkDependenciesDirty();
}

void ParticleSystem::SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
{
	GetFragment().GpuSimulationSettings = settings;
	MarkRenderProxyDataDirty();
}

void ParticleSystem::SetEvolvers(const Vector<TShared<ParticleEvolver>>& evolvers)
{
	ecs::ParticleSystem& config = GetFragment();
	config.Evolvers = evolvers;

	std::sort(config.Evolvers.begin(), config.Evolvers.end(), [](const TShared<ParticleEvolver>& a, const TShared<ParticleEvolver>& b)
			  {
			const i32 priorityA = a ? a->GetProperties().Priority : 0;
			const i32 priorityB = b ? b->GetProperties().Priority : 0;

			if (priorityA == priorityB)
				return a > b; // Use address, at this point it doesn't matter, but sorting requires us to differentiate
			else
				return priorityA > priorityB; });

	MarkRenderProxyDataDirty();
}

const Vector<TShared<ParticleEvolver>>& ParticleSystem::GetEvolvers() const
{
	return GetFragment().Evolvers;
}

void ParticleSystem::SetEmitters(const Vector<TShared<ParticleEmitter>>& emitters)
{
	GetFragment().Emitters = emitters;
	MarkRenderProxyDataDirty();
}

const Vector<TShared<ParticleEmitter>>& ParticleSystem::GetEmitters() const
{
	return GetFragment().Emitters;
}

void ParticleSystem::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		B3D_LOG(Warning, LogParticles, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	GetFragment().Layer = layer;
	MarkRenderProxyDataDirty();
}

void ParticleSystem::Play()
{
	const TShared<ParticleScene>& particleScene = SceneObject()->GetScene()->GetParticleScene();
	particleScene->Play(GetSimulationFragment(), GetSettings());
}

void ParticleSystem::Pause()
{
	const TShared<ParticleScene>& particleScene = SceneObject()->GetScene()->GetParticleScene();
	particleScene->Pause(GetSimulationFragment());
}

void ParticleSystem::Stop()
{
	const TShared<ParticleScene>& particleScene = SceneObject()->GetScene()->GetParticleScene();
	particleScene->Stop(GetSimulationFragment());
}

void ParticleSystem::Simulate(float timeDelta, const EvaluatedAnimationData* animData)
{
	const TShared<ParticleScene>& particleScene = SceneObject()->GetScene()->GetParticleScene();
	const ecs::WorldTransform& worldTransformFragment = GetECSRegistry()->GetComponents<ecs::WorldTransform>(GetECSEntity());

	particleScene->AdvanceSimulation(GetSimulationFragment(), GetFragment(), worldTransformFragment, timeDelta, animData);
}

AABox ParticleSystem::CalculateBounds() const
{
	const TShared<ParticleScene>& particleScene = SceneObject()->GetScene()->GetParticleScene();
	return particleScene->CalculateBounds(GetSimulationFragment());
}

float ParticleSystem::AdvanceTime(float time, float timeDelta, float duration, bool loop, float& outTimeStep)
{
	outTimeStep = timeDelta;
	float newTime = time + outTimeStep;
	if(newTime >= duration)
	{
		if(loop)
			newTime = fmod(newTime, duration);
		else
		{
			outTimeStep = time - duration;
			newTime = duration;
		}
	}

	return newTime;
}

ecs::ParticleSystem& ParticleSystem::GetFragment()
{
	return GetECSRegistry()->GetComponents<ecs::ParticleSystem>(GetECSEntity());
}

const ecs::ParticleSystem& ParticleSystem::GetFragment() const
{
	return GetECSRegistry()->GetComponents<ecs::ParticleSystem>(GetECSEntity());
}

ecs::ParticleSimulation& ParticleSystem::GetSimulationFragment()
{
	return GetECSRegistry()->GetComponents<ecs::ParticleSimulation>(GetECSEntity());
}

const ecs::ParticleSimulation& ParticleSystem::GetSimulationFragment() const
{
	return GetECSRegistry()->GetComponents<ecs::ParticleSimulation>(GetECSEntity());
}

const TParticleSystemData<false>& ParticleSystem::GetParticleSystemData() const
{
	return GetFragment();
}

void ParticleSystem::MarkRenderProxyDataDirty(ComponentDirtyFlag flag)
{
	if(!SceneObject().IsValid())
		return;

	ecs::ParticleSystemECSUtility::MarkDirty(*GetECSRegistry(), GetECSEntity(), flag);
}

void ParticleSystem::Initialize()
{
	SetShared(B3DStaticGameObjectCast<ParticleSystem>(mThisHandle).GetShared());
	ecs::ParticleSystemECSUtility::CreateFragmentsIfMissing(*GetECSRegistry(), GetECSEntity());
	Component::Initialize();
	CoreObject::Initialize();
}

void ParticleSystem::OnCreated()
{
	const TShared<ParticleScene>& particleScene = SceneObject()->GetScene()->GetParticleScene();
	GetFragment().Id = particleScene->AllocateId();

	ecs::ParticleSimulation& simulation = GetSimulationFragment();
	simulation.Seed = rand();
}

void ParticleSystem::OnDestroyed()
{
	ecs::ParticleSystemECSUtility::RemoveFragments(*GetECSRegistry(), GetECSEntity());
	CoreObject::Destroy();
}

void ParticleSystem::OnDisabled()
{
	const TShared<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::ParticleSystemECSUtility::UnregisterFromRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);
	Stop();
}

void ParticleSystem::OnEnabled()
{
	if(mPreviewMode)
	{
		Stop();
		mPreviewMode = false;
	}

	const TShared<RendererScene>& rendererScene = SceneObject()->GetScene()->GetRendererScene();
	ecs::ParticleSystemECSUtility::RegisterWithRenderer(*GetECSRegistry(), GetECSEntity(), rendererScene);

	const TShared<SceneInstance>& scene = SceneObject()->GetScene();
	if(scene->IsRunning())
		Play();
}

void ParticleSystem::OnSceneChanged(SceneInstance* oldScene, ecs::Entity oldEntity)
{
	ecs::ParticleSystemECSUtility::ChangeScene(oldScene, oldEntity, *SceneObject()->GetScene(), GetECSEntity(), GetEnabled());
}

void ParticleSystem::OnTransformChanged(TransformChangedFlags flags)
{
	MarkRenderProxyDataDirty(ComponentDirtyFlag::Transform);
}

void ParticleSystem::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	const ecs::ParticleSystem& fragment = GetFragment();

	if(fragment.Settings.Mesh.IsLoaded())
		dependencies.push_back(fragment.Settings.Mesh.Get());

	if(fragment.Settings.Material.IsLoaded())
		dependencies.push_back(fragment.Settings.Material.Get());
}

bool ParticleSystem::TogglePreviewMode(bool enabled)
{
	const TShared<SceneInstance>& scene = SceneObject()->GetScene();
	const bool isRunning = scene->IsRunning();

	if(enabled)
	{
		// Cannot enable preview while running
		if(isRunning)
			return false;

		if(!mPreviewMode)
		{
			Play();
			mPreviewMode = true;
		}

		return true;
	}
	else
	{
		if(!isRunning)
			Stop();

		mPreviewMode = false;
		return false;
	}
}

RTTIType* ParticleSystem::GetRttiStatic()
{
	return ParticleSystemRTTI::Instance();
}

RTTIType* ParticleSystem::GetRtti() const
{
	return ParticleSystem::GetRttiStatic();
}
