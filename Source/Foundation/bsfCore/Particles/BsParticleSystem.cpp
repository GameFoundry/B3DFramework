//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleSystem.h"
#include "Particles/BsParticleManager.h"
#include "Particles/BsParticleEmitter.h"
#include "Particles/BsParticleEvolver.h"
#include "Private/Particles/BsParticleSet.h"
#include "Private/RTTI/BsParticleSystemRTTI.h"
#include "Allocators/BsPoolAlloc.h"
#include "Material/BsMaterial.h"
#include "Renderer/BsCamera.h"
#include "Renderer/BsRenderer.h"
#include "Physics/BsPhysics.h"
#include "Particles/BsVectorField.h"
#include "Mesh/BsMesh.h"
#include "CoreObject/BsCoreObjectSync.h"
#include "Renderer/BsRendererScene.h"
#include "Scene/BsSceneManager.h"

using namespace b3d;

static constexpr u32 kInitialParticleCapacity = 1000;

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

ParticleSystem::ParticleSystem(const SPtr<SceneInstance>& scene)
	:SceneActor(scene)
{
	mId = ParticleManager::Instance().RegisterParticleSystem(this);
	mSeed = rand();

	auto emitter = B3DMakeShared<ParticleEmitter>();

	PARTICLE_SPHERE_SHAPE_DESC desc;
	desc.Radius = 0.05f;

	emitter->SetShape(ParticleEmitterSphereShape::Create(desc));

	mEmitters = { emitter };
}

ParticleSystem::ParticleSystem()
	:ParticleSystem(nullptr)
{ }

ParticleSystem::~ParticleSystem()
{
	ParticleManager::Instance().UnregisterParticleSystem(this);

	if(mParticleSet)
		B3DDelete(mParticleSet);
}

void ParticleSystem::SetSettings(const ParticleSystemSettings& settings)
{
	if(settings.UseAutomaticSeed != mSettings.UseAutomaticSeed)
	{
		if(settings.UseAutomaticSeed)
			mSeed = rand();
		else
			mSeed = settings.ManualSeed;

		mRandom.SetSeed(mSeed);
	}
	else
	{
		if(!settings.UseAutomaticSeed)
		{
			mSeed = settings.ManualSeed;
			mRandom.SetSeed(mSeed);
		}
	}

	if(settings.MaxParticles < mSettings.MaxParticles)
		mParticleSet->Clear(settings.MaxParticles);

	mSettings = settings;
	MarkSceneActorRenderProxyDataDirty();
	MarkDependenciesDirty();
}

void ParticleSystem::SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
{
	mGpuSimulationSettings = settings;
	MarkSceneActorRenderProxyDataDirty();
}

void ParticleSystem::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		B3D_LOG(Warning, Particles, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	MarkSceneActorRenderProxyDataDirty();
}

void ParticleSystem::SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters)
{
	mEmitters = emitters;
	MarkSceneActorRenderProxyDataDirty();
}

void ParticleSystem::SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers)
{
	mEvolvers = evolvers;

	std::sort(mEvolvers.begin(), mEvolvers.end(), [](const SPtr<ParticleEvolver>& a, const SPtr<ParticleEvolver>& b)
			  {
			i32 priorityA = a ? a->GetProperties().Priority : 0;
			i32 priorityB = b ? b->GetProperties().Priority : 0;

			if (priorityA == priorityB)
				return a > b; // Use address, at this point it doesn't matter, but sorting requires us to differentiate
			else
				return priorityA > priorityB; });

	MarkSceneActorRenderProxyDataDirty();
}

void ParticleSystem::Play()
{
	if(mState == State::Playing)
		return;

	if(mState == State::Uninitialized)
	{
		u32 particleCapacity = std::min(mSettings.MaxParticles, kInitialParticleCapacity);
		mParticleSet = B3DNew<ParticleSet>(particleCapacity);
	}

	mState = State::Playing;
	mTime = 0.0f;
	mRandom.SetSeed(mSeed);
}

void ParticleSystem::Pause()
{
	if(mState == State::Playing)
		mState = State::Paused;
}

void ParticleSystem::Stop()
{
	if(mState != State::Playing && mState != State::Paused)
		return;

	mState = State::Stopped;
	mParticleSet->Clear();
}

void ParticleSystem::SimulateInternal(float timeDelta, const EvaluatedAnimationData* animData)
{
	if(mState != State::Playing)
		return;

	float timeStep;
	const float newTime = AdvanceTimeInternal(mTime, timeDelta, mSettings.Duration, mSettings.IsLooping, timeStep);

	if(timeStep < 0.00001f)
		return;

	// Generate per-frame state
	ParticleSystemState state;
	state.TimeStart = mTime;
	state.TimeEnd = newTime;
	state.NrmTimeStart = state.TimeStart / mSettings.Duration;
	state.NrmTimeEnd = state.TimeEnd / mSettings.Duration;
	state.Length = mSettings.Duration;
	state.TimeStep = timeStep;
	state.MaxParticles = mSettings.MaxParticles;
	state.WorldSpace = mSettings.SimulationSpace == ParticleSimulationSpace::World;
	state.GpuSimulated = mSettings.GpuSimulation;
	state.LocalToWorld = mTransform.GetMatrix();
	state.WorldToLocal = state.LocalToWorld.InverseAffine();
	state.System = this;
	state.Scene = (mScene && mScene->IsActive()) ? mScene.get() : GetSceneManager().GetMainScene().get();
	state.AnimData = animData;

	// For GPU simulation we only care about newly spawned particles, so clear old ones
	if(mSettings.GpuSimulation)
		mParticleSet->Clear();

	// Spawn new particles
	for(auto& emitter : mEmitters)
	{
		if(emitter)
			emitter->Spawn(mRandom, state, *mParticleSet);
	}

	// Simulate if running on CPU, otherwise just pass the spawned particles off to the render thread
	if(!mSettings.GpuSimulation)
	{
		const u32 numParticles = mParticleSet->GetParticleCount();

		PreSimulate(state, 0, numParticles, false, 0.0f);
		Simulate(state, 0, numParticles, false, 0.0f);
		PostSimulate(state, 0, numParticles, false, 0.0f);
	}

	mTime = newTime;
}

void ParticleSystem::PreSimulate(const ParticleSystemState& state, u32 startIdx, u32 count, bool spacing, float spacingOffset)
{
	const ParticleSetData& particles = mParticleSet->GetParticles();
	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
	const u32 endIdx = startIdx + count;

	// Decrement lifetime
	for(u32 i = startIdx; i < endIdx; i++)
	{
		float timeStep = state.TimeStep;
		if(spacing)
		{
			// Note: We're calculating this in a few places during a single frame. Store it and re-use?
			const u32 localIdx = i - startIdx;
			const float subFrameOffset = ((float)localIdx + spacingOffset) * subFrameSpacing;
			timeStep *= subFrameOffset;
		}

		particles.Lifetime[i] -= timeStep;
	}

	// Kill expired particles
	u32 numParticles = count;
	for(u32 i = 0; i < numParticles;)
	{
		const u32 particleIdx = startIdx + i;
		if(particles.Lifetime[particleIdx] <= 0.0f)
		{
			mParticleSet->FreeParticle(particleIdx);
			numParticles--;
		}
		else
			i++;
	}

	// Remember old positions
	for(u32 i = startIdx; i < endIdx; i++)
		particles.PrevPosition[i] = particles.Position[i];

	// Evolve pre-simulation
	for(auto& evolver : mEvolvers)
	{
		if(!evolver)
			continue;

		const ParticleEvolverProperties& props = evolver->GetProperties();
		if(props.Priority < 0)
			break;

		evolver->Evolve(mRandom, state, *mParticleSet, startIdx, count, spacing, spacingOffset);
	}
}

void ParticleSystem::Simulate(const ParticleSystemState& state, u32 startIdx, u32 count, bool spacing, float spacingOffset)
{
	const ParticleSetData& particles = mParticleSet->GetParticles();
	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
	const u32 endIdx = startIdx + count;

	for(u32 i = startIdx; i < endIdx; i++)
	{
		float timeStep = state.TimeStep;
		if(spacing)
		{
			const u32 localIdx = i - startIdx;
			const float subFrameOffset = ((float)localIdx + spacingOffset) * subFrameSpacing;
			timeStep *= subFrameOffset;
		}

		particles.Position[i] += particles.Velocity[i] * timeStep;
	}
}

void ParticleSystem::PostSimulate(const ParticleSystemState& state, u32 startIdx, u32 count, bool spacing, float spacingOffset)
{
	// Evolve post-simulation
	for(auto& evolver : mEvolvers)
	{
		if(!evolver)
			continue;

		const ParticleEvolverProperties& props = evolver->GetProperties();
		if(props.Priority >= 0)
			continue;

		evolver->Evolve(mRandom, state, *mParticleSet, startIdx, count, spacing, spacingOffset);
	}
}

AABox ParticleSystem::CalculateBoundsInternal() const
{
	// TODO - If evolvers are deterministic (as well as their properties), calculate the maximinal bounds in an
	// analytical way

	const u32 particleCount = mParticleSet->GetParticleCount();
	if(particleCount == 0)
		return AABox::kBoxEmpty;

	const ParticleSetData& particles = mParticleSet->GetParticles();
	AABox bounds(Vector3::kInf, -Vector3::kInf);
	for(u32 i = 0; i < particleCount; i++)
		bounds.Merge(particles.Position[i]);

	return bounds;
}

float ParticleSystem::AdvanceTimeInternal(float time, float timeDelta, float duration, bool loop, float& timeStep)
{
	timeStep = timeDelta;
	float newTime = time + timeStep;
	if(newTime >= duration)
	{
		if(loop)
			newTime = fmod(newTime, duration);
		else
		{
			timeStep = time - duration;
			newTime = duration;
		}
	}

	return newTime;
}

SPtr<render::RenderProxy> ParticleSystem::CreateRenderProxy() const
{
	render::ParticleSystem* renderProxy = new(B3DAllocate<render::ParticleSystem>()) render::ParticleSystem(B3DGetRenderProxy(mScene), mId);
	SPtr<render::ParticleSystem> renderProxyShared = B3DMakeSharedFromExisting<render::ParticleSystem>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

void ParticleSystem::MarkSceneActorRenderProxyDataDirty(ActorDirtyFlag flag)
{
	MarkRenderProxyDataDirty((u32)flag);
}

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(ParticleSystem, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(mSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(mGpuSimulationSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mLayer)
		B3D_SYNC_BLOCK_ENTRY_PACKET_BASE(SceneActor, SceneActorPacket)
	B3D_SYNC_BLOCK_END
}

RenderProxySyncPacket* ParticleSystem::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	SyncPacket* syncPacket = allocator.Construct<SyncPacket>(*this, allocator, flags);
	syncPacket->SceneActorPacket = CreateSceneActorRenderProxySyncPacket(allocator, flags);

	return syncPacket;
}

void ParticleSystem::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mSettings.Mesh.IsLoaded())
		dependencies.push_back(mSettings.Mesh.Get());

	if(mSettings.Material.IsLoaded())
		dependencies.push_back(mSettings.Material.Get());
}

SPtr<ParticleSystem> ParticleSystem::Create(const SPtr<SceneInstance>& scene)
{
	ParticleSystem* rawPtr = new(B3DAllocate<ParticleSystem>()) ParticleSystem(scene);
	SPtr<ParticleSystem> ptr = B3DMakeSharedFromExisting<ParticleSystem>(rawPtr);
	ptr->SetShared(ptr);
	ptr->Initialize();

	return ptr;
}

SPtr<ParticleSystem> ParticleSystem::CreateEmpty()
{
	ParticleSystem* rawPtr = new(B3DAllocate<ParticleSystem>()) ParticleSystem();
	SPtr<ParticleSystem> ptr = B3DMakeSharedFromExisting<ParticleSystem>(rawPtr);
	ptr->SetShared(ptr);

	return ptr;
}

RTTIType* ParticleSystem::GetRttiStatic()
{
	return ParticleSystemRTTI::Instance();
}

RTTIType* ParticleSystem::GetRtti() const
{
	return ParticleSystem::GetRttiStatic();
}

namespace b3d { namespace render
{
ParticleSystem::~ParticleSystem()
{
	if(mActive)
	{
		const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
		rendererScene->UnregisterParticleSystem(this);
	}
}

void ParticleSystem::Initialize()
{
	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	rendererScene->RegisterParticleSystem(this);
}

void ParticleSystem::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		B3D_LOG(Warning, Particles, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	MarkSceneActorRenderProxyDataDirty();
}

void ParticleSystem::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	auto* const syncPacket = data.GetSyncPacket<b3d::ParticleSystem::SyncPacket>();
	if(!syncPacket)
		return;

	bool oldIsActive = mActive;
	syncPacket->ApplySyncData(this);

	constexpr u32 updateEverythingFlag = (u32)ActorDirtyFlag::Everything | (u32)ActorDirtyFlag::Active | (u32)ActorDirtyFlag::Dependency;

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	if((syncPacket->Flags & updateEverythingFlag) != 0)
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				rendererScene->RegisterParticleSystem(this);
			else
				rendererScene->UnregisterParticleSystem(this);
		}
		else
		{
			if(mActive)
				rendererScene->UpdateParticleSystem(this, false);
		}
	}
	else if((syncPacket->Flags & ((u32)ActorDirtyFlag::Mobility | (u32)ActorDirtyFlag::Transform)) != 0)
		rendererScene->UpdateParticleSystem(this, true);
}
}}
