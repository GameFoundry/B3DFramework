//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Allocators/BsGroupAlloc.h"
#include "Image/BsColor.h"
#include "Math/BsVector3.h"

namespace bs 
{
	/** @addtogroup Particles-Internal 
	 *  @{
	 */

	// TODO - Doc
	class ParticleSet : public INonCopyable
	{
		/** Determines how much to increase capacity once the cap is reached, in percent. */
		static constexpr float CAPACITY_SCALE = 1.2f; // 20%

	public:
		ParticleSet(UINT32 capacity)
			:mParticles(capacity)
		{ }

		UINT32 allocParticle()
		{
			const UINT32 particleIdx = mCount;
			mCount++;

			if(mCount > mParticles.capacity)
			{
				const auto newCapacity = (UINT32)(mParticles.capacity * CAPACITY_SCALE);
				ParticleData newData(newCapacity, mParticles);
				mParticles = std::move(newData);
			}

			return particleIdx;
		}

		void freeParticle(UINT32 idx)
		{
			// Note: We always keep the active particles sequential. This makes it faster to iterate over all particles, but
			// increases the cost when removing particles. Considering iteration should happen many times per-particle,
			// while removal will happen only once, this should be the more performant approach, but will likely be worth
			// profiling in the future. An alternative approach is to flag dead particles without moving them.

			// TODO - This approach might not even be feasible, as I might require the indices to remain static

			assert(idx < mCount);

			const UINT32 lastIdx = mCount - 1;
			if(idx != lastIdx)
			{
				std::swap(mParticles.position[idx], mParticles.position[lastIdx]);
				std::swap(mParticles.velocity[idx], mParticles.velocity[lastIdx]);
				std::swap(mParticles.size[idx], mParticles.size[lastIdx]);
				std::swap(mParticles.lifetime[idx], mParticles.lifetime[lastIdx]);
				std::swap(mParticles.color[idx], mParticles.color[lastIdx]);
			}

			mCount--;
		}

	private:
		struct ParticleData
		{
			ParticleData(UINT32 capacity)
				:capacity(capacity)
			{
				allocate();
			}

			ParticleData(UINT32 capacity, const ParticleData& other)
				:capacity(capacity)
			{
				allocate();
				copy(other);
			}

			ParticleData(ParticleData&& other) noexcept
			{
				move(other);
			}

			ParticleData& operator=(ParticleData&& other) noexcept
			{
				if(this != &other)
				{
					free();
					move(other);
				}
				
				return *this;
			}

			~ParticleData()
			{
				free();
			}

			UINT32 capacity = 0;

			Vector3* position = nullptr;
			Vector3* velocity = nullptr;
			Vector3* size = nullptr;
			float* lifetime = nullptr;
			Color* color = nullptr;

		private:
			void allocate()
			{
				alloc.
					reserve<Vector3>(capacity).
					reserve<Vector3>(capacity).
					reserve<Vector3>(capacity).
					reserve<float>(capacity).
					reserve<Color>(capacity); // TODO - Can I get away with storing 32-bit color here?

				position = alloc.alloc<Vector3>(capacity);
				velocity = alloc.alloc<Vector3>(capacity);
				size = alloc.alloc<Vector3>(capacity);
				lifetime = alloc.alloc<float>(capacity);
				color = alloc.alloc<Color>(capacity);
			}

			void free()
			{
				if(position) alloc.free(position);
				if(velocity) alloc.free(velocity);
				if(size) alloc.free(size);
				if(lifetime) alloc.free(lifetime);
				if(color) alloc.free(color);
			}

			void move(ParticleData& other)
			{
				position = other.position; other.position = nullptr;
				velocity = other.velocity; other.velocity = nullptr;
				size = other.size; other.size = nullptr;
				lifetime = other.lifetime; other.lifetime = nullptr;
				color = other.color; other.color = nullptr;
				capacity = other.capacity; other.capacity = 0;
				alloc = std::move(other.alloc);
			}

			void copy(const ParticleData& other)
			{
				assert(capacity >= other.capacity);

				memcpy(position, other.position, other.capacity * sizeof(Vector3));
				memcpy(velocity, other.velocity, other.capacity * sizeof(Vector3));
				memcpy(size, other.size, other.capacity * sizeof(Vector3));
				memcpy(lifetime, other.lifetime, other.capacity * sizeof(float));
				memcpy(color, other.color, other.capacity * sizeof(Color));
			}

			GroupAlloc alloc;
		};

		template<class T>
		using ParticleVector = Vector<T, StdFrameAlloc<T>>;

		ParticleData mParticles;
		UINT32 mCount = 0;
	};

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	// TODO - Doc
	class BS_CORE_EXPORT ParticleSystem : public INonCopyable
	{
	};

	/** @} */
}
