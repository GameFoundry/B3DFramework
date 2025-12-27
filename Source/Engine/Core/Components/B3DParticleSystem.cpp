//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DParticleSystem.h"

#include "CoreObject/B3DCoreObjectSync.h"
#include "Private/Particles/B3DParticleSet.h"
#include "Scene/B3DSceneObject.h"
#include "Utility/B3DTime.h"
#include "RTTI/B3DParticleSystemRTTI.h"
#include "Renderer/B3DRendererScene.h"
#include "Scene/B3DSceneInstance.h"
#include "Mesh/B3DMesh.h"
#include "Particles/B3DVectorField.h"
#include "Material/B3DMaterial.h"
#include "Particles/B3DParticleScene.h"

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

namespace b3d
{
	B3D_SYNC_BLOCK_BEGIN(ParticleSystem, FullSyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(mSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(mGpuSimulationSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(mLayer)
		B3D_SYNC_BLOCK_ENTRY(mId)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(bool, mActive)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(SPtr<SceneInstance>, mSceneInstance)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(ParticleSystem, TransformSyncPacket)
		B3D_SYNC_BLOCK_ENTRY_CUSTOM_SETTER(Transform, mTransform)
	B3D_SYNC_BLOCK_END
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
	mSettings = settings;

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
	MarkRenderProxyDataDirty();
	MarkDependenciesDirty();
}

void ParticleSystem::SetGpuSimulationSettings(const ParticleGpuSimulationSettings& settings)
{
	mGpuSimulationSettings = settings;
	MarkRenderProxyDataDirty();
}

void ParticleSystem::SetEvolvers(const Vector<SPtr<ParticleEvolver>>& evolvers)
{
	mEvolvers = evolvers;

	std::sort(mEvolvers.begin(), mEvolvers.end(), [](const SPtr<ParticleEvolver>& a, const SPtr<ParticleEvolver>& b)
			  {
			const i32 priorityA = a ? a->GetProperties().Priority : 0;
			const i32 priorityB = b ? b->GetProperties().Priority : 0;

			if (priorityA == priorityB)
				return a > b; // Use address, at this point it doesn't matter, but sorting requires us to differentiate
			else
				return priorityA > priorityB; });

	MarkRenderProxyDataDirty();
}

void ParticleSystem::SetEmitters(const Vector<SPtr<ParticleEmitter>>& emitters)
{
	mEmitters = emitters;
	MarkRenderProxyDataDirty();
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
	MarkRenderProxyDataDirty();
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


void ParticleSystem::Simulate(float timeDelta, const EvaluatedAnimationData* animData)
{
	if(mState != State::Playing)
		return;

	float timeStep;
	const float newTime = AdvanceTime(mTime, timeDelta, mSettings.Duration, mSettings.IsLooping, timeStep);

	if(timeStep < 0.00001f)
		return;

	const HSceneObject& sceneObject = SceneObject();

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
	state.LocalToWorld = sceneObject->GetTransform().GetMatrix();
	state.WorldToLocal = state.LocalToWorld.InverseAffine();
	state.System = this;
	state.Scene = sceneObject->GetScene().get();
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
		const u32 particleCount = mParticleSet->GetParticleCount();

		PreSimulate(state, 0, particleCount, false, 0.0f);
		Simulate(state, 0, particleCount, false, 0.0f);
		PostSimulate(state, 0, particleCount, false, 0.0f);
	}

	mTime = newTime;
}

AABox ParticleSystem::CalculateBounds() const
{
	// TODO - If evolvers are deterministic (as well as their properties), calculate the maximal bounds in an
	// analytical way

	const u32 particleCount = mParticleSet->GetParticleCount();
	if(particleCount == 0)
		return AABox::kEmpty;

	const ParticleSetData& particles = mParticleSet->GetParticles();
	AABox bounds(Vector3(-(float)kMaximumSceneExtent), Vector3((float)kMaximumSceneExtent));
	for(u32 particleIndex = 0; particleIndex < particleCount; particleIndex++)
		bounds.Merge(particles.Position[particleIndex]);

	return bounds;
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

void ParticleSystem::PreSimulate(const ParticleSystemState& state, u32 startIndex, u32 count, bool spacing, float spacingOffset)
{
	ParticleSetData& particles = mParticleSet->GetParticles();
	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
	const u32 endIndex = startIndex + count;

	// Decrement lifetime
	for(u32 particleIndex = startIndex; particleIndex < endIndex; particleIndex++)
	{
		float timeStep = state.TimeStep;
		if(spacing)
		{
			// Note: We're calculating this in a few places during a single frame. Store it and re-use?
			const u32 localIndex = particleIndex - startIndex;
			const float subFrameOffset = ((float)localIndex + spacingOffset) * subFrameSpacing;
			timeStep *= subFrameOffset;
		}

		particles.Lifetime[particleIndex] -= timeStep;
	}

	// Kill expired particles
	u32 particleCount = count;
	for(u32 particleSubIndex = 0; particleSubIndex < particleCount;)
	{
		const u32 particleIndex = startIndex + particleSubIndex;
		if(particles.Lifetime[particleIndex] <= 0.0f)
		{
			mParticleSet->FreeParticle(particleIndex);
			particleCount--;
		}
		else
			particleSubIndex++;
	}

	// Remember old positions
	for(u32 particleIndex = startIndex; particleIndex < endIndex; particleIndex++)
		particles.PrevPosition[particleIndex] = particles.Position[particleIndex];

	// Evolve pre-simulation
	for(auto& evolver : mEvolvers)
	{
		if(!evolver)
			continue;

		const ParticleEvolverProperties& props = evolver->GetProperties();
		if(props.Priority < 0)
			break;

		evolver->Evolve(mRandom, state, *mParticleSet, startIndex, count, spacing, spacingOffset);
	}
}

void ParticleSystem::Simulate(const ParticleSystemState& state, u32 startIndex, u32 count, bool spacing, float spacingOffset)
{
	ParticleSetData& particles = mParticleSet->GetParticles();
	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
	const u32 endIndex = startIndex + count;

	for(u32 particleIndex = startIndex; particleIndex < endIndex; particleIndex++)
	{
		float timeStep = state.TimeStep;
		if(spacing)
		{
			const u32 localIndex = particleIndex - startIndex;
			const float subFrameOffset = ((float)localIndex + spacingOffset) * subFrameSpacing;
			timeStep *= subFrameOffset;
		}

		particles.Position[particleIndex] += particles.Velocity[particleIndex] * timeStep;
	}
}

void ParticleSystem::PostSimulate(const ParticleSystemState& state, u32 startIndex, u32 count, bool spacing, float spacingOffset)
{
	// Evolve post-simulation
	for(auto& evolver : mEvolvers)
	{
		if(!evolver)
			continue;

		const ParticleEvolverProperties& props = evolver->GetProperties();
		if(props.Priority >= 0)
			continue;

		evolver->Evolve(mRandom, state, *mParticleSet, startIndex, count, spacing, spacingOffset);
	}
}

SPtr<render::RenderProxy> ParticleSystem::CreateRenderProxy() const
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();

	render::ParticleSystem* renderProxy = new(B3DAllocate<render::ParticleSystem>()) render::ParticleSystem(B3DGetRenderProxy(scene), mId);
	SPtr<render::ParticleSystem> renderProxyShared = B3DMakeSharedFromExisting<render::ParticleSystem>(renderProxy);
	renderProxyShared->SetShared(renderProxyShared);

	return renderProxyShared;
}

RenderProxySyncPacket* ParticleSystem::CreateRenderProxySyncPacket(FrameAllocator& allocator, u32 flags)
{
	if(flags != (u32)ComponentDirtyFlag::Transform)
	{
		FullSyncPacket* const syncPacket = allocator.Construct<FullSyncPacket>(*this, allocator, flags);
		syncPacket->mActive = GetEnabled();
		syncPacket->mSceneInstance = B3DGetRenderProxy(SceneObject()->GetScene());
		syncPacket->mTransform = SceneObject()->GetTransform();

		return syncPacket;
	}
	else
	{
		TransformSyncPacket* const syncPacket = allocator.Construct<TransformSyncPacket>(*this, allocator, flags);
		syncPacket->mTransform = SceneObject()->GetTransform();

		return syncPacket;
	}
}

void ParticleSystem::GetCoreDependencies(Vector<CoreObject*>& dependencies)
{
	if(mSettings.Mesh.IsLoaded())
		dependencies.push_back(mSettings.Mesh.Get());

	if(mSettings.Material.IsLoaded())
		dependencies.push_back(mSettings.Material.Get());
}

void ParticleSystem::Initialize()
{
	SetShared(B3DStaticGameObjectCast<ParticleSystem>(mThisHandle).GetShared());

	Component::Initialize();
	CoreObject::Initialize();
}

void ParticleSystem::OnCreated()
{
	const SPtr<ParticleScene>& particleScene = SceneObject()->GetScene()->GetParticleScene();

	mId = particleScene->RegisterParticleSystem(this);
	mSeed = rand();

	// Makes sure the ID is synced to the render thread
	MarkRenderProxyDataDirty();
}

void ParticleSystem::OnDestroyed()
{
	const SPtr<ParticleScene>& particleScene = SceneObject()->GetScene()->GetParticleScene();
	particleScene->UnregisterParticleSystem(this);

	CoreObject::Destroy();
}

void ParticleSystem::OnDisabled()
{
	Stop();
	MarkRenderProxyDataDirty();
}

void ParticleSystem::OnEnabled()
{
	if(mPreviewMode)
	{
		Stop();
		mPreviewMode = false;
	}

	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
	if(scene->IsRunning())
	{
		Play();
	}

	MarkRenderProxyDataDirty();
}

void ParticleSystem::OnTransformChanged(TransformChangedFlags flags)
{
	MarkRenderProxyDataDirty();
}

bool ParticleSystem::TogglePreviewMode(bool enabled)
{
	const SPtr<SceneInstance>& scene = SceneObject()->GetScene();
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

	RenderProxy::Initialize();
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
}

void ParticleSystem::SyncFromCoreObject(const CoreSyncData& data, FrameAllocator& allocator)
{
	RenderProxySyncPacket* const syncPacket = data.GetSyncPacket();
	if(syncPacket == nullptr)
		return;

	const bool wasActive = mActive;
	syncPacket->ApplySyncData(this);

	const SPtr<RendererScene>& rendererScene = mSceneInstance->GetRendererScene();
	const u32 flags = syncPacket->Flags;
	const u32 updateEverythingFlag = ~(u32)ComponentDirtyFlag::Transform;
	if((flags & updateEverythingFlag) != 0)
	{
		if(wasActive != mActive)
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
	else
	{
		if(mActive)
			rendererScene->UpdateParticleSystem(this, true);
	}
}
}}
