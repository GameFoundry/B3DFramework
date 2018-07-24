//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleEvolver.h"
#include "Private/Particles/BsParticleSet.h"
#include "Image/BsSpriteTexture.h"
#include "BsParticleSystem.h"
#include "Material/BsMaterial.h"
#include "Math/BsRay.h"
#include "Physics/BsPhysics.h"
#include "Physics/BsCollider.h"
#include "Math/BsLineSegment3.h"
#include "Private/RTTI/BsParticleSystemRTTI.h"

namespace bs
{
	// Arbitrary random numbers to add variation to different random particle properties, since we use just a single
	// seed value per particle
	static constexpr UINT32 PARTICLE_ROW_VARIATION = 0x1e8b2f4a;
	static constexpr UINT32 PARTICLE_ORBIT_VELOCITY = 0x24c00a5b;
	static constexpr UINT32 PARTICLE_ORBIT_RADIAL = 0x35978d21;
	static constexpr UINT32 PARTICLE_LINEAR_VELOCITY = 0x0a299430;

	/** Helper method that applies a transform to either a point or a direction. */
	template<bool dir>
	Vector3 applyTransform(const Matrix4& tfrm, const Vector3& input)
	{
		return tfrm.multiplyAffine(input);		
	}

	template<>
	Vector3 applyTransform<true>(const Matrix4& tfrm, const Vector3& input)
	{
		return tfrm.multiplyDirection(input);
	}

	/**
	 * Evaluates a 3D vector distribution and transforms the output into the same space as the particle system. 
	 * @p inWorldSpace parameter controls whether the values in the distribution are assumed to be in world or local space.
	 * 
	 * @tparam	dir		If true the evaluated vector is assumed to be a direction, otherwise a point.
	 */
	template<bool dir = false>
	Vector3 evaluateTransformed(const Vector3Distribution& distribution, const ParticleSystemState& state, float t, 
		const Random& factor, bool inWorldSpace)
	{
		const Vector3 output = distribution.evaluate(t, factor);

		if(state.worldSpace == inWorldSpace)
			return output;

		if(state.worldSpace)
			return applyTransform<dir>(state.localToWorld, output);
		else
			return applyTransform<dir>(state.worldToLocal, output);
	}

	ParticleTextureAnimation::ParticleTextureAnimation(const PARTICLE_TEXTURE_ANIMATION_DESC& desc)
		:mDesc(desc)
	{ }

	void ParticleTextureAnimation::evolve(Random& random, const ParticleSystemState& state, ParticleSet& set) const
	{
		const UINT32 count = set.getParticleCount();
		ParticleSetData& particles = set.getParticles();

		SpriteTexture* texture = nullptr;
		if(mParent)
		{
			const HMaterial& material = mParent->getMaterial();
			if(material.isLoaded(false))
			{
				const HSpriteTexture& spriteTex = material->getSpriteTexture("gTexture");
				if(spriteTex.isLoaded(true))
					texture = spriteTex.get();
			}
		}

		bool hasValidAnimation = texture != nullptr;
		if(hasValidAnimation)
		{
			const SpriteSheetGridAnimation& gridAnim = texture->getAnimation();
			hasValidAnimation = gridAnim.numRows > 0 && gridAnim.numColumns > 0 && gridAnim.count > 0;
		}

		if(!hasValidAnimation)
		{
			for (UINT32 i = 0; i < count; i++)
				particles.frame[i] = 0.0f;

			return;
		}
		
		const SpriteSheetGridAnimation& gridAnim = texture->getAnimation();

		for (UINT32 i = 0; i < count; i++)
		{
			UINT32 frameOffset;
			UINT32 numFrames;
			if (mDesc.randomizeRow)
			{
				const UINT32 rowSeed = particles.seed[i] + PARTICLE_ROW_VARIATION;
				const UINT32 row = Random(rowSeed).getRange(0, gridAnim.numRows);

				frameOffset = row * gridAnim.numColumns;
				numFrames = gridAnim.numColumns;
			}
			else
			{
				frameOffset = 0;
				numFrames = gridAnim.count;
			}

			float particleT = (particles.initialLifetime[i] - particles.lifetime[i]) / particles.initialLifetime[i];
			particleT = Math::repeat(mDesc.numCycles * particleT, 1.0f);

			const float frame = particleT * numFrames;
			particles.frame[i] = frameOffset + Math::clamp(frame, 0.0f, (float)(numFrames - 1));
		}
	}

	RTTITypeBase* ParticleTextureAnimation::getRTTIStatic()
	{
		return ParticleTextureAnimationRTTI::instance();
	}

	RTTITypeBase* ParticleTextureAnimation::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleOrbit::ParticleOrbit(const PARTICLE_ORBIT_DESC& desc)
		:mDesc(desc)
	{ }

	void ParticleOrbit::evolve(Random& random, const ParticleSystemState& state, ParticleSet& set) const
	{
		const UINT32 count = set.getParticleCount();
		ParticleSetData& particles = set.getParticles();

		const Vector3 center = evaluateTransformed(mDesc.center, state, state.nrmTime, random, mDesc.worldSpace);
		for (UINT32 i = 0; i < count; i++)
		{
			const float particleT = (particles.initialLifetime[i] - particles.lifetime[i]) / particles.initialLifetime[i];

			const UINT32 velocitySeed = particles.seed[i] + PARTICLE_ORBIT_VELOCITY;
			Vector3 orbitVelocity = evaluateTransformed<true>(mDesc.velocity, state, particleT, Random(velocitySeed), 
				mDesc.worldSpace);
			orbitVelocity *= Math::TWO_PI;
			orbitVelocity *= state.timeStep;

			const Matrix3 rotation(Radian(orbitVelocity.x), Radian(orbitVelocity.y), Radian(orbitVelocity.z));

			const Vector3 point = particles.position[i] - center;
			const Vector3 newPoint = rotation.multiply(point);

			Vector3 velocity = newPoint - point;

			const UINT32 radialSeed = particles.seed[i] + PARTICLE_ORBIT_RADIAL;
			const float radial = mDesc.radial.evaluate(particleT, Random(radialSeed).getUNorm());
			if(radial != 0.0f)
				velocity += Vector3::normalize(point) * radial * state.timeStep;

			particles.position[i] += velocity;
		}
	}

	RTTITypeBase* ParticleOrbit::getRTTIStatic()
	{
		return ParticleOrbitRTTI::instance();
	}

	RTTITypeBase* ParticleOrbit::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleVelocity::ParticleVelocity(const PARTICLE_VELOCITY_DESC& desc)
		:mDesc(desc)
	{ }

	void ParticleVelocity::evolve(Random& random, const ParticleSystemState& state, ParticleSet& set) const
	{
		const UINT32 count = set.getParticleCount();
		ParticleSetData& particles = set.getParticles();

		for (UINT32 i = 0; i < count; i++)
		{
			const float particleT = (particles.initialLifetime[i] - particles.lifetime[i]) / particles.initialLifetime[i];

			const UINT32 velocitySeed = particles.seed[i] + PARTICLE_ORBIT_VELOCITY;
			const Vector3 velocity = evaluateTransformed<true>(mDesc.velocity, state, particleT, Random(velocitySeed), 
				mDesc.worldSpace) * state.timeStep;

			particles.position[i] += velocity;
		}
	}

	RTTITypeBase* ParticleVelocity::getRTTIStatic()
	{
		return ParticleVelocityRTTI::instance();
	}

	RTTITypeBase* ParticleVelocity::getRTTI() const
	{
		return getRTTIStatic();
	}

	/** Information about a particle collision. */
	struct ParticleHitInfo
	{
		Vector3 position;
		Vector3 normal;
		UINT32 idx;
	};

	/** Calculates the new position and velocity after a particle was detected to be colliding. */
	void calcCollisionResponse(Vector3& position, Vector3& velocity, const ParticleHitInfo& hitInfo, 
		const PARTICLE_COLLISONS_DESC& desc)
	{
		Vector3 diff = position - hitInfo.position;

		// Reflect & dampen
		const float dampenFactor = 1.0f - desc.dampening;

		Vector3 reflectedPos = diff.reflect(hitInfo.normal) * dampenFactor;
		Vector3 reflectedVel = velocity.reflect(hitInfo.normal) * dampenFactor;

		// Bounce
		const float restitutionFactor = 1.0f - desc.restitution;

		reflectedPos -= hitInfo.normal * reflectedPos.dot(hitInfo.normal) * restitutionFactor;
		reflectedVel -= hitInfo.normal * reflectedVel.dot(hitInfo.normal) * restitutionFactor;

		position = hitInfo.position + reflectedPos;
		velocity = reflectedVel;
	}

	UINT32 groupRaycast(LineSegment3* segments, ParticleHitInfo* hits, UINT32 numRays, UINT64 layer)
	{
		if(numRays == 0)
			return 0;

		// Calculate bounds of all rays
		AABox groupBounds = AABox::INF_BOX;
		for(UINT32 i = 0; i < numRays; i++)
		{
			groupBounds.merge(segments[i].start);
			groupBounds.merge(segments[i].end);
		}

		Vector<Collider*> hitColliders = gPhysics()._boxOverlap(groupBounds, Quaternion::IDENTITY, layer);
		if(hitColliders.empty())
			return 0;

		UINT32 numHits = 0;
		for(UINT32 i = 0; i < numRays; i++)
		{
			float nearestHit = std::numeric_limits<float>::max();
			ParticleHitInfo hitInfo;
			hitInfo.idx = i;

			Vector3 diff = segments[i].end - segments[i].start;
			const float length = diff.length();

			if(Math::approxEquals(length, 0.0f))
				continue;

			Ray ray;
			ray.setOrigin(segments[i].start);
			ray.setDirection(diff / length);

			for(auto& collider : hitColliders)
			{
				PhysicsQueryHit queryHit;
				if(collider->rayCast(ray, queryHit, length))
				{
					if(queryHit.distance < nearestHit)
					{
						nearestHit = queryHit.distance;

						hitInfo.position = queryHit.point;
						hitInfo.normal = queryHit.normal;
					}
				}
			}

			if(nearestHit != std::numeric_limits<float>::max())
				hits[numHits++] = hitInfo;
		}

		return numHits;
	}

	ParticleCollisions::ParticleCollisions(const PARTICLE_COLLISONS_DESC& desc)
		:mDesc(desc)
	{
		mDesc.restitution = std::max(mDesc.restitution, 0.0f);
		mDesc.dampening = Math::clamp01(mDesc.dampening);
		mDesc.lifetimeLoss = Math::clamp01(mDesc.lifetimeLoss);
		mDesc.radius = std::max(mDesc.radius, 0.0f);
	}

	void ParticleCollisions::evolve(Random& random, const ParticleSystemState& state, ParticleSet& set) const
	{
		const UINT32 numParticles = set.getParticleCount();
		ParticleSetData& particles = set.getParticles();

		if(mDesc.mode == ParticleCollisionMode::Plane)
		{
			const auto numPlanes = (UINT32)mCollisionPlanes.size();

			Plane* planes;
			Plane* localPlanes = nullptr;

			// If particles are in world space, we can just use collision planes as is
			if(state.worldSpace)
				planes = (Plane*)mCollisionPlanes.data();
			else
			{
				const Matrix4& worldToLocal = state.worldToLocal;
				localPlanes = bs_stack_alloc<Plane>(numParticles);

				for (UINT32 i = 0; i < numPlanes; i++)
					localPlanes[i] = worldToLocal.multiplyAffine(mCollisionPlanes[i]);

				planes = localPlanes;
			}

			for(UINT32 i = 0; i < numParticles; i++)
			{
				Vector3& position = particles.position[i];
				Vector3& velocity = particles.velocity[i];

				for (UINT32 j = 0; j < numPlanes; j++)
				{
					const Plane& plane = planes[j];

					const float dist = plane.getDistance(position);
					if(dist > mDesc.radius)
						continue;

					const float distToTravelAlongNormal = plane.normal.dot(velocity);

					// Ignore movement parallel to the plane
					if (Math::approxEquals(distToTravelAlongNormal, 0.0f))
						continue;

					const float distFromBoundary = mDesc.radius - dist;
					const float rayT = distFromBoundary / distToTravelAlongNormal;

					ParticleHitInfo hitInfo;
					hitInfo.normal = plane.normal;
					hitInfo.position = position + velocity * rayT;
					hitInfo.idx = i;

					calcCollisionResponse(position, velocity, hitInfo, mDesc);
					particles.lifetime[i] -= mDesc.lifetimeLoss * particles.initialLifetime[i];

					break;
				}
			}

			if(localPlanes)
				bs_stack_free(localPlanes);
		}
		else
		{
			const UINT32 rayStart = 0;
			const UINT32 rayEnd = numParticles;
			const UINT32 numRays = rayEnd - rayStart;

			const auto segments = bs_stack_alloc<LineSegment3>(numRays);
			const auto hits = bs_stack_alloc<ParticleHitInfo>(numRays);

			for(UINT32 i = 0; i < numRays; i++)
			{
				const Vector3& prevPosition = particles.prevPosition[rayStart + i];
				const Vector3& position = particles.position[rayStart + i];

				segments[i] = LineSegment3(prevPosition, position);
			}

			if(!state.worldSpace)
			{
				for (UINT32 i = 0; i < numRays; i++)
				{
					segments[i].start = state.localToWorld.multiplyAffine(segments[i].start);
					segments[i].end = state.localToWorld.multiplyAffine(segments[i].end);
				}
			}

			const UINT32 numHits = groupRaycast(segments, hits, numRays, mDesc.layer);

			if(!state.worldSpace)
			{
				for (UINT32 i = 0; i < numHits; i++)
				{
					hits[i].position = state.worldToLocal.multiplyAffine(hits[i].position);
					hits[i].normal = state.worldToLocal.multiplyDirection(hits[i].normal);
				}
			}

			for(UINT32 i = 0; i < numHits; i++)
			{
				ParticleHitInfo& hitInfo = hits[i];
				const UINT32 particleIdx = rayStart + hitInfo.idx;
				
				Vector3& position = particles.position[particleIdx];
				Vector3& velocity = particles.velocity[particleIdx];

				calcCollisionResponse(position, velocity, hitInfo, mDesc);

				particles.lifetime[particleIdx] -= mDesc.lifetimeLoss * particles.initialLifetime[particleIdx];
			}

			bs_stack_free(hits);
			bs_stack_free(segments);
		}
	}

	RTTITypeBase* ParticleCollisions::getRTTIStatic()
	{
		return ParticleCollisionsRTTI::instance();
	}

	RTTITypeBase* ParticleCollisions::getRTTI() const
	{
		return getRTTIStatic();
	}
}
