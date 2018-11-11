//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsParticleEmitter.h"
#include "Mesh/BsMeshData.h"
#include "Mesh/BsMeshUtility.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Math/BsRandom.h"
#include "Renderer/BsRenderable.h"
#include "Private/Particles/BsParticleSet.h"
#include "Private/RTTI/BsParticleSystemRTTI.h"
#include "Animation/BsAnimation.h"
#include "Animation/BsAnimationManager.h"
#include "Mesh/BsMesh.h"

namespace bs
{
	MeshWeightedTriangles::MeshWeightedTriangles(const MeshData& meshData)
	{
		calculate(meshData);
	}

	void MeshWeightedTriangles::calculate(const MeshData& meshData)
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

			// Note: Using squared length here would be faster, but the weights can be small and squaring them just
			// makes them smaller, causing precision issues
			weight.cumulativeWeight = Vector3::cross(b - a, c - a).length();
			totalArea += weight.cumulativeWeight;
		}

		const float invTotalArea = 1.0f / totalArea;
		for (UINT32 i = 0; i < numTriangles; i++)
			mWeights[i].cumulativeWeight *= invTotalArea;

		for (UINT32 i = 1; i < numTriangles; i++)
			mWeights[i].cumulativeWeight += mWeights[i - 1].cumulativeWeight;

		mWeights[numTriangles - 1].cumulativeWeight = 1.0f;
	}

	void MeshWeightedTriangles::getTriangle(const Random& random, std::array<UINT32, 3>& indices) const
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
			memcpy(indices.data(), findIter->indices, sizeof(indices));
		else
			bs_zero_out(indices);
	}

	template <class Pr>
	UINT32 spawnMultiple(ParticleSet& particles, UINT32 count, Pr predicate)
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			predicate(i - index, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	template <class T>
	UINT32 spawnMultipleRandom(T* spawner, const Random& random, ParticleSet& particles, UINT32 count)
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawner->_spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	template <class T>
	UINT32 spawnMultipleSpread(T* spawner, float length, float interval, ParticleSet& particles, UINT32 count)
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const float dt = length / (float)count;

		float accum = 0.0f;
		for (UINT32 i = 0; i < count; i++)
		{
			float t = accum;
			if(interval > 0)
				t = Math::roundToMultiple(accum, interval);

			const UINT32 particleIdx = index + i;
			spawner->_spawn(t, particleData.position[particleIdx], particleData.velocity[particleIdx]);

			accum += dt;
		}

		return index;
	}

	template <class T>
	UINT32 spawnMultipleLoop(T* spawner, float length, float speed, float interval, ParticleSet& particles, 
		UINT32 count, const ParticleSystemState& state)
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const float dt = state.timeStep / (float)count;

		for (UINT32 i = 0; i < count; i++)
		{
			float t = (state.timeStart + dt * i) * speed;
			t = fmod(t, length);

			if(interval > 0.0f)
				t = Math::roundToMultiple(t, interval);

			const UINT32 particleIdx = index + i;
			spawner->_spawn(t, particleData.position[particleIdx], particleData.velocity[particleIdx]);
		}

		return index;
	}

	template <class T>
	UINT32 spawnMultiplePingPong(T* spawner, float length, float speed, float interval, ParticleSet& particles, 
		UINT32 count, const ParticleSystemState& state)
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const float dt = state.timeStep / (float)count;

		for (UINT32 i = 0; i < count; i++)
		{
			float t = (state.timeStart + dt * i) * speed;

			const auto loop = (UINT32)(t / length);
			if (loop % 2 == 1)
				t = length - fmod(t, length);
			else
				t = fmod(t, length);

			if(interval > 0.0f)
				t = Math::roundToMultiple(t, interval);

			const UINT32 particleIdx = index + i;
			spawner->_spawn(t, particleData.position[particleIdx], particleData.velocity[particleIdx]);
		}

		return index;
	}

	template <class T>
	UINT32 spawnMultipleMode(T* spawner, ParticleEmissionModeType type, float length, float speed, float interval, 
		const Random& random, ParticleSet& particles, UINT32 count, const ParticleSystemState& state)
	{
		if(count > 0)
		{
			switch (type)
			{
			case ParticleEmissionModeType::Random:
				return spawnMultipleRandom(spawner, random, particles, count);
			case ParticleEmissionModeType::Loop:
				return spawnMultipleLoop(spawner, length, speed, interval, particles,
					count, state);
			case ParticleEmissionModeType::PingPong:
				return spawnMultiplePingPong(spawner, length, speed, interval, particles,
					count, state);
			case ParticleEmissionModeType::Spread:
				return spawnMultipleSpread(spawner, length, interval, particles, count);
			default:
				break;
			}
		}

		return particles.getParticleCount();
	}

	ParticleEmitterConeShape::ParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UINT32 ParticleEmitterConeShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		return spawnMultipleMode(this, mInfo.mode.type, mInfo.arc.valueRadians(), mInfo.mode.speed * Math::DEG2RAD, 
			mInfo.mode.interval * Math::DEG2RAD, random, particles, count, state);
	}

	void ParticleEmitterConeShape::_spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		Vector2 pos2D;
		if (Math::approxEquals(mInfo.arc.valueDegrees(), 360.0f))
			pos2D = random.getPointInCircleShell(mInfo.thickness);
		else
			pos2D = random.getPointInArcShell(mInfo.arc, mInfo.thickness);

		getPointInCone(pos2D, random.getUNorm() * mInfo.length, position, normal);
	}

	void ParticleEmitterConeShape::_spawn(float t, Vector3& position, Vector3& normal) const
	{
		const Vector2 pos2D(Math::cos(t), Math::sin(t));

		getPointInCone(pos2D, 0.0f, position, normal);
	}

	void ParticleEmitterConeShape::getPointInCone(const Vector2& pos2D, float distance, Vector3& position, 
		Vector3& normal) const
	{
		const float angleSin = Math::sin(mInfo.angle);
		normal = Vector3(pos2D.x * angleSin, pos2D.y * angleSin, Math::cos(mInfo.angle));
		normal.normalize();

		position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);

		if(mInfo.type == ParticleEmitterConeType::Volume)
			position += normal * distance;
	}

	SPtr<ParticleEmitterConeShape> ParticleEmitterConeShape::create(const PARTICLE_CONE_SHAPE_DESC& desc)
	{
		ParticleEmitterConeShape* output = bs_new<ParticleEmitterConeShape>(desc);
		return bs_shared_ptr(output);
	}

	void ParticleEmitterConeShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		const float sinAngle = Math::sin(mInfo.angle);
		const float cosAngle = Math::cos(mInfo.angle);

		if(mInfo.type == ParticleEmitterConeType::Base)
		{
			shape.setMin(Vector3(-mInfo.radius, -mInfo.radius, 0.0f));
			shape.setMax(Vector3(mInfo.radius, mInfo.radius, 0.0f));
		}
		else
		{
			const float topRadius = mInfo.radius + mInfo.length * sinAngle;
			const float length = mInfo.length * cosAngle;

			shape.setMin(Vector3(-topRadius, -topRadius, 0.0f));
			shape.setMax(Vector3(topRadius, topRadius, length));
		}

		velocity.setMin(Vector3(-sinAngle, -sinAngle, 0.0f));
		velocity.setMax(Vector3(sinAngle, sinAngle, 1.0f));
	}
	
	RTTITypeBase* ParticleEmitterConeShape::getRTTIStatic()
	{
		return ParticleEmitterConeShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterConeShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleEmitterSphereShape::ParticleEmitterSphereShape(const PARTICLE_SPHERE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UINT32 ParticleEmitterSphereShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		return spawnMultipleRandom(this, random, particles, count);
	}

	void ParticleEmitterSphereShape::_spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = random.getPointInSphereShell(mInfo.thickness);
		normal = Vector3::normalize(position);

		position *= mInfo.radius;
	}

	void ParticleEmitterSphereShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		shape.setMin(Vector3::ONE * -mInfo.radius);
		shape.setMax(Vector3::ONE * mInfo.radius);

		velocity.setMin(-Vector3::ONE);
		velocity.setMax(Vector3::ONE);
	}
	
	SPtr<ParticleEmitterShape> ParticleEmitterSphereShape::create(const PARTICLE_SPHERE_SHAPE_DESC& desc)
	{
		return bs_shared_ptr<ParticleEmitterShape>(bs_new<ParticleEmitterSphereShape>(desc));
	}

	RTTITypeBase* ParticleEmitterSphereShape::getRTTIStatic()
	{
		return ParticleEmitterSphereShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterSphereShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleEmitterHemisphereShape::ParticleEmitterHemisphereShape(const PARTICLE_HEMISPHERE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UINT32 ParticleEmitterHemisphereShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		return spawnMultipleRandom(this, random, particles, count);
	}

	void ParticleEmitterHemisphereShape::_spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = random.getPointInSphereShell(mInfo.thickness);
		if (position.z < 0.0f)
			position.z *= -1.0f;

		normal = Vector3::normalize(position);
		position *= mInfo.radius;
	}

	void ParticleEmitterHemisphereShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		shape.setMin(Vector3(-mInfo.radius, -mInfo.radius, 0.0f));
		shape.setMax(Vector3::ONE * mInfo.radius);

		velocity.setMin(Vector3(-1.0f, -1.0f, 0.0f));
		velocity.setMax(Vector3::ONE);
	}
	
	SPtr<ParticleEmitterShape> ParticleEmitterHemisphereShape::create(const PARTICLE_HEMISPHERE_SHAPE_DESC& desc)
	{
		return bs_shared_ptr<ParticleEmitterShape>(bs_new<ParticleEmitterHemisphereShape>(desc));
	}

	RTTITypeBase* ParticleEmitterHemisphereShape::getRTTIStatic()
	{
		return ParticleEmitterHemisphereShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterHemisphereShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleEmitterBoxShape::ParticleEmitterBoxShape(const PARTICLE_BOX_SHAPE_DESC& desc)
		:mInfo(desc)
	{
		switch(mInfo.type)
		{
		case ParticleEmitterBoxType::Surface: 
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
			break;
		case ParticleEmitterBoxType::Edge: 
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
			break;
		default:
		case ParticleEmitterBoxType::Volume: break;
		}
	}

	UINT32 ParticleEmitterBoxShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		return spawnMultipleRandom(this, random, particles, count);
	}

	void ParticleEmitterBoxShape::_spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		switch(mInfo.type)
		{
		default:
		case ParticleEmitterBoxType::Volume: 
			position.x = mInfo.extents.x * random.getSNorm();
			position.y = mInfo.extents.y * random.getSNorm();
			position.z = mInfo.extents.z * random.getSNorm();
			normal = Vector3::UNIT_Z;
			break;
		case ParticleEmitterBoxType::Surface:
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
			break;
		case ParticleEmitterBoxType::Edge:
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
			break;
		}
	}

	void ParticleEmitterBoxShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		shape.setMin(-mInfo.extents);
		shape.setMax(mInfo.extents);

		velocity.setMin(Vector3::ZERO);
		velocity.setMax(Vector3::UNIT_Z);
	}

	SPtr<ParticleEmitterShape> ParticleEmitterBoxShape::create(const PARTICLE_BOX_SHAPE_DESC& desc)
	{
		ParticleEmitterBoxShape* output = bs_new<ParticleEmitterBoxShape>(desc);
		return bs_shared_ptr<ParticleEmitterShape>(output);
	}

	RTTITypeBase* ParticleEmitterBoxShape::getRTTIStatic()
	{
		return ParticleEmitterBoxShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterBoxShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleEmitterLineShape::ParticleEmitterLineShape(const PARTICLE_LINE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UINT32 ParticleEmitterLineShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		return spawnMultipleMode(this, mInfo.mode.type, mInfo.length, mInfo.mode.speed, 
			mInfo.mode.interval, random, particles, count, state);
	}

	void ParticleEmitterLineShape::_spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = Vector3(random.getSNorm() * mInfo.length * 0.5f, 0.0f, 0.0f);
		normal = Vector3::UNIT_Z;
	}

	void ParticleEmitterLineShape::_spawn(float t, Vector3& position, Vector3& normal) const
	{
		position = Vector3(t * mInfo.length - mInfo.length * 0.5f, 0.0f, 0.0f);
		normal = Vector3::UNIT_Z;
	}

	void ParticleEmitterLineShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		shape.setMin(Vector3(-mInfo.length * 0.5f, 0.0f, 0.0f));
		shape.setMax(Vector3(mInfo.length * 0.5f, 0.0f, 0.0f));

		velocity.setMin(Vector3::ZERO);
		velocity.setMax(Vector3::UNIT_Z);
	}

	SPtr<ParticleEmitterShape> ParticleEmitterLineShape::create(const PARTICLE_LINE_SHAPE_DESC& desc)
	{
		return bs_shared_ptr<ParticleEmitterShape>(bs_new<ParticleEmitterLineShape>(desc));
	}

	RTTITypeBase* ParticleEmitterLineShape::getRTTIStatic()
	{
		return ParticleEmitterLineShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterLineShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleEmitterCircleShape::ParticleEmitterCircleShape(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UINT32 ParticleEmitterCircleShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		return spawnMultipleMode(this, mInfo.mode.type, mInfo.arc.valueRadians(), mInfo.mode.speed * Math::DEG2RAD, 
			mInfo.mode.interval * Math::DEG2RAD, random, particles, count, state);
	}

	void ParticleEmitterCircleShape::_spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		Vector2 pos2D;
		if (Math::approxEquals(mInfo.arc.valueDegrees(), 360.0f))
			pos2D = random.getPointInCircleShell(mInfo.thickness);
		else
			pos2D = random.getPointInArcShell(mInfo.arc, mInfo.thickness);

		position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
		normal = Vector3::UNIT_Z;
	}

	void ParticleEmitterCircleShape::_spawn(float t, Vector3& position, Vector3& normal) const
	{
		const Vector2 pos2D(Math::cos(t), Math::sin(t));

		position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
		normal = Vector3::UNIT_Z;
	}

	void ParticleEmitterCircleShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		shape.setMin(Vector3(-mInfo.radius, -mInfo.radius, 0.0f));
		shape.setMax(Vector3(mInfo.radius, mInfo.radius, 0.0f));

		velocity.setMin(Vector3::ZERO);
		velocity.setMax(Vector3::UNIT_Z);
	}

	SPtr<ParticleEmitterShape> ParticleEmitterCircleShape::create(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
	{
		ParticleEmitterCircleShape* output = bs_new<ParticleEmitterCircleShape>(desc);
		return bs_shared_ptr<ParticleEmitterShape>(output);
	}

	RTTITypeBase* ParticleEmitterCircleShape::getRTTIStatic()
	{
		return ParticleEmitterCircleShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterCircleShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleEmitterRectShape::ParticleEmitterRectShape(const PARTICLE_RECT_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UINT32 ParticleEmitterRectShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		return spawnMultipleRandom(this, random, particles, count);
	}

	void ParticleEmitterRectShape::_spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position.x = random.getSNorm() * mInfo.extents.x;
		position.y = random.getSNorm() * mInfo.extents.y;
		position.z = 0.0f;

		normal = Vector3::UNIT_Z;
	}

	void ParticleEmitterRectShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		shape.setMin(Vector3(-mInfo.extents.x, -mInfo.extents.y, 0.0f));
		shape.setMax(Vector3(mInfo.extents.x, mInfo.extents.y, 0.0f));

		velocity.setMin(Vector3::ZERO);
		velocity.setMax(Vector3::UNIT_Z);
	}

	SPtr<ParticleEmitterShape> ParticleEmitterRectShape::create(const PARTICLE_RECT_SHAPE_DESC& desc)
	{
		return bs_shared_ptr<ParticleEmitterShape>(bs_new<ParticleEmitterRectShape>(desc));
	}

	RTTITypeBase* ParticleEmitterRectShape::getRTTIStatic()
	{
		return ParticleEmitterRectShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterRectShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	bool MeshEmissionHelper::initialize(const HMesh& mesh, bool perVertex, bool skinning)
	{
		// Validate
		if(mesh)
		{
			mMeshData = mesh->getCachedData();

			if(!mMeshData)
			{
				LOGWRN_VERBOSE("Particle emitter mesh not created with CPU caching, performing an expensive GPU read.");

				mMeshData = mesh->allocBuffer();
				mesh->readData(mMeshData);

				gCoreThread().submit(true);
			}
		}

		if(!mMeshData)
		{
			// No warning as user could want to add mesh data later
			return false;
		}

		const SPtr<VertexDataDesc>& vertexDesc = mMeshData->getVertexDesc();
		const VertexElement* positionElement = vertexDesc->getElement(VES_POSITION);
		if(positionElement == nullptr)
		{
			LOGERR("Mesh particle emitter requires position vertex data to be present in the provided mesh data.");
			return false;
		}

		if(positionElement->getType() != VET_FLOAT3)
		{
			LOGERR("Mesh particle emitter requires position vertex data to use 3D vectors for individual elements.");
			return false;
		}

		if(!perVertex && (mMeshData->getNumIndices() % 3 != 0))
		{
			LOGERR("Unless using the per-vertex emission mode, mesh particle emitter requires the number of indices to be \
				 divisible by three, using a triangle list layout.");
			return false;
		}

		if(skinning)
		{
			const VertexElement* blendIdxElement = vertexDesc->getElement(VES_BLEND_INDICES);
			const VertexElement* blendWeightElement = vertexDesc->getElement(VES_BLEND_WEIGHTS);

			if (blendIdxElement == nullptr || blendWeightElement == nullptr)
			{
				LOGERR("Skinned mesh particle emitter requires blend indices and blend weight data to be present in the \
				provided mesh data.");
				return false;
			}

			if (blendIdxElement->getType() != VET_UBYTE4)
			{
				LOGERR("Skinned mesh particle emitter requires blend indices to be a 4-byte encoded format.");
				return false;
			}

			if (blendWeightElement->getType() != VET_FLOAT4)
			{
				LOGERR("Skinned mesh particle emitter requires blend weights to be a 4D vector format.");
				return false;
			}
		}

		// Initialize
		mVertices = mMeshData->getElementData(VES_POSITION);
		mNumVertices = mMeshData->getNumVertices();
		mVertexStride = vertexDesc->getVertexStride();

		const VertexElement* normalElement = vertexDesc->getElement(VES_NORMAL);

		mNormals = nullptr;
		if(normalElement)
		{
			if(normalElement->getType() == VET_UBYTE4_NORM)
			{
				mNormals = mMeshData->getElementData(VES_NORMAL);
				m32BitNormals = true;
			}
			else if(normalElement->getType() == VET_FLOAT3)
			{
				mNormals = mMeshData->getElementData(VES_NORMAL);
				m32BitNormals = false;
			}
		}

		if(skinning)
		{
			mBoneIndices = mMeshData->getElementData(VES_BLEND_INDICES);
			mBoneWeights = mMeshData->getElementData(VES_BLEND_WEIGHTS);
		}

		if(!perVertex)
			mWeightedTriangles.calculate(*mMeshData);

		return true;
	}

	void MeshEmissionHelper::getSequentialVertex(class Vector3& position, class Vector3& normal, UINT32& idx) const
	{
		idx = mNextSequentialIdx;
		position = *(Vector3*)(mVertices + mVertexStride * idx);

		if (mNormals)
		{
			if (m32BitNormals)
				normal = MeshUtility::unpackNormal(mNormals + mVertexStride * idx);
			else
				normal = *(Vector3*)(mNormals + mVertexStride * idx);
		}
		else
			normal = Vector3::UNIT_Z;

		mNextSequentialIdx = (mNextSequentialIdx + 1) % mNumVertices;
	}

	void MeshEmissionHelper::getRandomVertex(const Random& random, Vector3& position, Vector3& normal, 
		UINT32& idx) const
	{
		idx = random.get() % mNumVertices;
		position = *(Vector3*)(mVertices + mVertexStride * idx);

		if (mNormals)
		{
			if (m32BitNormals)
				normal = MeshUtility::unpackNormal(mNormals + mVertexStride * idx);
			else
				normal = *(Vector3*)(mNormals + mVertexStride * idx);
		}
		else
			normal = Vector3::UNIT_Z;
	}

	void MeshEmissionHelper::getRandomEdge(const Random& random, std::array<Vector3, 2>& position, 
		std::array<Vector3, 2>& normal, std::array<UINT32, 2>& idx) const
	{
		std::array<UINT32, 3> triIndices;
		mWeightedTriangles.getTriangle(random, triIndices);

		// Pick edge
		// Note: Longer edges should be given higher chance, but we're assuming they are all equal length for performance
		const int32_t edge = random.getRange(0, 2);
		switch (edge)
		{
		default:
		case 0:
			idx[0] = triIndices[0];
			idx[1] = triIndices[1];
			break;
		case 1:
			idx[0] = triIndices[1];
			idx[1] = triIndices[2];
			break;
		case 2:
			idx[0] = triIndices[2];
			idx[1] = triIndices[0];
			break;
		}

		position[0] = *(Vector3*)(mVertices + mVertexStride * idx[0]);
		position[1] = *(Vector3*)(mVertices + mVertexStride * idx[1]);

		if (mNormals)
		{
			if (m32BitNormals)
			{
				normal[0] = MeshUtility::unpackNormal(mNormals + mVertexStride * idx[0]);
				normal[1] = MeshUtility::unpackNormal(mNormals + mVertexStride * idx[1]);
			}
			else
			{
				normal[0] = *(Vector3*)(mNormals + mVertexStride * idx[0]);
				normal[1] = *(Vector3*)(mNormals + mVertexStride * idx[1]);
			}
		}
		else
		{
			normal[0] = Vector3::UNIT_Z;
			normal[1] = Vector3::UNIT_Z;
		}
	}

	void MeshEmissionHelper::getRandomTriangle(const Random& random, std::array<Vector3, 3>& position, 
		std::array<Vector3, 3>& normal, std::array<UINT32, 3>& idx) const
	{
		mWeightedTriangles.getTriangle(random, idx);

		for (uint32_t i = 0; i < 3; i++)
		{
			position[i] = *(Vector3*)(mVertices + mVertexStride * idx[i]);

			if (mNormals)
			{
				if (m32BitNormals)
					normal[i] = MeshUtility::unpackNormal(mNormals + mVertexStride * idx[i]);
				else
					normal[i] = *(Vector3*)(mNormals + mVertexStride * idx[i]);
			}
			else
				normal[i] = Vector3::UNIT_Z;
		}
	}

	Matrix4 MeshEmissionHelper::getBlendMatrix(const Matrix4* bones, UINT32 vertexIdx) const
	{
		if(bones)
		{
			const UINT32 boneIndices = *(UINT32*)(mBoneIndices + vertexIdx * mVertexStride);
			const Vector4& boneWeights = *(Vector4*)(mBoneWeights + vertexIdx * mVertexStride);

			return
				bones[boneIndices & 0xFF] * boneWeights[0] +
				bones[(boneIndices >> 8) & 0xFF] * boneWeights[1] +
				bones[(boneIndices >> 16) & 0xFF] * boneWeights[2] +
				bones[(boneIndices >> 24) & 0xFF] * boneWeights[3];
		}
		else
			return Matrix4::IDENTITY;
	}

	ParticleEmitterStaticMeshShape::ParticleEmitterStaticMeshShape(const PARTICLE_STATIC_MESH_SHAPE_DESC& desc)
		:mInfo(desc)
	{
		mIsValid = mMeshEmissionHelper.initialize(desc.mesh, desc.type == ParticleEmitterMeshType::Vertex, false);
	}

	UINT32 ParticleEmitterStaticMeshShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		if(count == 0)
			return particles.getParticleCount();

		switch(mInfo.type)
		{
		case ParticleEmitterMeshType::Vertex: 
			if(mInfo.sequential)
			{
				return spawnMultiple(particles, count, [this](UINT32 idx, Vector3& position, Vector3& normal)
				{
					UINT32 vertexIdx;
					mMeshEmissionHelper.getSequentialVertex(position, normal, vertexIdx);
				});
			}
			else
			{
				return spawnMultiple(particles, count, [this, &random](UINT32 idx, Vector3& position, Vector3& normal)
				{
					UINT32 vertexIdx;
					mMeshEmissionHelper.getRandomVertex(random, position, normal, vertexIdx);
				});
			}
		case ParticleEmitterMeshType::Edge:
			return spawnMultiple(particles, count, [this, &random](UINT32 idx, Vector3& position, Vector3& normal)
			{
				std::array<Vector3, 2> edgePositions, edgeNormals;
				std::array<UINT32, 2> edgeIndices;

				mMeshEmissionHelper.getRandomEdge(random, edgePositions, edgeNormals, edgeIndices);

				const float rnd = random.getUNorm();
				position = Math::lerp(rnd, edgePositions[0], edgePositions[1]);
				normal = Math::lerp(rnd, edgeNormals[0], edgeNormals[1]);
			});
		default:
		case ParticleEmitterMeshType::Triangle:
			return spawnMultiple(particles, count, [this, &random](UINT32 idx, Vector3& position, Vector3& normal)
			{
				std::array<Vector3, 3> triPositions, triNormals;
				std::array<UINT32, 3> triIndices;

				mMeshEmissionHelper.getRandomTriangle(random, triPositions, triNormals, triIndices);

				position = Vector3::ZERO;
				normal = Vector3::ZERO;
				Vector3 barycenter = random.getBarycentric();

				for (uint32_t i = 0; i < 3; i++)
				{
					position += triPositions[i] * barycenter[i];
					normal += triNormals[i] * barycenter[i];
				}
			});
		}
	}

	void ParticleEmitterStaticMeshShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		if(mInfo.mesh.isLoaded(false))
			shape = mInfo.mesh->getProperties().getBounds().getBox();
		else
			shape = AABox::BOX_EMPTY;

		velocity.setMin(-Vector3::ONE);
		velocity.setMax(Vector3::ONE);
	}

	SPtr<ParticleEmitterShape> ParticleEmitterStaticMeshShape::create(const PARTICLE_STATIC_MESH_SHAPE_DESC& desc)
	{
		ParticleEmitterStaticMeshShape* output = bs_new<ParticleEmitterStaticMeshShape>(desc);
		return bs_shared_ptr<ParticleEmitterShape>(output);
	}

	RTTITypeBase* ParticleEmitterStaticMeshShape::getRTTIStatic()
	{
		return ParticleEmitterStaticMeshShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterStaticMeshShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleEmitterSkinnedMeshShape::ParticleEmitterSkinnedMeshShape(const PARTICLE_SKINNED_MESH_SHAPE_DESC& desc)
		:mInfo(desc)
	{
		HMesh mesh;
		if(desc.renderable)
			mesh = desc.renderable->getMesh();

		mIsValid = mMeshEmissionHelper.initialize(mesh, desc.type == ParticleEmitterMeshType::Vertex, false);
	}

	UINT32 ParticleEmitterSkinnedMeshShape::_spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		if(count == 0)
			return particles.getParticleCount();

		const Matrix4* bones = nullptr;

		if(mInfo.renderable)
		{
			const SPtr<Animation>& animation = mInfo.renderable->getAnimation();
			if(animation)
			{
				const UINT64 animId = animation->_getId();

				if(state.animData)
				{
					const auto iterFind = state.animData->infos.find(animId);
					if(iterFind != state.animData->infos.end())
						bones = &state.animData->transforms[iterFind->second.poseInfo.startIdx];
				}
			}
		}

		switch(mInfo.type)
		{
		case ParticleEmitterMeshType::Vertex: 
			if(mInfo.sequential)
			{
				float dt = (state.timeStep / count) / state.length;

				return spawnMultiple(particles, count, [this, &state, dt, bones]
				(UINT32 idx, Vector3& position, Vector3& normal)
				{
					UINT32 vertexIdx;
					mMeshEmissionHelper.getSequentialVertex(position, normal, vertexIdx);

					Matrix4 blendMatrix = mMeshEmissionHelper.getBlendMatrix(bones, vertexIdx);
					position = blendMatrix.multiplyAffine(position);
					normal = blendMatrix.multiplyDirection(normal);
				});
			}
			else
			{
				return spawnMultiple(particles, count, [this, &random, bones]
				(UINT32 idx, Vector3& position, Vector3& normal)
				{
					UINT32 vertexIdx;
					mMeshEmissionHelper.getRandomVertex(random, position, normal, vertexIdx);

					Matrix4 blendMatrix = mMeshEmissionHelper.getBlendMatrix(bones, vertexIdx);
					position = blendMatrix.multiplyAffine(position);
					normal = blendMatrix.multiplyDirection(normal);
				});
			}
		case ParticleEmitterMeshType::Edge:
			return spawnMultiple(particles, count, [this, &random, bones]
			(UINT32 idx, Vector3& position, Vector3& normal)
			{
				std::array<Vector3, 2> edgePositions, edgeNormals;
				std::array<UINT32, 2> edgeIndices;

				mMeshEmissionHelper.getRandomEdge(random, edgePositions, edgeNormals, edgeIndices);

				for(uint32_t i = 0; i < 2; i++)
				{
					Matrix4 blendMatrix = mMeshEmissionHelper.getBlendMatrix(bones, edgeIndices[i]);
					edgePositions[i] = blendMatrix.multiplyAffine(edgePositions[i]);
					edgeNormals[i] = blendMatrix.multiplyAffine(edgeNormals[i]);
				}

				const float rnd = random.getUNorm();
				position = Math::lerp(rnd, edgePositions[0], edgePositions[1]);
				normal = Math::lerp(rnd, edgeNormals[0], edgeNormals[1]);
			});
		default:
		case ParticleEmitterMeshType::Triangle:
			return spawnMultiple(particles, count, [this, &random, bones]
			(UINT32 idx, Vector3& position, Vector3& normal)
			{
				std::array<Vector3, 3> triPositions, triNormals;
				std::array<UINT32, 3> triIndices;

				mMeshEmissionHelper.getRandomTriangle(random, triPositions, triNormals, triIndices);

				position = Vector3::ZERO;
				normal = Vector3::ZERO;
				Vector3 barycenter = random.getBarycentric();

				for(uint32_t i = 0; i < 3; i++)
				{
					Matrix4 blendMatrix = mMeshEmissionHelper.getBlendMatrix(bones, triIndices[i]);
					triPositions[i] = blendMatrix.multiplyAffine(triPositions[i]);
					triNormals[i] = blendMatrix.multiplyAffine(triNormals[i]);
				}

				for (uint32_t i = 0; i < 3; i++)
				{
					position += triPositions[i] * barycenter[i];
					normal += triNormals[i] * barycenter[i];
				}
			});
		};
	}

	void ParticleEmitterSkinnedMeshShape::calcBounds(AABox& shape, AABox& velocity) const
	{
		if(mInfo.renderable)
		{
			const SPtr<Animation>& anim = mInfo.renderable->getAnimation();
			if(anim)
			{
				// No culling, make the box infinite
				if(!anim->getCulling())
					shape = AABox::INF_BOX;
				else
					shape = anim->getBounds();
			}
			else
			{
				const HMesh& mesh = mInfo.renderable->getMesh();
				if (mesh.isLoaded(false))
					shape = mesh->getProperties().getBounds().getBox();
				else
					shape = AABox::BOX_EMPTY;
			}
		}
		else
			shape = AABox::BOX_EMPTY;

		velocity.setMin(-Vector3::ONE);
		velocity.setMax(Vector3::ONE);
	}

	SPtr<ParticleEmitterShape> ParticleEmitterSkinnedMeshShape::create(const PARTICLE_SKINNED_MESH_SHAPE_DESC& desc)
	{
		ParticleEmitterSkinnedMeshShape* output = bs_new<ParticleEmitterSkinnedMeshShape>(desc);
		return bs_shared_ptr<ParticleEmitterShape>(output);
	}

	RTTITypeBase* ParticleEmitterSkinnedMeshShape::getRTTIStatic()
	{
		return ParticleEmitterSkinnedMeshShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterSkinnedMeshShape::getRTTI() const
	{
		return getRTTIStatic();
	} 

	void ParticleEmitter::setEmissionBursts(Vector<ParticleBurst> bursts)
	{
		mBursts = std::move(bursts);
		mBurstAccumulator.resize(mBursts.size());

		for(auto& entry : mBurstAccumulator)
			entry = 0.0f;
	}

	void ParticleEmitter::spawn(Random& random, const ParticleSystemState& state, ParticleSet& set) const
	{
		if(!mShape || !mShape->isValid())
			return;

		const float emitterT = state.nrmTimeEnd;

		// Continous emission rate
		const float rate = mEmissionRate.evaluate(emitterT, random);

		mEmitAccumulator += rate * state.timeStep;
		const auto numContinous = (UINT32)mEmitAccumulator;
		mEmitAccumulator -= (float)numContinous;

		const float spacing = numContinous > 0 ? 1.0f / (float)numContinous : 1.0f;

		// Bursts
		UINT32 numBurst = 0;
		const auto emitBursts = [this, &emitterT, &random](float start, float end)
		{
			constexpr float MIN_BURST_INTERVAL = 0.01f;

			UINT32 numBurst = 0;
			for (UINT32 i = 0; i < (UINT32)mBursts.size(); i++)
			{
				const ParticleBurst& burst = mBursts[i];

				const float relT0 = std::max(0.0f, start - burst.time);
				const float relT1 = end - burst.time;
				if (relT1 <= 0.0f)
					continue;

				// Handle initial burst cycle
				if (relT0 == 0.0f)
					numBurst += (UINT32)burst.count.evaluate(emitterT, random);

				// Handle remaining cycles
				const float dt = relT1 - relT0;
				const float interval = std::max(burst.interval, MIN_BURST_INTERVAL);

				const float emitDuration = dt + mBurstAccumulator[i];
				const UINT32 emitCycles = Math::floorToPosInt(emitDuration / interval);
				mBurstAccumulator[i] = emitDuration - emitCycles * interval;

				for (UINT32 j = 0; j < emitCycles; j++)
					numBurst += (UINT32)burst.count.evaluate(emitterT, random);
			}

			return numBurst;
		};

		// Handle loop
		if(state.timeEnd < state.timeStart)
		{
			numBurst += emitBursts(state.timeStart, state.length);

			// Reset accumulator
			for(auto& entry : mBurstAccumulator)
				entry = 0.0f;

			numBurst += emitBursts(0.0f, state.timeEnd);
		}
		else
			numBurst += emitBursts(state.timeStart, state.timeEnd);

		UINT32 numToSpawn = numContinous + numBurst;

		const UINT32 numPartices = set.getParticleCount() + numToSpawn;
		if(!state.gpuSimulated)
		{
			if (numPartices > state.maxParticles)
				numToSpawn = state.maxParticles - set.getParticleCount();
		}

		const UINT32 firstIdx = mShape->_spawn(random, set, numToSpawn, state);
		const UINT32 endIdx = firstIdx + numToSpawn;

		ParticleSetData& particles = set.getParticles();

		for(UINT32 i = firstIdx; i < endIdx; i++)
		{
			const float lifetime = mInitialLifetime.evaluate(emitterT, random);

			particles.initialLifetime[i] = lifetime;
			particles.lifetime[i] = lifetime;
		}

		for(UINT32 i = firstIdx; i < endIdx; i++)
			particles.velocity[i] *= mInitialSpeed.evaluate(emitterT, random);

		if(!mUse3DSize)
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
			{
				const float size = mInitialSize.evaluate(emitterT, random);

				// Encode UV flip in size XY as sign
				const float flipU = random.getUNorm() < mFlipU ? -1.0f : 1.0f;
				const float flipV = random.getUNorm() < mFlipV ? -1.0f : 1.0f;

				particles.size[i] = Vector3(size * flipU, size * flipV, size);
			}
		}
		else
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
			{
				Vector3 size = mInitialSize3D.evaluate(emitterT, random);

				// Encode UV flip in size XY as sign
				size.x *= random.getUNorm() < mFlipU ? -1.0f : 1.0f;
				size.y *= random.getUNorm() < mFlipV ? -1.0f : 1.0f;

				particles.size[i] = size;
			}
		}

		if(mRandomOffset > 0.0f)
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
				particles.position[i] += Vector3(random.getSNorm(), random.getSNorm(), random.getSNorm()) * mRandomOffset;
		}

		if(!mUse3DRotation)
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
			{
				const float rotation = mInitialRotation.evaluate(emitterT, random);
				particles.rotation[i] = Vector3(rotation, 0.0f, 0.0f);
			}
		}
		else
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
			{
				const Vector3 rotation = mInitialRotation3D.evaluate(emitterT, random);
				particles.rotation[i] = rotation;
			}
		}

		for(UINT32 i = firstIdx; i < endIdx; i++)
			particles.color[i] = mInitialColor.evaluate(emitterT, random);

		for(UINT32 i = firstIdx; i < endIdx; i++)
			particles.seed[i] = random.get();

		for(UINT32 i = firstIdx; i < endIdx; i++)
			particles.frame[i] = 0.0f;

		// If in world-space we apply the transform here, otherwise we apply it in the rendering code
		if(state.worldSpace)
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
				particles.position[i] = state.localToWorld.multiplyAffine(particles.position[i]);

			for (UINT32 i = firstIdx; i < endIdx; i++)
				particles.velocity[i] = state.localToWorld.multiplyDirection(particles.velocity[i]);
		}
	}	
	
	RTTITypeBase* ParticleEmitter::getRTTIStatic()
	{
		return ParticleEmitterRTTI::instance();
	}

	RTTITypeBase* ParticleEmitter::getRTTI() const
	{
		return getRTTIStatic();
	}
}
