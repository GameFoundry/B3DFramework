//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleSystem.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Mesh/BsMeshUtility.h"

namespace bs
{
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

		Vector3* position = nullptr;
		Vector3* velocity = nullptr;
		Vector3* size = nullptr;
		float* lifetime = nullptr;
		RGBA* color = nullptr;

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
				reserve<float>(capacity).
				reserve<RGBA>(capacity);

			position = alloc.alloc<Vector3>(capacity);
			velocity = alloc.alloc<Vector3>(capacity);
			size = alloc.alloc<Vector3>(capacity);
			lifetime = alloc.alloc<float>(capacity);
			color = alloc.alloc<RGBA>(capacity);
		}

		/** Frees the internal buffers. */
		void free()
		{
			if(position) alloc.free(position);
			if(velocity) alloc.free(velocity);
			if(size) alloc.free(size);
			if(lifetime) alloc.free(lifetime);
			if(color) alloc.free(color);
		}

		/** Transfers ownership of @p other internal buffers to this object. */
		void move(ParticleSetData& other)
		{
			position = other.position; other.position = nullptr;
			velocity = other.velocity; other.velocity = nullptr;
			size = other.size; other.size = nullptr;
			lifetime = other.lifetime; other.lifetime = nullptr;
			color = other.color; other.color = nullptr;
			capacity = other.capacity; other.capacity = 0;
			alloc = std::move(other.alloc);
		}

		/** Copies data from @p other buffers to this object. */
		void copy(const ParticleSetData& other)
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
				std::swap(mParticles.position[idx], mParticles.position[lastIdx]);
				std::swap(mParticles.velocity[idx], mParticles.velocity[lastIdx]);
				std::swap(mParticles.size[idx], mParticles.size[lastIdx]);
				std::swap(mParticles.lifetime[idx], mParticles.lifetime[lastIdx]);
				std::swap(mParticles.color[idx], mParticles.color[lastIdx]);
			}

			mCount--;
		}

		/** Returns all data about the particles. Active particles are always sequential at the start of the buffer. */
		ParticleSetData& getParticles() { return mParticles; }

		/** Returns the number of particles that are currently active. */
		UINT32 getParticleCount() const { return mCount; }

	private:
		ParticleSetData mParticles;
		UINT32 mCount = 0;
	};

	/** 
	 * Calculates and stores per-triangle weights that can be used for easily picking a random triangle on a mesh, ensuring
	 * larger triangles are picked more likely.
	 */
	class MeshWeightedTriangles
	{
		/** Contains the cumulative, normalized weight of the triangle and its vertex indices. */
		struct TriangleWeight
		{
			float cumulativeWeight;
			UINT32 indices[3];
		};

	public:
		MeshWeightedTriangles(const MeshData& meshData);

		/** Find a random triangle on the mesh and outputs its vertex indices. */
		void getTriangle(const Random& random, UINT32 (&indices)[3]) const;

	private:
		Vector<TriangleWeight> mWeights;
	};

	MeshWeightedTriangles::MeshWeightedTriangles(const MeshData& meshData)
	{
		const UINT32 numIndices = meshData.getNumIndices();
		assert(numIndices % 3 == 0);

		const UINT32 numTriangles = numIndices / 3;
		mWeights.resize(numTriangles);

		UINT8* vertices = meshData.getElementData(VES_POSITION);

		const SPtr<VertexDataDesc>& vertexDesc = meshData.getVertexDesc();
		const UINT32 stride = vertexDesc->getVertexStride();

		float totalArea = 0.0f;
		if(meshData.getIndexType() == IT_32BIT)
		{
			UINT32* indices = meshData.getIndices32();

			for(UINT32 i = 0; i < numTriangles; i++)
			{
				TriangleWeight& weight = mWeights[i];

				weight.indices[0] = indices[i * 3 + 0];
				weight.indices[1] = indices[i * 3 + 1];
				weight.indices[2] = indices[i * 3 + 2];
			}
		}
		else
		{
			UINT16* indices = meshData.getIndices16();

			for(UINT32 i = 0; i < numTriangles; i++)
			{
				TriangleWeight& weight = mWeights[i];

				weight.indices[0] = indices[i * 3 + 0];
				weight.indices[1] = indices[i * 3 + 1];
				weight.indices[2] = indices[i * 3 + 2];
			}
		}

		for (UINT32 i = 0; i < numTriangles; i++)
		{
			TriangleWeight& weight = mWeights[i];
			const Vector3& a = *(Vector3*)(vertices + weight.indices[0] * stride);
			const Vector3& b = *(Vector3*)(vertices + weight.indices[1] * stride);
			const Vector3& c = *(Vector3*)(vertices + weight.indices[2] * stride);

			weight.cumulativeWeight = Vector3::cross(b - a, c - a).squaredLength();
			totalArea += weight.cumulativeWeight;
		}

		const float invTotalArea = 1.0f / totalArea;
		for (UINT32 i = 0; i < numTriangles; i++)
			mWeights[i].cumulativeWeight *= invTotalArea;

		for (UINT32 i = 1; i < numTriangles; i++)
			mWeights[i].cumulativeWeight += mWeights[i - 1].cumulativeWeight;

		mWeights[numTriangles - 1].cumulativeWeight = 1.0f;
	}

	void MeshWeightedTriangles::getTriangle(const Random& random, UINT32(&indices)[3]) const
	{
		struct Comp
		{
			bool operator()(float a, const TriangleWeight& b) const
			{
				return a < b.cumulativeWeight;
			}

			bool operator()(const TriangleWeight& a, float b) const
			{
				return a.cumulativeWeight < b;
			}
		};

		const float val = random.getUNorm();

		const auto findIter = std::lower_bound(mWeights.begin(), mWeights.end(), val, Comp());
		if(findIter != mWeights.end())
			memcpy(indices, findIter->indices, sizeof(indices));
		else
			bs_zero_out(indices);
	}

	template<bool VOLUME, bool ARC>
	class TParticleEmitterConeShape
	{ };

	template<>
	class TParticleEmitterConeShape<false, false> : public ParticleEmitterConeShape
	{
	public:
		TParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc)
			:ParticleEmitterConeShape(desc)
		{ }

		/** @copydoc ParticleEmitterShape::spawn */
		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const Vector2 pos2D = random.getPointInCircleShell(mInfo.thickness);

			const float angleSin = Math::sin(mInfo.angle);
			normal = Vector3(pos2D.x * angleSin, pos2D.y * angleSin, Math::cos(mInfo.angle));
			normal.normalize();

			position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
		}
	};

	template<>
	class TParticleEmitterConeShape<true, false> : public ParticleEmitterConeShape
	{
	public:
		TParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc)
			:ParticleEmitterConeShape(desc)
		{ }

		/** @copydoc ParticleEmitterShape::spawn */
		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const Vector2 pos2D = random.getPointInCircleShell(mInfo.thickness);

			const float angleSin = Math::sin(mInfo.angle);
			normal = Vector3(pos2D.x * angleSin, pos2D.y * angleSin, Math::cos(mInfo.angle));
			normal.normalize();

			position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
			position += normal * mInfo.length * random.getUNorm();
		}
	};

	template<>
	class TParticleEmitterConeShape<false, true> : public ParticleEmitterConeShape
	{
	public:
		TParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc)
			:ParticleEmitterConeShape(desc)
		{ }

		/** @copydoc ParticleEmitterShape::spawn */
		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const Vector2 pos2D = random.getPointInArcShell(mInfo.arc, mInfo.thickness);

			const float angleSin = Math::sin(mInfo.angle);
			normal = Vector3(pos2D.x * angleSin, pos2D.y * angleSin, Math::cos(mInfo.angle));
			normal.normalize();

			position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
		}
	};

	template<>
	class TParticleEmitterConeShape<true, true> : public ParticleEmitterConeShape
	{
	public:
		TParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc)
			:ParticleEmitterConeShape(desc)
		{ }

		/** @copydoc ParticleEmitterShape::spawn */
		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const Vector2 pos2D = random.getPointInArcShell(mInfo.arc, mInfo.thickness);

			const float angleSin = Math::sin(mInfo.angle);
			normal = Vector3(pos2D.x * angleSin, pos2D.y * angleSin, Math::cos(mInfo.angle));
			normal.normalize();

			position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
			position += normal * mInfo.length * random.getUNorm();
		}
	};

	ParticleEmitterConeShape::ParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UPtr<ParticleEmitterConeShape> ParticleEmitterConeShape::create(const PARTICLE_CONE_SHAPE_DESC& desc)
	{
		ParticleEmitterConeShape* output;
		if(desc.type == ParticleEmitterConeType::Base)
		{
			if(Math::approxEquals(desc.arc.valueDegrees(), 360.0f))
				output = bs_new<TParticleEmitterConeShape<false, false>>(desc);
			else
				output = bs_new<TParticleEmitterConeShape<false, true>>(desc);
		}
		else // Volume
		{
			if(Math::approxEquals(desc.arc.valueDegrees(), 360.0f))
				output = bs_new<TParticleEmitterConeShape<true, false>>(desc);
			else
				output = bs_new<TParticleEmitterConeShape<true, true>>(desc);
		}

		return bs_unique_ptr(output);
	}

	ParticleEmitterSphereShape::ParticleEmitterSphereShape(const PARTICLE_SPHERE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	void ParticleEmitterSphereShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleEmitterState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);
	}

	void ParticleEmitterSphereShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = random.getPointInSphereShell(mInfo.thickness);
		normal = Vector3::normalize(position);

		position *= mInfo.radius;
	}

	UPtr<ParticleEmitterSphereShape> ParticleEmitterSphereShape::create(const PARTICLE_SPHERE_SHAPE_DESC& desc)
	{
		return bs_unique_ptr_new<ParticleEmitterSphereShape>(desc);
	}

	ParticleEmitterHemisphereShape::ParticleEmitterHemisphereShape(const PARTICLE_HEMISPHERE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	void ParticleEmitterHemisphereShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleEmitterState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);
	}

	void ParticleEmitterHemisphereShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = random.getPointInSphereShell(mInfo.thickness);
		if (position.z < 0.0f)
			position.z *= -1.0f;

		normal = Vector3::normalize(position);
		position *= mInfo.radius;
	}

	UPtr<ParticleEmitterHemisphereShape> ParticleEmitterHemisphereShape::create(
		const PARTICLE_HEMISPHERE_SHAPE_DESC& desc)
	{
		return bs_unique_ptr_new<ParticleEmitterHemisphereShape>(desc);
	}

	template<int TYPE>
	class TParticleEmitterBoxShape
	{ };

	template<>
	class TParticleEmitterBoxShape<(int)ParticleEmitterBoxType::Volume> : public ParticleEmitterBoxShape
	{
	public:
		TParticleEmitterBoxShape(const PARTICLE_BOX_SHAPE_DESC& desc)
			:ParticleEmitterBoxShape(desc)
		{ }

		/** @copydoc ParticleEmitterShape::spawn */
		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			position.x = mInfo.extents.x * random.getSNorm();
			position.y = mInfo.extents.y * random.getSNorm();
			position.z = mInfo.extents.z * random.getSNorm();
			normal = Vector3::UNIT_Z;
		}
	};

	template<>
	class TParticleEmitterBoxShape<(int)ParticleEmitterBoxType::Surface> : public ParticleEmitterBoxShape
	{
	public:
		TParticleEmitterBoxShape(const PARTICLE_BOX_SHAPE_DESC& desc)
			:ParticleEmitterBoxShape(desc)
		{
			float totalSurfaceArea = 0.0f;
			for(UINT32 i = 0; i < 3; i++)
			{
				mSurfaceArea[i] = Math::sqr(desc.extents[i]);
				totalSurfaceArea += mSurfaceArea[i];
			}

			if(totalSurfaceArea > 0.0f)
			{
				const float invTotalSurfaceArea = 1.0f / totalSurfaceArea;
				for(UINT32 i = 0; i < 3; i++)
					mSurfaceArea[i] *= invTotalSurfaceArea;

				mSurfaceArea[1] += mSurfaceArea[0];
				mSurfaceArea[2] = 1.0f;
			}
		}

		/** @copydoc ParticleEmitterShape::spawn */
		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const float u = random.getSNorm();
			const float v = random.getSNorm();

			// Determine an axis (based on their size, larger being more likely)
			const float axisRnd = random.getUNorm();
			UINT32 axis = 0;
			for (; axis < 3; axis++)
			{
				if(axisRnd <= mSurfaceArea[axis])
					break;
			}

			switch(axis)
			{
			case 0:
				position.x = mInfo.extents.x * u;
				position.y = mInfo.extents.y * v;
				position.z = random.getUNorm() > 0.5f ? mInfo.extents.z : -mInfo.extents.z;
				break;
			case 1:
				position.x = mInfo.extents.x * u;
				position.y = random.getUNorm() > 0.5f ? mInfo.extents.y : -mInfo.extents.y;
				position.z = mInfo.extents.z * v;
				break;
			case 2:
				position.x = random.getUNorm() > 0.5f ? mInfo.extents.x : -mInfo.extents.x;
				position.y = mInfo.extents.y * v;
				position.z = mInfo.extents.z * u;
				break;
			default:
				break;
			}

			normal = Vector3::UNIT_Z;
		}

	private:
		float mSurfaceArea[3];
	};

	template<>
	class TParticleEmitterBoxShape<(int)ParticleEmitterBoxType::Edge> : public ParticleEmitterBoxShape
	{
	public:
		TParticleEmitterBoxShape(const PARTICLE_BOX_SHAPE_DESC& desc)
			:ParticleEmitterBoxShape(desc)
		{
			float totalEdgeLength = 0.0f;
			for(UINT32 i = 0; i < 3; i++)
			{
				mEdgeLengths[i] = desc.extents[i];
				totalEdgeLength += mEdgeLengths[i];
			}

			if(totalEdgeLength > 0.0f)
			{
				const float invTotalEdgeLength = 1.0f / totalEdgeLength;
				for(UINT32 i = 0; i < 3; i++)
					mEdgeLengths[i] *= invTotalEdgeLength;

				mEdgeLengths[1] += mEdgeLengths[0];
				mEdgeLengths[2] = 1.0f;
			}
		}

		/** @copydoc ParticleEmitterShape::spawn */
		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		/** Spawns a single particle, generating its position and normal. */
		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const float u = random.getSNorm();

			// Determine an axis (based on their length, longer being more likely)
			const float axisRnd = random.getUNorm();
			UINT32 axis = 0;
			for (; axis < 3; axis++)
			{
				if(axisRnd <= mEdgeLengths[axis])
					break;
			}

			switch(axis)
			{
			case 0:
				position.x = mInfo.extents.x * u;
				position.y = random.getUNorm() > 0.5f ? mInfo.extents.y : -mInfo.extents.y;
				position.z = random.getUNorm() > 0.5f ? mInfo.extents.z : -mInfo.extents.z;
				break;
			case 1:
				position.x = random.getUNorm() > 0.5f ? mInfo.extents.x : -mInfo.extents.x;
				position.y = mInfo.extents.y * u;
				position.z = random.getUNorm() > 0.5f ? mInfo.extents.z : -mInfo.extents.z;
				break;
			case 2:
				position.x = random.getUNorm() > 0.5f ? mInfo.extents.x : -mInfo.extents.x;
				position.y = random.getUNorm() > 0.5f ? mInfo.extents.y : -mInfo.extents.y;
				position.z = mInfo.extents.z * u;
				break;
			default:
				break;
			}

			normal = Vector3::UNIT_Z;
		}

	private:
		float mEdgeLengths[3];
	};

	ParticleEmitterBoxShape::ParticleEmitterBoxShape(const PARTICLE_BOX_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UPtr<ParticleEmitterBoxShape> ParticleEmitterBoxShape::create(const PARTICLE_BOX_SHAPE_DESC& desc)
	{
		ParticleEmitterBoxShape* output;
		switch(desc.type)
		{
		default:
		case ParticleEmitterBoxType::Volume: 
			output = bs_new<TParticleEmitterBoxShape<(int)ParticleEmitterBoxType::Volume>>(desc);
			break;
		case ParticleEmitterBoxType::Surface:
			output = bs_new<TParticleEmitterBoxShape<(int)ParticleEmitterBoxType::Surface>>(desc);
			break;
		case ParticleEmitterBoxType::Edge:
			output = bs_new<TParticleEmitterBoxShape<(int)ParticleEmitterBoxType::Edge>>(desc);
			break;
		}

		return bs_unique_ptr(output);
	}

	ParticleEmitterLineShape::ParticleEmitterLineShape(const PARTICLE_LINE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	void ParticleEmitterLineShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleEmitterState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);
	}

	void ParticleEmitterLineShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = random.getSNorm() * mInfo.length * 0.5f;
		normal = Vector3::UNIT_Z;
	}

	UPtr<ParticleEmitterLineShape> ParticleEmitterLineShape::create(const PARTICLE_LINE_SHAPE_DESC& desc)
	{
		return bs_unique_ptr_new<ParticleEmitterLineShape>(desc);
	}

	template<bool ARC>
	class TParticleEmitterCircleShape
	{ };

	template<>
	class TParticleEmitterCircleShape<false> : public ParticleEmitterCircleShape
	{
	public:
		TParticleEmitterCircleShape(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
			:ParticleEmitterCircleShape(desc)
		{ }

		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const Vector2 pos2D = random.getPointInCircleShell(mInfo.thickness);

			position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
			normal = Vector3::UNIT_Z;
		}
	};

	template<>
	class TParticleEmitterCircleShape<true> : public ParticleEmitterCircleShape
	{
	public:
		TParticleEmitterCircleShape(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
			:ParticleEmitterCircleShape(desc)
		{ }

		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const Vector2 pos2D = random.getPointInArcShell(mInfo.arc, mInfo.thickness);

			position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
			normal = Vector3::UNIT_Z;
		}
	};

	ParticleEmitterCircleShape::ParticleEmitterCircleShape(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UPtr<ParticleEmitterCircleShape> ParticleEmitterCircleShape::create(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
	{
		ParticleEmitterCircleShape* output;
		if (Math::approxEquals(desc.arc.valueDegrees(), 360.0f))
			output = bs_new<TParticleEmitterCircleShape<false>>(desc);
		else
			output = bs_new<TParticleEmitterCircleShape<true>>(desc);

		return bs_unique_ptr(output);
	}

	ParticleEmitterRectShape::ParticleEmitterRectShape(const PARTICLE_RECT_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	void ParticleEmitterRectShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleEmitterState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);
	}

	void ParticleEmitterRectShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position.x = random.getSNorm() * mInfo.extents.x;
		position.y = random.getSNorm() * mInfo.extents.y;
		position.z = 0.0f;

		normal = Vector3::UNIT_Z;
	}

	UPtr<ParticleEmitterRectShape> ParticleEmitterRectShape::create(const PARTICLE_RECT_SHAPE_DESC& desc)
	{
		return bs_unique_ptr_new<ParticleEmitterRectShape>(desc);
	}

	template<int TYPE>
	class TParticleEmitterStaticMeshShape
	{ };

	template<>
	class TParticleEmitterStaticMeshShape<(int)ParticleEmitterMeshType::Vertex> : public ParticleEmitterStaticMeshShape
	{
	public:
		TParticleEmitterStaticMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc)
			:ParticleEmitterStaticMeshShape(desc)
		{ }

		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			const UINT32 vertexIdx = random.get() % mNumVertices;
			position = *(Vector3*)(mVertices + mVertexStride * vertexIdx);

			if (mNormals)
			{
				if (m32BitNormals)
					normal = MeshUtility::unpackNormal(mNormals + mVertexStride * vertexIdx);
				else
					normal = *(Vector3*)(mNormals + mVertexStride * vertexIdx);
			}
			else
				normal = Vector3::UNIT_Z;
		}
	};

	template<>
	class TParticleEmitterStaticMeshShape<(int)ParticleEmitterMeshType::Edge> : public ParticleEmitterStaticMeshShape
	{
	public:
		TParticleEmitterStaticMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc)
			:ParticleEmitterStaticMeshShape(desc), mWeightedTriangles(*desc.meshData)
		{ }

		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			UINT32 indices[3];
			mWeightedTriangles.getTriangle(random, indices);

			// Pick edge
			UINT32 edgeIndices[2];

			// Note: Longer edges should be given higher chance, but we're assuming they are all equal length for performance
			const int32_t edge = random.getRange(0, 2);
			switch (edge)
			{
			default:
			case 0:
				edgeIndices[0] = indices[0];
				edgeIndices[1] = indices[1];
				break;
			case 1:
				edgeIndices[0] = indices[1];
				edgeIndices[1] = indices[2];
				break;
			case 2:
				edgeIndices[0] = indices[2];
				edgeIndices[1] = indices[0];
				break;
			}

			const Vector3& posA = *(Vector3*)(mVertices + mVertexStride * edgeIndices[0]);
			const Vector3& posB = *(Vector3*)(mVertices + mVertexStride * edgeIndices[1]);

			const float rnd = random.getUNorm();
			position = Math::lerp(rnd, posA, posB);

			if (mNormals)
			{
				if (m32BitNormals)
				{
					const Vector3 nrmA = MeshUtility::unpackNormal(mNormals + mVertexStride * edgeIndices[0]);
					const Vector3 nrmB = MeshUtility::unpackNormal(mNormals + mVertexStride * edgeIndices[1]);

					normal = Math::lerp(rnd, nrmA, nrmB);
				}
				else
				{
					const Vector3& nrmA = *(Vector3*)(mNormals + mVertexStride * edgeIndices[0]);
					const Vector3& nrmB = *(Vector3*)(mNormals + mVertexStride * edgeIndices[1]);

					normal = Math::lerp(rnd, nrmA, nrmB);
				}
			}
			else
				normal = Vector3::UNIT_Z;
		}

	private:
		MeshWeightedTriangles mWeightedTriangles;
	};

	template<>
	class TParticleEmitterStaticMeshShape<(int)ParticleEmitterMeshType::Triangle> : public ParticleEmitterStaticMeshShape
	{
	public:
		TParticleEmitterStaticMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc)
			:ParticleEmitterStaticMeshShape(desc), mWeightedTriangles(*desc.meshData)
		{ }

		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, particleData.position[i], particleData.velocity[i]);
		}

		void spawn(const Random& random, Vector3& position, Vector3& normal) const
		{
			UINT32 indices[3];
			mWeightedTriangles.getTriangle(random, indices);

			position = Vector3::ZERO;
			normal = Vector3::ZERO;
			Vector3 barycenter = random.getBarycentric();

			for (uint32_t i = 0; i < 3; i++)
			{
				const Vector3& curPosition = *(Vector3*)(mVertices + mVertexStride * indices[i]);
				position += curPosition * barycenter[i];

				if (mNormals)
				{
					if (m32BitNormals)
					{
						const Vector3 curNormal = MeshUtility::unpackNormal(mNormals + mVertexStride * indices[i]);
						normal += curNormal * barycenter[i];
					}
					else
					{
						const Vector3& curNormal = *(Vector3*)(mNormals + mVertexStride * indices[i]);
						normal += curNormal * barycenter[i];
					}
				}
				else
					normal += Vector3::UNIT_Z * barycenter[i];
			}
		}

	private:
		MeshWeightedTriangles mWeightedTriangles;
	};

	ParticleEmitterStaticMeshShape::ParticleEmitterStaticMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc)
		:mInfo(desc)
	{
		mVertices = desc.meshData->getElementData(VES_POSITION);
		mNumVertices = desc.meshData->getNumVertices();
		
		const SPtr<VertexDataDesc>& vertexDesc = desc.meshData->getVertexDesc();
		mVertexStride = vertexDesc->getVertexStride();

		const VertexElement* normalElement = vertexDesc->getElement(VES_NORMAL);

		mNormals = nullptr;
		if(normalElement)
		{
			if(normalElement->getType() == VET_UBYTE4_NORM)
			{
				mNormals = desc.meshData->getElementData(VES_NORMAL);
				m32BitNormals = true;
			}
			else if(normalElement->getType() == VET_FLOAT3)
			{
				mNormals = desc.meshData->getElementData(VES_NORMAL);
				m32BitNormals = false;
			}
		}
	}

	UPtr<ParticleEmitterStaticMeshShape> ParticleEmitterStaticMeshShape::create(const PARTICLE_MESH_SHAPE_DESC& desc)
	{
		// TODO - Only support TRIANGLE_LIST draw operation
		// TODO - Ensure mesh-data at least has position (with correct type)

		ParticleEmitterStaticMeshShape* output;
		switch(desc.type)
		{
		case ParticleEmitterMeshType::Vertex: 
			output = bs_new<TParticleEmitterStaticMeshShape<(int)ParticleEmitterMeshType::Vertex>>(desc);
			break;
		case ParticleEmitterMeshType::Edge:
			output = bs_new<TParticleEmitterStaticMeshShape<(int)ParticleEmitterMeshType::Edge>>(desc);
			break;
		default:
		case ParticleEmitterMeshType::Triangle:
			output = bs_new<TParticleEmitterStaticMeshShape<(int)ParticleEmitterMeshType::Triangle>>(desc);
			break;
		}
		return bs_unique_ptr(output);
	}

	template<int TYPE>
	class TParticleEmitterSkinnedMeshShape
	{ };

	template<>
	class TParticleEmitterSkinnedMeshShape<(int)ParticleEmitterMeshType::Vertex> : public ParticleEmitterSkinnedMeshShape 
	{
	public:
		TParticleEmitterSkinnedMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc)
			:ParticleEmitterSkinnedMeshShape(desc)
		{ }

		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, state, particleData.position[i], particleData.velocity[i]);
		}

		void spawn(const Random& random, const ParticleEmitterState& state, Vector3& position, 
			Vector3& normal) const
		{
			const UINT32 vertexIdx = random.get() % mNumVertices;

			Matrix4 blendMatrix = getBlendMatrix(state, vertexIdx);

			position = *(Vector3*)(mVertices + mVertexStride * vertexIdx);
			position = blendMatrix.multiplyAffine(position);

			if (mNormals)
			{
				if (m32BitNormals)
					normal = MeshUtility::unpackNormal(mNormals + mVertexStride * vertexIdx);
				else
					normal = *(Vector3*)(mNormals + mVertexStride * vertexIdx);

				normal = blendMatrix.multiplyDirection(normal);
			}
			else
				normal = Vector3::UNIT_Z;
		}
	};

	template<>
	class TParticleEmitterSkinnedMeshShape<(int)ParticleEmitterMeshType::Edge> : public ParticleEmitterSkinnedMeshShape 
	{
	public:
		TParticleEmitterSkinnedMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc)
			:ParticleEmitterSkinnedMeshShape(desc), mWeightedTriangles(*desc.meshData)
		{ }

		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, state, particleData.position[i], particleData.velocity[i]);
		}

		void spawn(const Random& random, const ParticleEmitterState& state, Vector3& position, 
			Vector3& normal) const
		{
			UINT32 indices[3];
			mWeightedTriangles.getTriangle(random, indices);

			// Pick edge
			UINT32 edgeIndices[2];

			// Note: Longer edges should be given higher chance, but we're assuming they are all equal length for performance
			const int32_t edge = random.getRange(0, 2);
			switch (edge)
			{
			default:
			case 0:
				edgeIndices[0] = indices[0];
				edgeIndices[1] = indices[1];
				break;
			case 1:
				edgeIndices[0] = indices[1];
				edgeIndices[1] = indices[2];
				break;
			case 2:
				edgeIndices[0] = indices[2];
				edgeIndices[1] = indices[0];
				break;
			}

			Vector3 posA = *(Vector3*)(mVertices + mVertexStride * edgeIndices[0]);
			Vector3 posB = *(Vector3*)(mVertices + mVertexStride * edgeIndices[1]);

			Matrix4 blendMatrixA = getBlendMatrix(state, edgeIndices[0]);
			Matrix4 blendMatrixB = getBlendMatrix(state, edgeIndices[1]);

			posA = blendMatrixA.multiplyAffine(posA);
			posB = blendMatrixB.multiplyAffine(posB);

			const float rnd = random.getUNorm();
			position = Math::lerp(rnd, posA, posB);

			Vector3 nrmA = MeshUtility::unpackNormal(mNormals + mVertexStride * edgeIndices[0]);
			Vector3 nrmB = MeshUtility::unpackNormal(mNormals + mVertexStride * edgeIndices[1]);
			if (mNormals)
			{
				if (m32BitNormals)
				{
					nrmA = MeshUtility::unpackNormal(mNormals + mVertexStride * edgeIndices[0]);
					nrmB = MeshUtility::unpackNormal(mNormals + mVertexStride * edgeIndices[1]);

				}
				else
				{
					nrmA = *(Vector3*)(mNormals + mVertexStride * edgeIndices[0]);
					nrmB = *(Vector3*)(mNormals + mVertexStride * edgeIndices[1]);
				}
			}
			else
			{
				nrmA = Vector3::UNIT_Z;
				nrmB = Vector3::UNIT_Z;
			}

			nrmA = blendMatrixA.multiplyDirection(nrmA);
			nrmB = blendMatrixB.multiplyDirection(nrmB);
			normal = Math::lerp(rnd, nrmA, nrmB);
		}

	private:
		MeshWeightedTriangles mWeightedTriangles;
	};

	template<>
	class TParticleEmitterSkinnedMeshShape<(int)ParticleEmitterMeshType::Triangle> : public ParticleEmitterSkinnedMeshShape 
	{
	public:
		TParticleEmitterSkinnedMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc)
			:ParticleEmitterSkinnedMeshShape(desc), mWeightedTriangles(*desc.meshData)
		{ }

		void spawn(const Random& random, ParticleSet& particles, UINT32 count, 
			const ParticleEmitterState& state) const override
		{
			const UINT32 index = particles.allocParticles(count);
			ParticleSetData& particleData = particles.getParticles();

			const UINT32 end = index + count;
			for(UINT32 i = index; i < end; i++)
				spawn(random, state, particleData.position[i], particleData.velocity[i]);
		}

		void spawn(const Random& random, const ParticleEmitterState& state, Vector3& position, 
			Vector3& normal) const
		{
			UINT32 indices[3];
			mWeightedTriangles.getTriangle(random, indices);

			position = Vector3::ZERO;
			normal = Vector3::ZERO;
			Vector3 barycenter = random.getBarycentric();

			for (uint32_t i = 0; i < 3; i++)
			{
				Matrix4 blendMatrix = getBlendMatrix(state, indices[i]);

				Vector3 curPosition = *(Vector3*)(mVertices + mVertexStride * indices[i]);
				curPosition = blendMatrix.multiplyAffine(curPosition);

				position += curPosition * barycenter[i];

				Vector3 curNormal;
				if (mNormals)
				{
					if (m32BitNormals)
						curNormal = MeshUtility::unpackNormal(mNormals + mVertexStride * indices[i]);
					else
						curNormal = *(Vector3*)(mNormals + mVertexStride * indices[i]);
				}
				else
					curNormal = barycenter[i];

				curNormal = blendMatrix.multiplyDirection(curNormal);
				normal += curNormal * barycenter[i];
			}
		}

	private:
		MeshWeightedTriangles mWeightedTriangles;
	};

	ParticleEmitterSkinnedMeshShape::ParticleEmitterSkinnedMeshShape(const PARTICLE_MESH_SHAPE_DESC& desc)
		:mInfo(desc)
	{
		mVertices = desc.meshData->getElementData(VES_POSITION);
		mBoneIndices = desc.meshData->getElementData(VES_BLEND_INDICES);
		mBoneWeights = desc.meshData->getElementData(VES_BLEND_WEIGHTS);
		mNumVertices = desc.meshData->getNumVertices();
		
		const SPtr<VertexDataDesc>& vertexDesc = desc.meshData->getVertexDesc();
		mVertexStride = vertexDesc->getVertexStride();

		const VertexElement* normalElement = vertexDesc->getElement(VES_NORMAL);

		mNormals = nullptr;
		if(normalElement)
		{
			if(normalElement->getType() == VET_UBYTE4_NORM)
			{
				mNormals = desc.meshData->getElementData(VES_NORMAL);
				m32BitNormals = true;
			}
			else if(normalElement->getType() == VET_FLOAT3)
			{
				mNormals = desc.meshData->getElementData(VES_NORMAL);
				m32BitNormals = false;
			}
		}
	}

	Matrix4 ParticleEmitterSkinnedMeshShape::getBlendMatrix(const ParticleEmitterState& state, UINT32 vertexIdx) const
	{
		if(state.skinnedMesh.bones)
		{
			const UINT32 boneIndices = *(UINT32*)(mBoneIndices + vertexIdx * mVertexStride);
			const Vector4& boneWeights = *(Vector4*)(mBoneWeights + vertexIdx * mVertexStride);

			return
				state.skinnedMesh.bones[boneIndices & 0xFF] * boneWeights[0] +
				state.skinnedMesh.bones[(boneIndices >> 8) & 0xFF] * boneWeights[1] +
				state.skinnedMesh.bones[(boneIndices >> 16) & 0xFF] * boneWeights[2] +
				state.skinnedMesh.bones[(boneIndices >> 24) & 0xFF] * boneWeights[3];
		}
		else
			return Matrix4::IDENTITY;
	}

	UPtr<ParticleEmitterSkinnedMeshShape> ParticleEmitterSkinnedMeshShape::create(const PARTICLE_MESH_SHAPE_DESC& desc)
	{
		// TODO - Only support TRIANGLE_LIST draw operation
		// TODO - Ensure mesh-data at least has position (with correct type)
		// TODO - Ensure mesh data has bone weights and indices (with correct types)

		ParticleEmitterSkinnedMeshShape* output;
		switch(desc.type)
		{
		case ParticleEmitterMeshType::Vertex: 
			output = bs_new<TParticleEmitterSkinnedMeshShape<(int)ParticleEmitterMeshType::Vertex>>(desc);
			break;
		case ParticleEmitterMeshType::Edge:
			output = bs_new<TParticleEmitterSkinnedMeshShape<(int)ParticleEmitterMeshType::Edge>>(desc);
			break;
		default:
		case ParticleEmitterMeshType::Triangle:
			output = bs_new<TParticleEmitterSkinnedMeshShape<(int)ParticleEmitterMeshType::Triangle>>(desc);
			break;
		}
		return bs_unique_ptr(output);
	}

}
