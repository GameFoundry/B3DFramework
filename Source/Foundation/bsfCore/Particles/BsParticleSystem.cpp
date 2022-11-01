//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
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
#include "CoreThread/BsCoreObjectSync.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

static constexpr u32 kInitialParticleCapacity = 1000;

RTTITypeBase* ParticleSystemSettings::GetRttiStatic()
{
	return ParticleSystemSettingsRTTI::Instance();
}

RTTITypeBase* ParticleSystemSettings::GetRtti() const
{
	return GetRttiStatic();
}

template <bool Core>
template <class P>
void TParticleSystemSettings<Core>::RttiEnumFields(P p)
{
	p(GpuSimulation);
	p(SimulationSpace);
	p(Orientation);
	p(OrientationPlaneNormal);
	p(OrientationLockY);
	p(Duration);
	p(IsLooping);
	p(SortMode);
	p(Material);
	p(UseAutomaticBounds);
	p(CustomBounds);
	p(RenderMode);
	p(Mesh);
}

template <bool Core>
template <class P>
void TParticleVectorFieldSettings<Core>::RttiEnumFields(P p)
{
	p(Intensity);
	p(Tightness);
	p(Scale);
	p(Offset);
	p(Rotation);
	p(RotationRate);
	p(TilingX);
	p(TilingY);
	p(TilingZ);
	p(VectorField);
}

RTTITypeBase* ParticleVectorFieldSettings::GetRttiStatic()
{
	return ParticleVectorFieldSettingsRTTI::Instance();
}

RTTITypeBase* ParticleVectorFieldSettings::GetRtti() const
{
	return GetRttiStatic();
}

template <class P>
void ParticleDepthCollisionSettings::RttiEnumFields(P p)
{
	p(Enabled);
	p(Restitution);
	p(Dampening);
	p(RadiusScale);
}

RTTITypeBase* ParticleDepthCollisionSettings::GetRttiStatic()
{
	return ParticleDepthCollisionSettingsRTTI::Instance();
}

RTTITypeBase* ParticleDepthCollisionSettings::GetRtti() const
{
	return GetRttiStatic();
}

template <bool Core>
template <class P>
void TParticleGpuSimulationSettings<Core>::RttiEnumFields(P p)
{
	p(ColorOverLifetime);
	p(SizeScaleOverLifetime);
	p(Acceleration);
	p(Drag);
	p(DepthCollision);
	p(VectorField);
};

RTTITypeBase* ParticleGpuSimulationSettings::GetRttiStatic()
{
	return ParticleGpuSimulationSettingsRTTI::Instance();
}

RTTITypeBase* ParticleGpuSimulationSettings::GetRtti() const
{
	return GetRttiStatic();
}

ParticleSystem::ParticleSystem()
{
	mId = ParticleManager::Instance().RegisterParticleSystem(this);
	mSeed = rand();

	auto emitter = B3DMakeShared<ParticleEmitter>();

	PARTICLE_SPHERE_SHAPE_DESC desc;
	desc.Radius = 0.05f;

	emitter->SetShape(ParticleEmitterSphereShape::Create(desc));

	mEmitters = { emitter };
}

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
	MarkCoreDirtyInternal();
	MarkDependenciesDirty();
}

void ParticleSystem::SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
{
	mGpuSimulationSettings = settings;
	MarkCoreDirtyInternal();
}

void ParticleSystem::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		BS_LOG(Warning, Particles, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	MarkCoreDirtyInternal();
}

void ParticleSystem::SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters)
{
	mEmitters = emitters;
	MarkCoreDirtyInternal();
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

	MarkCoreDirtyInternal();
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

	// Simulate if running on CPU, otherwise just pass the spawned particles off to the core thread
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

SPtr<ct::ParticleSystem> ParticleSystem::GetCore() const
{
	return std::static_pointer_cast<ct::ParticleSystem>(mCoreSpecific);
}

SPtr<ct::CoreObject> ParticleSystem::CreateCore() const
{
	ct::ParticleSystem* rawPtr = new(B3DAllocate<ct::ParticleSystem>()) ct::ParticleSystem(mId);
	SPtr<ct::ParticleSystem> ptr = B3DMakeSharedFromExisting<ct::ParticleSystem>(rawPtr);
	ptr->SetThisPtrInternal(ptr);

	return ptr;
}

void ParticleSystem::MarkCoreDirtyInternal(ActorDirtyFlag flag)
{
	MarkCoreDirty((u32)flag);
}

CoreSyncData ParticleSystem::SyncToCore(FrameAlloc* allocator)
{
	u32 size = B3DRTTISize(GetCoreDirtyFlags()).Bytes;
	size += csync_size((SceneActor&)*this);
	size += csync_size(mSettings);
	size += csync_size(mGpuSimulationSettings);
	size += B3DRTTISize(mLayer).Bytes;

	u8* data = allocator->Alloc(size);
	Bitstream stream(data, size);
	B3DRTTIWrite(GetCoreDirtyFlags(), stream);
	csync_write((SceneActor&)*this, stream);
	csync_write(mSettings, stream);
	csync_write(mGpuSimulationSettings, stream);
	B3DRTTIWrite(mLayer, stream);

	return CoreSyncData(data, size);
}

void ParticleSystem::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mSettings.Mesh.IsLoaded())
		dependencies.push_back(mSettings.Mesh.Get());

	if(mSettings.Material.IsLoaded())
		dependencies.push_back(mSettings.Material.Get());
}

SPtr<ParticleSystem> ParticleSystem::Create()
{
	SPtr<ParticleSystem> ptr = CreateEmpty();
	ptr->Initialize();

	return ptr;
}

SPtr<ParticleSystem> ParticleSystem::CreateEmpty()
{
	ParticleSystem* rawPtr = new(B3DAllocate<ParticleSystem>()) ParticleSystem();
	SPtr<ParticleSystem> ptr = B3DMakeCoreFromExisting<ParticleSystem>(rawPtr);
	ptr->SetThisPtrInternal(ptr);

	return ptr;
}

RTTITypeBase* ParticleSystem::GetRttiStatic()
{
	return ParticleSystemRTTI::Instance();
}

RTTITypeBase* ParticleSystem::GetRtti() const
{
	return ParticleSystem::GetRttiStatic();
}

namespace bs { namespace ct
{
ParticleSystem::~ParticleSystem()
{
	if(mActive)
		GetRenderer()->NotifyParticleSystemRemoved(this);
}

void ParticleSystem::Initialize()
{
	GetRenderer()->NotifyParticleSystemAdded(this);
}

void ParticleSystem::SetLayer(u64 layer)
{
	const bool isPow2 = layer && !((layer - 1) & layer);

	if(!isPow2)
	{
		BS_LOG(Warning, Particles, "Invalid layer provided. Only one layer bit may be set. Ignoring.");
		return;
	}

	mLayer = layer;
	MarkCoreDirtyInternal();
}

void ParticleSystem::SyncToCore(const CoreSyncData& data)
{
	Bitstream stream((uint8_t*)data.GetBuffer(), data.GetBufferSize());

	u32 dirtyFlags = 0;
	const bool oldIsActive = mActive;

	B3DRTTIRead(dirtyFlags, stream);
	csync_read((SceneActor&)*this, stream);
	csync_read(mSettings, stream);
	csync_read(mGpuSimulationSettings, stream);
	B3DRTTIRead(mLayer, stream);

	constexpr u32 updateEverythingFlag = (u32)ActorDirtyFlag::Everything | (u32)ActorDirtyFlag::Active | (u32)ActorDirtyFlag::Dependency;

	if((dirtyFlags & updateEverythingFlag) != 0)
	{
		if(oldIsActive != mActive)
		{
			if(mActive)
				GetRenderer()->NotifyParticleSystemAdded(this);
			else
				GetRenderer()->NotifyParticleSystemRemoved(this);
		}
		else
		{
			if(mActive)
				GetRenderer()->NotifyParticleSystemUpdated(this, false);
		}
	}
	else if((dirtyFlags & ((u32)ActorDirtyFlag::Mobility | (u32)ActorDirtyFlag::Transform)) != 0)
		GetRenderer()->NotifyParticleSystemUpdated(this, true);
}
}}
