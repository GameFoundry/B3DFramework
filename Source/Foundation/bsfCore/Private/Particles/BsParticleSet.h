//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Image/BsColor.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Utility/BsBitwise.h"
#include "Allocators/BsGroupAlloc.h"

namespace bs
{
	/** @addtogroup Particles-Internal 
	 *  @{
	 */

	/** Handles buffers containing particle data and their allocation/deallocation. */
	struct ParticleSetData
	{
		/** Creates a new set and allocates enough space for @p capacity particles. */
		ParticleSetData(UINT32 capacity)
			:capacity(capacity)
		{
			allocate();
		}

		/** 
		 * Creates a new set, allocates enough space for @p capacity particles and initializes the particles by copying
		 * them from the @p other set. 
		 */
		ParticleSetData(UINT32 capacity, const ParticleSetData& other)
			:capacity(capacity)
		{
			allocate();
			copy(other);
		}

		/** Moves data from @p other to this set. */
		ParticleSetData(ParticleSetData&& other) noexcept
		{
			move(other);
		}

		/** Moves data from @p other to this set. */
		ParticleSetData& operator=(ParticleSetData&& other) noexcept
		{
			if(this != &other)
			{
				free();
				move(other);
			}
			
			return *this;
		}

		~ParticleSetData()
		{
			free();
		}

		UINT32 capacity = 0;

		Vector3* prevPosition = nullptr;
		Vector3* position = nullptr;
		Vector3* velocity = nullptr;
		Vector3* size = nullptr;
		Vector3* rotation = nullptr;
		float* initialLifetime = nullptr;
		float* lifetime = nullptr;
		RGBA* color = nullptr;
		UINT32* seed = nullptr;
		float* frame = nullptr;
		UINT32* indices = nullptr;

	private:
		/** 
		 * Allocates a new set of buffers with enough space to store number of particles equal to the current capacity. *
		 * Called must ensure any previously allocated buffer is freed by calling free().
		 */
		void allocate()
		{
			alloc.
				reserve<Vector3>(capacity).
				reserve<Vector3>(capacity).
				reserve<Vector3>(capacity).
				reserve<Vector3>(capacity).
				reserve<Vector3>(capacity).
				reserve<float>(capacity).
				reserve<float>(capacity).
				reserve<RGBA>(capacity).
				reserve<UINT32>(capacity).
				reserve<float>(capacity).
				reserve<UINT32>(capacity).
				init();

			prevPosition = alloc.alloc<Vector3>(capacity);
			position = alloc.alloc<Vector3>(capacity);
			velocity = alloc.alloc<Vector3>(capacity);
			size = alloc.alloc<Vector3>(capacity);
			rotation = alloc.alloc<Vector3>(capacity);
			lifetime = alloc.alloc<float>(capacity);
			initialLifetime = alloc.alloc<float>(capacity);
			color = alloc.alloc<RGBA>(capacity);
			seed = alloc.alloc<UINT32>(capacity);
			frame = alloc.alloc<float>(capacity);
			indices = alloc.alloc<UINT32>(capacity);
		}

		/** Frees the internal buffers. */
		void free()
		{
			if(prevPosition) alloc.free(prevPosition);
			if(position) alloc.free(position);
			if(velocity) alloc.free(velocity);
			if(size) alloc.free(size);
			if(rotation) alloc.free(rotation);
			if(lifetime) alloc.free(lifetime);
			if(initialLifetime) alloc.free(initialLifetime);
			if(color) alloc.free(color);
			if(seed) alloc.free(seed);
			if(frame) alloc.free(frame);
			if(indices) alloc.free(indices);

			alloc.clear();
		}

		/** Transfers ownership of @p other internal buffers to this object. */
		void move(ParticleSetData& other)
		{
			prevPosition = std::exchange(other.prevPosition, nullptr);
			position = std::exchange(other.position, nullptr);
			velocity = std::exchange(other.velocity, nullptr);
			size = std::exchange(other.size, nullptr);
			rotation = std::exchange(other.rotation, nullptr);
			lifetime = std::exchange(other.lifetime, nullptr);
			initialLifetime = std::exchange(other.initialLifetime, nullptr);
			color = std::exchange(other.color, nullptr);
			seed = std::exchange(other.seed, nullptr);
			frame = std::exchange(other.frame, nullptr);
			indices = std::exchange(other.indices, nullptr);
			capacity = std::exchange(other.capacity, 0);

			alloc = std::move(other.alloc);
		}

		/** Copies data from @p other buffers to this object. */
		void copy(const ParticleSetData& other)
		{
			assert(capacity >= other.capacity);

			bs_copy(prevPosition, other.prevPosition, other.capacity);
			bs_copy(position, other.position, other.capacity);
			bs_copy(velocity, other.velocity, other.capacity);
			bs_copy(size, other.size, other.capacity);
			bs_copy(rotation, other.rotation, other.capacity);
			bs_copy(lifetime, other.lifetime, other.capacity);
			bs_copy(initialLifetime, other.initialLifetime, other.capacity);
			bs_copy(color, other.color, other.capacity);
			bs_copy(seed, other.seed, other.capacity);
			bs_copy(frame, other.frame, other.capacity);
			bs_copy(indices, other.indices, other.capacity);
		}

		GroupAlloc alloc;
	};

	/** Provides a simple and fast way to allocate and deallocate particles. */
	class ParticleSet : public INonCopyable
	{
		/** Determines how much to increase capacity once the cap is reached, in percent. */
		static constexpr float CAPACITY_SCALE = 1.2f; // 20%

	public:
		/** 
		 * Constructs a new particle set with enough space to hold @p capacity particles. The set will automatically 
		 * grow to larger capacity if the limit is reached. 
		 */
		ParticleSet(UINT32 capacity)
			:mParticles(capacity)
		{ }

		/** 
		 * Allocates a number of new particles and returns the index to the particle. Note that the returned index is not
		 * persistent and can become invalid after a call to freeParticle(). Returns the index to the first allocated
		 * particle.
		 */
		UINT32 allocParticles(UINT32 count)
		{
			const UINT32 particleIdx = mCount;
			mCount += count;

			if(mCount > mParticles.capacity)
			{
				const auto newCapacity = (UINT32)(mCount * CAPACITY_SCALE);
				ParticleSetData newData(newCapacity, mParticles);
				mParticles = std::move(newData);
			}

			const UINT32 particleEnd = particleIdx + count;
			if(particleEnd > mMaxIndex)
			{
				for (; mMaxIndex < particleEnd; mMaxIndex++)
					mParticles.indices[mMaxIndex] = mMaxIndex;
			}

			return particleIdx;
		}

		/** Deallocates a particle. Can invalidate particle indices. */
		void freeParticle(UINT32 idx)
		{
			// Note: We always keep the active particles sequential. This makes it faster to iterate over all particles, but
			// increases the cost when removing particles. Considering iteration should happen many times per-particle,
			// while removal will happen only once, this should be the more performant approach, but will likely be worth
			// profiling in the future. An alternative approach is to flag dead particles without moving them.

			assert(idx < mCount);

			const UINT32 lastIdx = mCount - 1;
			if(idx != lastIdx)
			{
				std::swap(mParticles.prevPosition[idx], mParticles.prevPosition[lastIdx]);
				std::swap(mParticles.position[idx], mParticles.position[lastIdx]);
				std::swap(mParticles.velocity[idx], mParticles.velocity[lastIdx]);
				std::swap(mParticles.size[idx], mParticles.size[lastIdx]);
				std::swap(mParticles.rotation[idx], mParticles.rotation[lastIdx]);
				std::swap(mParticles.lifetime[idx], mParticles.lifetime[lastIdx]);
				std::swap(mParticles.initialLifetime[idx], mParticles.initialLifetime[lastIdx]);
				std::swap(mParticles.color[idx], mParticles.color[lastIdx]);
				std::swap(mParticles.seed[idx], mParticles.seed[lastIdx]);
				std::swap(mParticles.frame[idx], mParticles.frame[lastIdx]);
				std::swap(mParticles.indices[idx], mParticles.indices[lastIdx]);
			}

			mCount--;
		}

		/** Frees all active partices past the provided particle count (0 to clear all particles). */
		void clear(UINT32 numPartices = 0)
		{
			if(mCount > numPartices)
				mCount = numPartices;
		}

		/** Returns all data about the particles. Active particles are always sequential at the start of the buffer. */
		ParticleSetData& getParticles() { return mParticles; }

		/** Returns all data about the particles. Active particles are always sequential at the start of the buffer. */
		const ParticleSetData& getParticles() const { return mParticles; }

		/** Returns the number of particles that are currently active. */
		UINT32 getParticleCount() const { return mCount; }

		/** 
		 * Calculates the size of a texture required for storing the data of this particle set. The texture is assumed
		 * to be square.
		 */
		UINT32 determineTextureSize() const
		{
			const UINT32 count = std::max(2U, getParticleCount());

			UINT32 width = Bitwise::nextPow2(count);
			UINT32 height = 1;

			while (width > height)
			{
				width /= 2;
				height *= 2;
			}

			// Make it square
			return height;
		}

	private:
		ParticleSetData mParticles;
		UINT32 mCount = 0;
		UINT32 mMaxIndex = 0;
	};

	/** @} */
}