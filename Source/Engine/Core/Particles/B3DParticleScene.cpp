//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/B3DParticleScene.h"

#include "B3DApplication.h"
#include "Utility/B3DTime.h"
#include "Allocators/B3DPoolAlloc.h"
#include "Private/Particles/B3DParticleSet.h"
#include "Animation/B3DAnimationScene.h"
#include "Components/B3DParticleSystem.h"
#include "Image/B3DPixelUtility.h"
#include "Scene/B3DSceneInstance.h"
#include "Scene/B3DSceneObjectFragments.h"
#include "ECS/B3DRegistry.h"
#include "Particles/B3DParticleEmitter.h"
#include "Particles/B3DParticleEvolver.h"

using namespace b3d;

static constexpr u32 kInitialParticleCapacity = 1000;

/** Helper method used for writing particle data into the @p pixels buffer. */
template <class T, class PR>
void IterateOverPixels(PixelData& pixels, u32 count, u32 stride, PR predicate)
{
	auto dest = (u8*)pixels.GetData();

	u32 x = 0;
	for(u32 i = 0; i < count; i++)
	{
		predicate((T*)dest, i);

		dest += stride;
		x++;

		if(x >= pixels.GetWidth())
		{
			x = 0;
			dest += pixels.GetRowSkip();
		}
	}
}

/** Helper method used for writing particle data into the @p pixels buffer. */
template <class T, class PR>
void IterateOverPixels(PixelData& pixels, u32 count, PR predicate)
{
	IterateOverPixels<T>(pixels, count, sizeof(T), predicate);
}

/**
 * Maintains a pool of buffers that are used for passing results of particle simulation from the simulation to the
 * render thread.
 */
class ParticleSimulationDataPool
{
	/** Contains a list of buffers for the specified size. */
	struct BuffersPerSize
	{
		Vector<ParticleRenderData*> Buffers;
		u32 NextFreeIdx = 0;
	};

public:
	~ParticleSimulationDataPool()
	{
		Lock lock(mMutex);

		for(auto& sizeEntry : mBillboardBufferList)
		{
			for(auto& entry : sizeEntry.second.Buffers)
				mBillboardAlloc.Destruct(static_cast<ParticleBillboardRenderData*>(entry));
		}

		for(auto& sizeEntry : mMeshBufferList)
		{
			for(auto& entry : sizeEntry.second.Buffers)
				mMeshAlloc.Destruct(static_cast<ParticleMeshRenderData*>(entry));
		}

		for(auto& entry : mGPUBufferList)
			mGPUAlloc.Destruct(entry);
	}

	/**
	 * Returns a set of buffers containing particle data from the provided particle set. Usable for rendering the
	 * results of the CPU particle simulation as billboards.
	 */
	ParticleBillboardRenderData* AllocCpuBillboard(const ParticleSet& particleSet)
	{
		const u32 size = particleSet.DetermineTextureSize();

		ParticleBillboardRenderData* output = nullptr;

		{
			Lock lock(mMutex);

			BuffersPerSize& buffers = mBillboardBufferList[size];
			if(buffers.NextFreeIdx < (u32)buffers.Buffers.size())
			{
				output = static_cast<ParticleBillboardRenderData*>(buffers.Buffers[buffers.NextFreeIdx]);
				buffers.NextFreeIdx++;
			}
		}

		if(!output)
		{
			output = CreateNewBillboardBuffersCpu(size);

			Lock lock(mMutex);

			BuffersPerSize& buffers = mBillboardBufferList[size];
			buffers.Buffers.push_back(output);
			buffers.NextFreeIdx++;
		}

		// Populate buffer contents
		const u32 count = particleSet.GetParticleCount();
		const ParticleSetData& particles = particleSet.GetParticles();

		// TODO: Use non-temporal writes?
		IterateOverPixels<Vector4>(output->PositionAndRotation, count, [&particles](Vector4* dst, u32 idx)
								   {
										   dst->X = particles.Position[idx].X;
										   dst->Y = particles.Position[idx].Y;
										   dst->Z = particles.Position[idx].Z;
										   dst->W = particles.Rotation[idx].X * Math::kDeG2Rad; });

		IterateOverPixels<RGBA>(output->Color, count, [&particles](RGBA* dst, u32 idx)
								{ *dst = particles.Color[idx]; });

		IterateOverPixels<u16>(output->SizeAndFrameIdx, count, sizeof(u16) * 4, [&particles](u16* dst, u32 idx)
							   {
				dst[0] = Bitwise::FloatToHalf(particles.Size[idx].X);
				dst[1] = Bitwise::FloatToHalf(particles.Size[idx].Y);
				dst[2] = Bitwise::FloatToHalf(particles.Frame[idx]); });

		output->Indices.clear();
		output->Indices.resize(count);

		return output;
	}

	/**
	 * Returns a set of buffers containing particle data from the provided particle set. Usable for rendering the
	 * results of the CPU particle simulation as 3D meshes.
	 */
	ParticleMeshRenderData* AllocCpuMesh(const ParticleSet& particleSet)
	{
		const u32 size = particleSet.DetermineTextureSize();

		ParticleMeshRenderData* output = nullptr;

		{
			Lock lock(mMutex);

			BuffersPerSize& buffers = mMeshBufferList[size];
			if(buffers.NextFreeIdx < (u32)buffers.Buffers.size())
			{
				output = static_cast<ParticleMeshRenderData*>(buffers.Buffers[buffers.NextFreeIdx]);
				buffers.NextFreeIdx++;
			}
		}

		if(!output)
		{
			output = CreateNewMeshBuffersCpu(size);

			Lock lock(mMutex);

			BuffersPerSize& buffers = mMeshBufferList[size];
			buffers.Buffers.push_back(output);
			buffers.NextFreeIdx++;
		}

		// Populate buffer contents
		const u32 count = particleSet.GetParticleCount();
		const ParticleSetData& particles = particleSet.GetParticles();

		// TODO: Use non-temporal writes?
		IterateOverPixels<Vector4>(output->Position, count, [&particles](Vector4* dst, u32 idx)
								   {
										   dst->X = particles.Position[idx].X;
										   dst->Y = particles.Position[idx].Y;
										   dst->Z = particles.Position[idx].Z; });

		IterateOverPixels<RGBA>(output->Color, count, [&particles](RGBA* dst, u32 idx)
								{ *dst = particles.Color[idx]; });

		IterateOverPixels<u16>(output->Rotation, count, sizeof(u16) * 4, [&particles](u16* dst, u32 idx)
							   {
				dst[0] = Bitwise::FloatToHalf(particles.Rotation[idx].X * Math::kDeG2Rad);
				dst[1] = Bitwise::FloatToHalf(particles.Rotation[idx].Y * Math::kDeG2Rad);
				dst[2] = Bitwise::FloatToHalf(particles.Rotation[idx].Z * Math::kDeG2Rad); });

		IterateOverPixels<u16>(output->Size, count, sizeof(u16) * 4, [&particles](u16* dst, u32 idx)
							   {
				dst[0] = Bitwise::FloatToHalf(particles.Size[idx].X);
				dst[1] = Bitwise::FloatToHalf(particles.Size[idx].Y);
				dst[2] = Bitwise::FloatToHalf(particles.Size[idx].Z); });

		output->Indices.clear();
		output->Indices.resize(count);

		return output;
	}

	/**
	 * Returns a list of particles from the provided particle set that may be used for inserting the particles into the
	 * GPU simulation.
	 */
	ParticleGPUSimulationData* AllocGpu(const ParticleSet& particleSet)
	{
		ParticleGPUSimulationData* output = nullptr;

		{
			Lock lock(mMutex);

			if(mNextFreeGPUBuffer < (u32)mGPUBufferList.size())
			{
				output = mGPUBufferList[mNextFreeGPUBuffer];
				mNextFreeGPUBuffer++;
			}
		}

		if(!output)
		{
			output = CreateNewBuffersGpu();

			Lock lock(mMutex);

			mGPUBufferList.push_back(output);
			mNextFreeGPUBuffer++;
		}

		// Populate buffer contents
		const u32 count = particleSet.GetParticleCount();
		const ParticleSetData& particles = particleSet.GetParticles();

		output->Particles.clear();
		output->Particles.resize(count);

		// TODO: Use non-temporal writes?
		for(u32 i = 0; i < count; i++)
		{
			GpuParticle particle;
			particle.Position = particles.Position[i];
			particle.Lifetime = particles.Lifetime[i];
			particle.InitialLifetime = particles.InitialLifetime[i];
			particle.Velocity = particles.Velocity[i];
			particle.Size = Vector2(particles.Size[i].X, particles.Size[i].Z);
			particle.Rotation = particles.Rotation[i].Z;

			output->Particles[i] = particle;
		}

		return output;
	}

	/** Makes all the buffers available for allocations. Does not free internal buffer memory. */
	void Clear()
	{
		Lock lock(mMutex);

		for(auto& buffers : mBillboardBufferList)
			buffers.second.NextFreeIdx = 0;

		for(auto& buffers : mMeshBufferList)
			buffers.second.NextFreeIdx = 0;

		mNextFreeGPUBuffer = 0;
	}

private:
	/** Allocates a new set of CPU buffers used for billboard rendering of the provided @p size width and height. */
	ParticleBillboardRenderData* CreateNewBillboardBuffersCpu(u32 size)
	{
		auto output = mBillboardAlloc.Construct<ParticleBillboardRenderData>();

		output->PositionAndRotation = PixelData(size, size, 1, PF_RGBA32F);
		output->Color = PixelData(size, size, 1, PF_RGBA8);
		output->SizeAndFrameIdx = PixelData(size, size, 1, PF_RGBA16F);

		// Note: Potentially allocate them all in one large block
		output->PositionAndRotation.AllocateInternalBuffer();
		output->Color.AllocateInternalBuffer();
		output->SizeAndFrameIdx.AllocateInternalBuffer();

		return output;
	}

	/** Allocates a new set of CPU buffers used for mesh rendering of the provided @p size width and height. */
	ParticleMeshRenderData* CreateNewMeshBuffersCpu(u32 size)
	{
		auto output = mMeshAlloc.Construct<ParticleMeshRenderData>();

		output->Position = PixelData(size, size, 1, PF_RGBA32F);
		output->Color = PixelData(size, size, 1, PF_RGBA8);
		output->Size = PixelData(size, size, 1, PF_RGBA16F);
		output->Rotation = PixelData(size, size, 1, PF_RGBA16F);

		// Note: Potentially allocate them all in one large block
		output->Position.AllocateInternalBuffer();
		output->Color.AllocateInternalBuffer();
		output->Size.AllocateInternalBuffer();
		output->Rotation.AllocateInternalBuffer();

		return output;
	}

	/** Allocates a new set of GPU buffers of the provided @p size width and height. */
	ParticleGPUSimulationData* CreateNewBuffersGpu()
	{
		return mGPUAlloc.Construct<ParticleGPUSimulationData>();
	}

	UnorderedMap<u32, BuffersPerSize> mBillboardBufferList;
	UnorderedMap<u32, BuffersPerSize> mMeshBufferList;
	Vector<ParticleGPUSimulationData*> mGPUBufferList;
	u32 mNextFreeGPUBuffer = 0;

	PoolAlloc<sizeof(ParticleBillboardRenderData), 32, 4, true> mBillboardAlloc;
	PoolAlloc<sizeof(ParticleMeshRenderData), 32, 4, true> mMeshAlloc;
	PoolAlloc<sizeof(ParticleGPUSimulationData), 32, 4, true> mGPUAlloc;
	Mutex mMutex;
};

struct ParticleScene::Members
{
	// TODO - Perhaps sharing one pool is better
	ParticleSimulationDataPool SimDataPool[RenderThread::kSyncBufferCount];
};

ParticleScene::ParticleScene()
	: m(B3DNew<Members>())
{}

ParticleScene::~ParticleScene()
{
	B3DDelete(m);
}

void ParticleScene::Play(ecs::ParticleSimulation& simulation, const ParticleSystemSettings& settings)
{
	if(simulation.State == ecs::ParticleSimulationState::Playing)
		return;

	if(simulation.State == ecs::ParticleSimulationState::Uninitialized)
	{
		u32 particleCapacity = std::min(settings.MaxParticles, kInitialParticleCapacity);
		simulation.Particles = B3DMakeShared<ParticleSet>(particleCapacity);
	}

	simulation.State = ecs::ParticleSimulationState::Playing;
	simulation.Time = 0.0f;
	simulation.Rng.SetSeed(simulation.Seed);
}

void ParticleScene::Pause(ecs::ParticleSimulation& simulation)
{
	if(simulation.State == ecs::ParticleSimulationState::Playing)
		simulation.State = ecs::ParticleSimulationState::Paused;
}

void ParticleScene::Stop(ecs::ParticleSimulation& simulation)
{
	if(simulation.State != ecs::ParticleSimulationState::Playing &&
	   simulation.State != ecs::ParticleSimulationState::Paused)
		return;

	simulation.State = ecs::ParticleSimulationState::Stopped;
	if(simulation.Particles)
		simulation.Particles->Clear();
}

AABox ParticleScene::CalculateBounds(const ecs::ParticleSimulation& simulation) const
{
	// TODO - If evolvers are deterministic (as well as their properties), calculate the maximal bounds in an
	// analytical way

	if(!simulation.Particles)
		return AABox::kEmpty;

	const u32 particleCount = simulation.Particles->GetParticleCount();
	if(particleCount == 0)
		return AABox::kEmpty;

	const ParticleSetData& particles = simulation.Particles->GetParticles();
	AABox bounds(Vector3(-(float)kMaximumSceneExtent), Vector3((float)kMaximumSceneExtent));
	for(u32 particleIndex = 0; particleIndex < particleCount; particleIndex++)
		bounds.Merge(particles.Position[particleIndex]);

	return bounds;
}

void ParticleScene::PreSimulate(const ParticleSystemState& state, u32 startIndex, u32 count, bool spacing, float spacingOffset)
{
	ParticleSetData& particles = state.Particles->GetParticles();
	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
	const u32 endIndex = startIndex + count;

	// Decrement lifetime
	for(u32 particleIndex = startIndex; particleIndex < endIndex; particleIndex++)
	{
		float timeStep = state.TimeStep;
		if(spacing)
		{
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
			state.Particles->FreeParticle(particleIndex);
			particleCount--;
		}
		else
			particleSubIndex++;
	}

	// Remember old positions
	for(u32 particleIndex = startIndex; particleIndex < endIndex; particleIndex++)
		particles.PrevPosition[particleIndex] = particles.Position[particleIndex];

	// Evolve pre-simulation
	for(auto& evolver : *state.Evolvers)
	{
		if(!evolver)
			continue;

		const ParticleEvolverProperties& props = evolver->GetProperties();
		if(props.Priority < 0)
			break;

		evolver->Evolve(*state.Rng, state, *state.Particles, startIndex, count, spacing, spacingOffset);
	}
}

void ParticleScene::Simulate(const ParticleSystemState& state, u32 startIndex, u32 count, bool spacing, float spacingOffset)
{
	ParticleSetData& particles = state.Particles->GetParticles();
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

void ParticleScene::PostSimulate(const ParticleSystemState& state, u32 startIndex, u32 count, bool spacing, float spacingOffset)
{
	// Evolve post-simulation
	for(auto& evolver : *state.Evolvers)
	{
		if(!evolver)
			continue;

		const ParticleEvolverProperties& props = evolver->GetProperties();
		if(props.Priority >= 0)
			continue;

		evolver->Evolve(*state.Rng, state, *state.Particles, startIndex, count, spacing, spacingOffset);
	}
}

void ParticleScene::AdvanceSimulation(ecs::ParticleSimulation& simulation, const ecs::ParticleSystem& config, const ecs::WorldTransform& transform, float timeDelta, const EvaluatedAnimationData* animData)
{
	if(simulation.State != ecs::ParticleSimulationState::Playing)
		return;

	if(!simulation.Particles)
	{
		u32 particleCapacity = std::min(config.Settings.MaxParticles, kInitialParticleCapacity);
		simulation.Particles = B3DMakeShared<ParticleSet>(particleCapacity);
	}

	const ParticleSystemSettings& settings = config.Settings;

	float timeStep;
	const float newTime = ParticleSystem::AdvanceTime(simulation.Time, timeDelta, settings.Duration, settings.IsLooping, timeStep);

	if(timeStep < 0.00001f)
		return;

	auto owner = mOwner.lock();

	// Generate per-frame state
	ParticleSystemState state;
	state.TimeStart = simulation.Time;
	state.TimeEnd = newTime;
	state.NrmTimeStart = state.TimeStart / settings.Duration;
	state.NrmTimeEnd = state.TimeEnd / settings.Duration;
	state.Length = settings.Duration;
	state.TimeStep = timeStep;
	state.MaxParticles = settings.MaxParticles;
	state.WorldSpace = settings.SimulationSpace == ParticleSimulationSpace::World;
	state.GpuSimulated = settings.GpuSimulation;
	state.LocalToWorld = transform.GetMatrix();
	state.WorldToLocal = state.LocalToWorld.InverseAffine();
	state.Material = settings.Material;
	state.ParticleScene = this;
	state.Scene = owner.get();
	state.AnimData = animData;
	state.Particles = simulation.Particles.get();
	state.Evolvers = &config.Evolvers;
	state.Rng = &simulation.Rng;

	// For GPU simulation we only care about newly spawned particles, so clear old ones
	if(settings.GpuSimulation)
		simulation.Particles->Clear();

	// Spawn new particles
	for(auto& emitter : config.Emitters)
	{
		if(emitter)
			emitter->Spawn(simulation.Rng, state, *simulation.Particles);
	}

	// Simulate if running on CPU, otherwise just pass the spawned particles off to the render thread
	if(!settings.GpuSimulation)
	{
		const u32 particleCount = simulation.Particles->GetParticleCount();

		PreSimulate(state, 0, particleCount, false, 0.0f);
		Simulate(state, 0, particleCount, false, 0.0f);
		PostSimulate(state, 0, particleCount, false, 0.0f);
	}

	simulation.Time = newTime;
}

EvaluatedParticleData* ParticleScene::Update(const EvaluatedAnimationData& animData)
{
	// Note: Allow the worker threads to work alongside the main thread? Would require extra synchronization but
	// potentially no benefit?

	// Advance the buffers (last write buffer becomes read buffer)
	if(mSwapBuffers)
	{
		mReadBufferIdx = (mReadBufferIdx + 1) % RenderThread::kSyncBufferCount;
		mWriteBufferIdx = (mWriteBufferIdx + 1) % RenderThread::kSyncBufferCount;

		mSwapBuffers = false;
	}

	if(mPaused)
		return &mSimulationData[mReadBufferIdx];

	// TODO - Perform culling (but only on deterministic particle systems). In which case cache the bounds so we don't
	// need to recalculate them below

	// Prepare the write buffer
	EvaluatedParticleData& simulationData = mSimulationData[mWriteBufferIdx];
	simulationData.CpuData.clear();
	simulationData.GpuData.clear();

	// Queue evaluation tasks
	float timeDelta = GetTime().GetFrameDelta();

	ParticleSimulationDataPool& simDataPool = m->SimDataPool[mWriteBufferIdx];
	simDataPool.Clear();

	auto owner = mOwner.lock();
	if(!B3D_ENSURE(owner != nullptr))
		return &mSimulationData[mReadBufferIdx];

	auto& registry = owner->GetECSRegistry();

	// Collect systems to simulate from the registry view
	struct SystemToSimulate
	{
		SystemToSimulate(ecs::ParticleSimulation* simulation, const ecs::ParticleSystem* config, const ecs::WorldTransform* transform)
			: Simulation(simulation) , Config(config) , Transform(transform) {}

		ecs::ParticleSimulation* Simulation;
		const ecs::ParticleSystem* Config;
		const ecs::WorldTransform* Transform;
	};

	Vector<SystemToSimulate> systems;
	auto view = registry.CreateView<ecs::ParticleSystem, ecs::ParticleSimulation, ecs::WorldTransform>();
	for(const auto& entity : view)
	{
		ecs::ParticleSimulation& simulation = registry.GetComponents<ecs::ParticleSimulation>(entity);
		const ecs::ParticleSystem& config = registry.GetComponents<ecs::ParticleSystem>(entity);
		const ecs::WorldTransform& transform = registry.GetComponents<ecs::WorldTransform>(entity);

		systems.emplace_back(&simulation, &config, &transform);
	}

	WaitGroup waitGroup((u32)systems.size());
	for(auto& system : systems)
	{
		const auto evaluateWorker = [this, &waitGroup, timeDelta, &system, &animData, &simDataPool, &simulationData]()
		{
			// Advance the simulation
			AdvanceSimulation(*system.Simulation, *system.Config, *system.Transform, timeDelta, &animData);

			ParticleRenderData* simulationDataCPU = nullptr;
			ParticleGPUSimulationData* simulationDataGPU = nullptr;
			if(system.Simulation->Particles)
			{
				// Generate simulation data to transfer to the render thread
				const u32 numParticles = system.Simulation->Particles->GetParticleCount();
				const ParticleSystemSettings& settings = system.Config->Settings;

				if(settings.GpuSimulation)
					simulationDataGPU = simDataPool.AllocGpu(*system.Simulation->Particles);
				else
				{
					if(settings.RenderMode == ParticleRenderMode::Billboard)
						simulationDataCPU = simDataPool.AllocCpuBillboard(*system.Simulation->Particles);
					else
						simulationDataCPU = simDataPool.AllocCpuMesh(*system.Simulation->Particles);

					simulationDataCPU->NumParticles = numParticles;

					if(settings.UseAutomaticBounds)
						simulationDataCPU->Bounds = CalculateBounds(*system.Simulation);
					else
						simulationDataCPU->Bounds = settings.CustomBounds;

					// If using a camera-independant sorting mode, sort the particles right away
					switch(settings.SortMode)
					{
					default:
					case ParticleSortMode::None: // No sort, just point the indices back to themselves
						for(u32 i = 0; i < numParticles; i++)
							simulationDataCPU->Indices[i] = i;
						break;
					case ParticleSortMode::OldToYoung:
					case ParticleSortMode::YoungToOld:
						SortParticles(*system.Simulation->Particles, settings.SortMode, Vector3::kZero, simulationDataCPU->Indices.data());
						break;
					case ParticleSortMode::Distance: break;
					}
				}
			}

			{
				Lock lock(mMutex);

				if(simulationDataCPU)
					simulationData.CpuData[system.Config->Id] = simulationDataCPU;
				else if(simulationDataGPU)
					simulationData.GpuData[system.Config->Id] = simulationDataGPU;
			}

			waitGroup.NotifyDone();
		};

		GetApplication().GetTaskScheduler().Post(SchedulerTask(evaluateWorker, "ParticleWorker"));
	}

	waitGroup.Wait();
	mSwapBuffers = true;

	return &mSimulationData[mWriteBufferIdx];
}

void ParticleScene::SortParticles(const ParticleSet& set, ParticleSortMode sortMode, const Vector3& viewPoint, u32* indices)
{
	B3D_ASSERT(sortMode != ParticleSortMode::None);

	struct ParticleSortData
	{
		ParticleSortData(float key, u32 idx)
			: Key(key), Idx(idx)
		{}

		float Key;
		u32 Idx;
	};

	const u32 count = set.GetParticleCount();
	const ParticleSetData& particles = set.GetParticles();

	B3DMarkAllocatorFrame();
	{
		FrameVector<ParticleSortData> sortData;
		sortData.reserve(count);

		switch(sortMode)
		{
		default:
		case ParticleSortMode::Distance:
			for(u32 i = 0; i < count; i++)
			{
				float distance = viewPoint.SquaredDistance(particles.Position[i]);
				sortData.emplace_back(distance, i);
			}
			break;
		case ParticleSortMode::OldToYoung:
			for(u32 i = 0; i < count; i++)
			{
				float lifetime = particles.Lifetime[i];
				sortData.emplace_back(lifetime, i);
			}
			break;
		case ParticleSortMode::YoungToOld:
			for(u32 i = 0; i < count; i++)
			{
				float lifetime = particles.InitialLifetime[i] - particles.Lifetime[i];
				sortData.emplace_back(lifetime, i);
			}
			break;
		}

		std::sort(sortData.begin(), sortData.end(), [](const ParticleSortData& lhs, const ParticleSortData& rhs)
				  { return rhs.Key < lhs.Key; });

		for(u32 i = 0; i < count; i++)
			indices[i] = sortData[i].Idx;
	}
	B3DClearAllocatorFrame();
}
