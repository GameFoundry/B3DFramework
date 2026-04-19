//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DColor.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector2.h"
#include "Utility/B3DBitwise.h"
#include "Allocators/B3DGroupAlloc.h"

namespace b3d
{
	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/** Handles buffers containing particle data and their allocation/deallocation. */
	struct ParticleSetData
	{
		/** Creates a new set and allocates enough space for @p capacity particles. */
		ParticleSetData(u32 capacity)
			: Capacity(capacity)
		{
			Allocate();
		}

		/**
		 * Creates a new set, allocates enough space for @p capacity particles and initializes the particles by copying
		 * them from the @p other set.
		 */
		ParticleSetData(u32 capacity, const ParticleSetData& other)
			: Capacity(capacity)
		{
			Allocate();
			Copy(other);
		}

		/** Moves data from @p other to this set. */
		ParticleSetData(ParticleSetData&& other) noexcept
		{
			Move(other);
		}

		/** Moves data from @p other to this set. */
		ParticleSetData& operator=(ParticleSetData&& other) noexcept
		{
			if(this != &other)
			{
				Free();
				Move(other);
			}

			return *this;
		}

		~ParticleSetData()
		{
			Free();
		}

		u32 Capacity = 0;

		TArrayView<Vector3> PrevPosition;
		TArrayView<Vector3> Position;
		TArrayView<Vector3> Velocity;
		TArrayView<Vector3> Size;
		TArrayView<Vector3> Rotation;
		TArrayView<float> InitialLifetime;
		TArrayView<float> Lifetime;
		TArrayView<RGBA> Color;
		TArrayView<u32> Seed;
		TArrayView<float> Frame;
		TArrayView<u32> Indices;

	private:
		/**
		 * Allocates a new set of buffers with enough space to store number of particles equal to the current capacity. *
		 * Called must ensure any previously allocated buffer is freed by calling free().
		 */
		void Allocate()
		{
			mAllocator.Reserve<Vector3>(Capacity).Reserve<Vector3>(Capacity).Reserve<Vector3>(Capacity).Reserve<Vector3>(Capacity).Reserve<Vector3>(Capacity).Reserve<float>(Capacity).Reserve<float>(Capacity).Reserve<RGBA>(Capacity).Reserve<u32>(Capacity).Reserve<float>(Capacity).Reserve<u32>(Capacity).Initialize();

			PrevPosition = mAllocator.Allocate<Vector3>(Capacity);
			Position = mAllocator.Allocate<Vector3>(Capacity);
			Velocity = mAllocator.Allocate<Vector3>(Capacity);
			Size = mAllocator.Allocate<Vector3>(Capacity);
			Rotation = mAllocator.Allocate<Vector3>(Capacity);
			Lifetime = mAllocator.Allocate<float>(Capacity);
			InitialLifetime = mAllocator.Allocate<float>(Capacity);
			Color = mAllocator.Allocate<RGBA>(Capacity);
			Seed = mAllocator.Allocate<u32>(Capacity);
			Frame = mAllocator.Allocate<float>(Capacity);
			Indices = mAllocator.Allocate<u32>(Capacity);
		}

		/** Frees the internal buffers. */
		void Free()
		{
			mAllocator.Clear();
		}

		/** Transfers ownership of @p other internal buffers to this object. */
		void Move(ParticleSetData& other)
		{
			PrevPosition = std::exchange(other.PrevPosition, TArrayView<Vector3>());
			Position = std::exchange(other.Position, TArrayView<Vector3>());
			Velocity = std::exchange(other.Velocity, TArrayView<Vector3>());
			Size = std::exchange(other.Size, TArrayView<Vector3>());
			Rotation = std::exchange(other.Rotation, TArrayView<Vector3>());
			Lifetime = std::exchange(other.Lifetime, TArrayView<float>());
			InitialLifetime = std::exchange(other.InitialLifetime, TArrayView<float>());
			Color = std::exchange(other.Color, TArrayView<RGBA>());
			Seed = std::exchange(other.Seed, TArrayView<u32>());
			Frame = std::exchange(other.Frame, TArrayView<float>());
			Indices = std::exchange(other.Indices, TArrayView<u32>());
			Capacity = std::exchange(other.Capacity, 0);

			mAllocator = std::move(other.mAllocator);
		}

		/** Copies data from @p other buffers to this object. */
		void Copy(const ParticleSetData& other)
		{
			B3D_ASSERT(Capacity >= other.Capacity);

			B3DCopy(PrevPosition.Data(), other.PrevPosition.Data(), other.Capacity);
			B3DCopy(Position.Data(), other.Position.Data(), other.Capacity);
			B3DCopy(Velocity.Data(), other.Velocity.Data(), other.Capacity);
			B3DCopy(Size.Data(), other.Size.Data(), other.Capacity);
			B3DCopy(Rotation.Data(), other.Rotation.Data(), other.Capacity);
			B3DCopy(Lifetime.Data(), other.Lifetime.Data(), other.Capacity);
			B3DCopy(InitialLifetime.Data(), other.InitialLifetime.Data(), other.Capacity);
			B3DCopy(Color.Data(), other.Color.Data(), other.Capacity);
			B3DCopy(Seed.Data(), other.Seed.Data(), other.Capacity);
			B3DCopy(Frame.Data(), other.Frame.Data(), other.Capacity);
			B3DCopy(Indices.Data(), other.Indices.Data(), other.Capacity);
		}

		GroupAllocator mAllocator;
	};

	/** Provides a simple and fast way to allocate and deallocate particles. */
	class ParticleSet : public INonCopyable
	{
		/** Determines how much to increase capacity once the cap is reached, in percent. */
		static constexpr float kCapacityScale = 1.2f; // 20%

	public:
		/**
		 * Constructs a new particle set with enough space to hold @p capacity particles. The set will automatically
		 * grow to larger capacity if the limit is reached.
		 */
		ParticleSet(u32 capacity)
			: mParticles(capacity)
		{}

		/**
		 * Allocates a number of new particles and returns the index to the particle. Note that the returned index is not
		 * persistent and can become invalid after a call to freeParticle(). Returns the index to the first allocated
		 * particle.
		 */
		u32 AllocParticles(u32 count)
		{
			const u32 particleIdx = mCount;
			mCount += count;

			if(mCount > mParticles.Capacity)
			{
				const auto newCapacity = (u32)(mCount * kCapacityScale);
				ParticleSetData newData(newCapacity, mParticles);
				mParticles = std::move(newData);
			}

			const u32 particleEnd = particleIdx + count;
			if(particleEnd > mMaxIndex)
			{
				for(; mMaxIndex < particleEnd; mMaxIndex++)
					mParticles.Indices[mMaxIndex] = mMaxIndex;
			}

			return particleIdx;
		}

		/** Deallocates a particle. Can invalidate particle indices. */
		void FreeParticle(u32 idx)
		{
			// Note: We always keep the active particles sequential. This makes it faster to iterate over all particles, but
			// increases the cost when removing particles. Considering iteration should happen many times per-particle,
			// while removal will happen only once, this should be the more performant approach, but will likely be worth
			// profiling in the future. An alternative approach is to flag dead particles without moving them.

			B3D_ASSERT(idx < mCount);

			const u32 lastIdx = mCount - 1;
			if(idx != lastIdx)
			{
				std::swap(mParticles.PrevPosition[idx], mParticles.PrevPosition[lastIdx]);
				std::swap(mParticles.Position[idx], mParticles.Position[lastIdx]);
				std::swap(mParticles.Velocity[idx], mParticles.Velocity[lastIdx]);
				std::swap(mParticles.Size[idx], mParticles.Size[lastIdx]);
				std::swap(mParticles.Rotation[idx], mParticles.Rotation[lastIdx]);
				std::swap(mParticles.Lifetime[idx], mParticles.Lifetime[lastIdx]);
				std::swap(mParticles.InitialLifetime[idx], mParticles.InitialLifetime[lastIdx]);
				std::swap(mParticles.Color[idx], mParticles.Color[lastIdx]);
				std::swap(mParticles.Seed[idx], mParticles.Seed[lastIdx]);
				std::swap(mParticles.Frame[idx], mParticles.Frame[lastIdx]);
				std::swap(mParticles.Indices[idx], mParticles.Indices[lastIdx]);
			}

			mCount--;
		}

		/** Frees all active partices past the provided particle count (0 to clear all particles). */
		void Clear(u32 numPartices = 0)
		{
			if(mCount > numPartices)
				mCount = numPartices;
		}

		/** Returns all data about the particles. Active particles are always sequential at the start of the buffer. */
		ParticleSetData& GetParticles() { return mParticles; }

		/** Returns all data about the particles. Active particles are always sequential at the start of the buffer. */
		const ParticleSetData& GetParticles() const { return mParticles; }

		/** Returns the number of particles that are currently active. */
		u32 GetParticleCount() const { return mCount; }

		/**
		 * Calculates the size of a texture required for storing the data of this particle set. The texture is assumed
		 * to be square.
		 */
		u32 DetermineTextureSize() const
		{
			const u32 count = std::max(2U, GetParticleCount());

			u32 width = Bitwise::NextPow2(count);
			u32 height = 1;

			while(width > height)
			{
				width /= 2;
				height *= 2;
			}

			// Make it square
			return height;
		}

	private:
		ParticleSetData mParticles;
		u32 mCount = 0;
		u32 mMaxIndex = 0;
	};

	/** @} */
} // namespace b3d
