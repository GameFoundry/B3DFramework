//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/BsParticleEvolver.h"
#include "Private/Particles/BsParticleSet.h"
#include "Private/RTTI/BsParticleSystemRTTI.h"
#include "Particles/BsVectorField.h"
#include "Image/BsSpriteTexture.h"
#include "BsParticleSystem.h"
#include "Material/BsMaterial.h"
#include "Math/BsRay.h"
#include "Physics/BsPhysics.h"
#include "Physics/BsCollider.h"
#include "Math/BsLineSegment3.h"
#include "Material/BsShader.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsSceneManager.h"

using namespace bs;

// Arbitrary random numbers to add variation to different random particle properties, since we use just a single
// seed value per particle
static constexpr u32 PARTICLE_ROW_VARIATION = 0x1e8b2f4a;
static constexpr u32 PARTICLE_ORBIT_VELOCITY = 0x24c00a5b;
static constexpr u32 PARTICLE_ORBIT_RADIAL = 0x35978d21;
static constexpr u32 PARTICLE_LINEAR_VELOCITY = 0x0a299430;
static constexpr u32 PARTICLE_FORCE = 0x1b618144;
static constexpr u32 PARTICLE_COLOR = 0x378578b2;
static constexpr u32 PARTICLE_SIZE = 0x91088409;
static constexpr u32 PARTICLE_ROTATION = 0x4680eaa4;

/** Helper method that applies a transform to either a point or a direction. */
template <bool dir>
Vector3 applyTransform(const Matrix4& tfrm, const Vector3& input)
{
	return tfrm.MultiplyAffine(input);
}

template <>
Vector3 applyTransform<true>(const Matrix4& tfrm, const Vector3& input)
{
	return tfrm.MultiplyDirection(input);
}

/**
 * Evaluates a 3D vector distribution and transforms the output into the same space as the particle system.
 * @p inWorldSpace parameter controls whether the values in the distribution are assumed to be in world or local space.
 *
 * @tparam	dir		If true the evaluated vector is assumed to be a direction, otherwise a point.
 */
template <bool dir = false>
Vector3 evaluateTransformed(const Vector3Distribution& distribution, const ParticleSystemState& state, float t, const Random& factor, bool inWorldSpace)
{
	const Vector3 output = distribution.Evaluate(t, factor);

	if(state.WorldSpace == inWorldSpace)
		return output;

	if(state.WorldSpace)
		return applyTransform<dir>(state.LocalToWorld, output);
	else
		return applyTransform<dir>(state.WorldToLocal, output);
}

ParticleTextureAnimation::ParticleTextureAnimation(const PARTICLE_TEXTURE_ANIMATION_DESC& desc)
	: mDesc(desc)
{}

void ParticleTextureAnimation::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	SpriteTexture* texture = nullptr;
	const HMaterial& material = state.System->GetSettings().Material;
	if(material.IsLoaded(false))
	{
		const HShader& shader = material->GetShader();
		if(shader->HasTextureParam("gTexture"))
		{
			const HSpriteTexture& spriteTex = material->GetSpriteTexture("gTexture");
			if(spriteTex.IsLoaded(true))
				texture = spriteTex.Get();
		}

		if(shader->HasTextureParam("gAlbedoTex"))
		{
			const HSpriteTexture& spriteTex = material->GetSpriteTexture("gAlbedoTex");
			if(spriteTex.IsLoaded(true))
				texture = spriteTex.Get();
		}
	}

	bool hasValidAnimation = texture != nullptr;
	if(hasValidAnimation)
	{
		const SpriteSheetGridAnimation& gridAnim = texture->GetAnimation();
		hasValidAnimation = gridAnim.NumRows > 0 && gridAnim.NumColumns > 0 && gridAnim.Count > 0;
	}

	if(!hasValidAnimation)
	{
		for(u32 i = startIdx; i < endIdx; i++)
			particles.Frame[i] = 0.0f;

		return;
	}

	const SpriteSheetGridAnimation& gridAnim = texture->GetAnimation();

	for(u32 i = startIdx; i < endIdx; i++)
	{
		u32 frameOffset;
		u32 numFrames;
		if(mDesc.RandomizeRow)
		{
			const u32 rowSeed = particles.Seed[i] + PARTICLE_ROW_VARIATION;
			const u32 row = Random(rowSeed).GetRange(0, gridAnim.NumRows);

			frameOffset = row * gridAnim.NumColumns;
			numFrames = gridAnim.NumColumns;
		}
		else
		{
			frameOffset = 0;
			numFrames = gridAnim.Count;
		}

		float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];
		particleT = Math::Repeat(mDesc.NumCycles * particleT, 1.0f);

		const float frame = particleT * (numFrames - 1);
		particles.Frame[i] = frameOffset + Math::Clamp(frame, 0.0f, (float)(numFrames - 1));
	}
}

SPtr<ParticleTextureAnimation> ParticleTextureAnimation::Create(const PARTICLE_TEXTURE_ANIMATION_DESC& desc)
{
	return bs_shared_ptr_new<ParticleTextureAnimation>(desc);
}

SPtr<ParticleTextureAnimation> ParticleTextureAnimation::Create()
{
	return bs_shared_ptr_new<ParticleTextureAnimation>();
}

RTTITypeBase* ParticleTextureAnimation::GetRttiStatic()
{
	return ParticleTextureAnimationRTTI::Instance();
}

RTTITypeBase* ParticleTextureAnimation::GetRtti() const
{
	return GetRttiStatic();
}

ParticleOrbit::ParticleOrbit(const PARTICLE_ORBIT_DESC& desc)
	: mDesc(desc)
{}

void ParticleOrbit::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	const Vector3 center = evaluateTransformed(mDesc.Center, state, state.NrmTimeEnd, random, mDesc.WorldSpace);
	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;

	for(u32 i = startIdx; i < endIdx; i++)
	{
		const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

		float timeStep = state.TimeStep;
		if(spacing)
		{
			const u32 localIdx = i - startIdx;
			const float subFrameOffset = ((float)localIdx + spacingOffset) * subFrameSpacing;
			timeStep *= subFrameOffset;
		}

		const u32 velocitySeed = particles.Seed[i] + PARTICLE_ORBIT_VELOCITY;
		Vector3 orbitVelocity = evaluateTransformed<true>(mDesc.Velocity, state, particleT, Random(velocitySeed), mDesc.WorldSpace);
		orbitVelocity *= Math::TWO_PI;

		orbitVelocity *= timeStep;

		const Matrix3 rotation(Radian(orbitVelocity.X), Radian(orbitVelocity.Y), Radian(orbitVelocity.Z));

		const Vector3 point = particles.Position[i] - center;
		const Vector3 newPoint = rotation.Multiply(point);

		Vector3 velocity = newPoint - point;

		const u32 radialSeed = particles.Seed[i] + PARTICLE_ORBIT_RADIAL;
		const float radial = mDesc.Radial.Evaluate(particleT, Random(radialSeed).GetUNorm());
		if(radial != 0.0f)
			velocity += Vector3::Normalize(point) * radial * timeStep;

		particles.Position[i] += velocity;
	}
}

SPtr<ParticleOrbit> ParticleOrbit::Create(const PARTICLE_ORBIT_DESC& desc)
{
	return bs_shared_ptr_new<ParticleOrbit>(desc);
}

SPtr<ParticleOrbit> ParticleOrbit::Create()
{
	return bs_shared_ptr_new<ParticleOrbit>();
}

RTTITypeBase* ParticleOrbit::GetRttiStatic()
{
	return ParticleOrbitRTTI::Instance();
}

RTTITypeBase* ParticleOrbit::GetRtti() const
{
	return GetRttiStatic();
}

ParticleVelocity::ParticleVelocity(const PARTICLE_VELOCITY_DESC& desc)
	: mDesc(desc)
{}

void ParticleVelocity::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
	for(u32 i = startIdx; i < endIdx; i++)
	{
		const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

		float timeStep = state.TimeStep;
		if(spacing)
		{
			const u32 localIdx = i - startIdx;
			const float subFrameOffset = ((float)localIdx + spacingOffset) * subFrameSpacing;
			timeStep *= subFrameOffset;
		}

		const u32 velocitySeed = particles.Seed[i] + PARTICLE_LINEAR_VELOCITY;
		const Vector3 velocity = evaluateTransformed<true>(mDesc.Velocity, state, particleT, Random(velocitySeed), mDesc.WorldSpace) * timeStep;

		particles.Position[i] += velocity;
	}
}

SPtr<ParticleVelocity> ParticleVelocity::Create(const PARTICLE_VELOCITY_DESC& desc)
{
	return bs_shared_ptr_new<ParticleVelocity>(desc);
}

SPtr<ParticleVelocity> ParticleVelocity::Create()
{
	return bs_shared_ptr_new<ParticleVelocity>();
}

RTTITypeBase* ParticleVelocity::GetRttiStatic()
{
	return ParticleVelocityRTTI::Instance();
}

RTTITypeBase* ParticleVelocity::GetRtti() const
{
	return GetRttiStatic();
}

ParticleForce::ParticleForce(const PARTICLE_FORCE_DESC& desc)
	: mDesc(desc)
{}

void ParticleForce::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
	for(u32 i = startIdx; i < endIdx; i++)
	{
		const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

		float timeStep = state.TimeStep;
		if(spacing)
		{
			const u32 localIdx = i - startIdx;
			const float subFrameOffset = ((float)localIdx + spacingOffset) * subFrameSpacing;
			timeStep *= subFrameOffset;
		}

		const u32 forceSeed = particles.Seed[i] + PARTICLE_FORCE;
		const Vector3 force = evaluateTransformed<true>(mDesc.Force, state, particleT, Random(forceSeed), mDesc.WorldSpace) * timeStep;

		particles.Velocity[i] += force * timeStep;
	}
}

SPtr<ParticleForce> ParticleForce::Create(const PARTICLE_FORCE_DESC& desc)
{
	return bs_shared_ptr_new<ParticleForce>(desc);
}

SPtr<ParticleForce> ParticleForce::Create()
{
	return bs_shared_ptr_new<ParticleForce>();
}

RTTITypeBase* ParticleForce::GetRttiStatic()
{
	return ParticleForceRTTI::Instance();
}

RTTITypeBase* ParticleForce::GetRtti() const
{
	return GetRttiStatic();
}

ParticleGravity::ParticleGravity(const PARTICLE_GRAVITY_DESC& desc)
	: mDesc(desc)
{}

void ParticleGravity::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	Vector3 gravity = state.Scene->GetPhysicsScene()->GetGravity() * mDesc.Scale;

	if(!state.WorldSpace)
		gravity = state.WorldToLocal.MultiplyDirection(gravity);

	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	const float subFrameSpacing = (spacing && count > 0) ? 1.0f / count : 1.0f;
	for(u32 i = startIdx; i < endIdx; i++)
	{
		float timeStep = state.TimeStep;
		if(spacing)
		{
			const u32 localIdx = i - startIdx;
			const float subFrameOffset = ((float)localIdx + spacingOffset) * subFrameSpacing;
			timeStep *= subFrameOffset;
		}

		particles.Velocity[i] += gravity * timeStep;
	}
}

SPtr<ParticleGravity> ParticleGravity::Create(const PARTICLE_GRAVITY_DESC& desc)
{
	return bs_shared_ptr_new<ParticleGravity>(desc);
}

SPtr<ParticleGravity> ParticleGravity::Create()
{
	return bs_shared_ptr_new<ParticleGravity>();
}

RTTITypeBase* ParticleGravity::GetRttiStatic()
{
	return ParticleGravityRTTI::Instance();
}

RTTITypeBase* ParticleGravity::GetRtti() const
{
	return GetRttiStatic();
}

ParticleColor::ParticleColor(const PARTICLE_COLOR_DESC& desc)
	: mDesc(desc)
{}

void ParticleColor::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	for(u32 i = startIdx; i < endIdx; i++)
	{
		const u32 colorSeed = particles.Seed[i] + PARTICLE_COLOR;
		const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

		particles.Color[i] = mDesc.Color.Evaluate(particleT, Random(colorSeed));
	}
}

SPtr<ParticleColor> ParticleColor::Create(const PARTICLE_COLOR_DESC& desc)
{
	return bs_shared_ptr_new<ParticleColor>(desc);
}

SPtr<ParticleColor> ParticleColor::Create()
{
	return bs_shared_ptr_new<ParticleColor>();
}

RTTITypeBase* ParticleColor::GetRttiStatic()
{
	return ParticleColorRTTI::Instance();
}

RTTITypeBase* ParticleColor::GetRtti() const
{
	return GetRttiStatic();
}

ParticleSize::ParticleSize(const PARTICLE_SIZE_DESC& desc)
	: mDesc(desc)
{}

void ParticleSize::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	if(!mDesc.Use3DSize)
	{
		for(u32 i = startIdx; i < endIdx; i++)
		{
			const u32 sizeSeed = particles.Seed[i] + PARTICLE_SIZE;
			const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

			const float size = mDesc.Size.Evaluate(particleT, Random(sizeSeed));
			particles.Size[i] = Vector3(size, size, size);
		}
	}
	else
	{
		for(u32 i = startIdx; i < endIdx; i++)
		{
			const u32 sizeSeed = particles.Seed[i] + PARTICLE_SIZE;
			const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

			particles.Size[i] = mDesc.Size3D.Evaluate(particleT, Random(sizeSeed));
		}
	}
}

SPtr<ParticleSize> ParticleSize::Create(const PARTICLE_SIZE_DESC& desc)
{
	return bs_shared_ptr_new<ParticleSize>(desc);
}

SPtr<ParticleSize> ParticleSize::Create()
{
	return bs_shared_ptr_new<ParticleSize>();
}

RTTITypeBase* ParticleSize::GetRttiStatic()
{
	return ParticleSizeRTTI::Instance();
}

RTTITypeBase* ParticleSize::GetRtti() const
{
	return GetRttiStatic();
}

ParticleRotation::ParticleRotation(const PARTICLE_ROTATION_DESC& desc)
	: mDesc(desc)
{}

void ParticleRotation::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	if(!mDesc.Use3DRotation)
	{
		for(u32 i = startIdx; i < endIdx; i++)
		{
			const u32 rotationSeed = particles.Seed[i] + PARTICLE_ROTATION;
			const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

			const float rotation = mDesc.Rotation.Evaluate(particleT, Random(rotationSeed));
			particles.Rotation[i] = Vector3(rotation, 0.0f, 0.0f);
		}
	}
	else
	{
		for(u32 i = startIdx; i < endIdx; i++)
		{
			const u32 rotationSeed = particles.Seed[i] + PARTICLE_ROTATION;
			const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

			particles.Rotation[i] = mDesc.Rotation3D.Evaluate(particleT, Random(rotationSeed));
		}
	}
}

SPtr<ParticleRotation> ParticleRotation::Create(const PARTICLE_ROTATION_DESC& desc)
{
	return bs_shared_ptr_new<ParticleRotation>(desc);
}

SPtr<ParticleRotation> ParticleRotation::Create()
{
	return bs_shared_ptr_new<ParticleRotation>();
}

RTTITypeBase* ParticleRotation::GetRttiStatic()
{
	return ParticleRotationRTTI::Instance();
}

RTTITypeBase* ParticleRotation::GetRtti() const
{
	return GetRttiStatic();
}

/** Information about a particle collision. */
struct ParticleHitInfo
{
	Vector3 Position;
	Vector3 Normal;
	u32 Idx;
};

/** Calculates the new position and velocity after a particle was detected to be colliding. */
void calcCollisionResponse(Vector3& position, Vector3& velocity, const ParticleHitInfo& hitInfo, const PARTICLE_COLLISIONS_DESC& desc)
{
	Vector3 diff = position - hitInfo.Position;

	// Reflect & dampen
	const float dampenFactor = 1.0f - desc.Dampening;

	Vector3 reflectedPos = diff.Reflect(hitInfo.Normal) * dampenFactor;
	Vector3 reflectedVel = velocity.Reflect(hitInfo.Normal) * dampenFactor;

	// Bounce
	const float restitutionFactor = 1.0f - desc.Restitution;

	reflectedPos -= hitInfo.Normal * reflectedPos.Dot(hitInfo.Normal) * restitutionFactor;
	reflectedVel -= hitInfo.Normal * reflectedVel.Dot(hitInfo.Normal) * restitutionFactor;

	position = hitInfo.Position + reflectedPos;
	velocity = reflectedVel;
}

u32 groupRaycast(const PhysicsScene& physicsScene, LineSegment3* segments, ParticleHitInfo* hits, u32 numRays, u64 layer)
{
	if(numRays == 0)
		return 0;

	// Calculate bounds of all rays
	AABox groupBounds = AABox::INF_BOX;
	for(u32 i = 0; i < numRays; i++)
	{
		groupBounds.Merge(segments[i].Start);
		groupBounds.Merge(segments[i].End);
	}

	Vector<Collider*> hitColliders = physicsScene.BoxOverlapInternal(groupBounds, Quaternion::IDENTITY, layer);
	if(hitColliders.empty())
		return 0;

	u32 numHits = 0;
	for(u32 i = 0; i < numRays; i++)
	{
		float nearestHit = std::numeric_limits<float>::max();
		ParticleHitInfo hitInfo;
		hitInfo.Idx = i;

		Vector3 diff = segments[i].End - segments[i].Start;
		const float length = diff.Length();

		if(Math::ApproxEquals(length, 0.0f))
			continue;

		Ray ray;
		ray.SetOrigin(segments[i].Start);
		ray.SetDirection(diff / length);

		for(auto& collider : hitColliders)
		{
			PhysicsQueryHit queryHit;
			if(collider->RayCast(ray, queryHit, length))
			{
				if(queryHit.Distance < nearestHit)
				{
					nearestHit = queryHit.Distance;

					hitInfo.Position = queryHit.Point;
					hitInfo.Normal = queryHit.Normal;
				}
			}
		}

		if(nearestHit != std::numeric_limits<float>::max())
			hits[numHits++] = hitInfo;
	}

	return numHits;
}

ParticleCollisions::ParticleCollisions(const PARTICLE_COLLISIONS_DESC& desc)
	: mDesc(desc)
{
	mDesc.Restitution = std::max(mDesc.Restitution, 0.0f);
	mDesc.Dampening = Math::Clamp01(mDesc.Dampening);
	mDesc.LifetimeLoss = Math::Clamp01(mDesc.LifetimeLoss);
	mDesc.Radius = std::max(mDesc.Radius, 0.0f);
}

void ParticleCollisions::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	if(mDesc.Mode == ParticleCollisionMode::Plane)
	{
		u32 numPlanes[2] = { 0, 0 };
		Plane* planes[2];

		// Extract planes from scene objects
		Plane* objPlanes = nullptr;

		if(!mCollisionPlaneObjects.empty())
		{
			objPlanes = bs_stack_alloc<Plane>((u32)mCollisionPlaneObjects.size());
			for(auto& entry : mCollisionPlaneObjects)
			{
				if(entry.IsDestroyed())
					continue;

				const Transform& tfrm = entry->GetTransform();
				Plane plane = Plane(tfrm.GetForward(), tfrm.GetPosition());

				if(!state.WorldSpace)
					plane = state.WorldToLocal.MultiplyAffine(plane);

				objPlanes[numPlanes[0]++] = plane;
			}
		}

		planes[0] = objPlanes;

		// If particles are in world space, we can just use collision planes as is
		Plane* localPlanes = nullptr;
		if(state.WorldSpace)
			planes[1] = (Plane*)mCollisionPlanes.data();
		else
		{
			const Matrix4& worldToLocal = state.WorldToLocal;
			localPlanes = bs_stack_alloc<Plane>((u32)mCollisionPlanes.size());

			for(u32 i = 0; i < (u32)mCollisionPlanes.size(); i++)
				localPlanes[i] = worldToLocal.MultiplyAffine(mCollisionPlanes[i]);

			planes[1] = localPlanes;
		}

		numPlanes[1] = (u32)mCollisionPlanes.size();

		for(u32 i = startIdx; i < endIdx; i++)
		{
			Vector3& position = particles.Position[i];
			Vector3& velocity = particles.Velocity[i];

			for(u32 j = 0; j < bs_size(planes); j++)
			{
				for(u32 k = 0; k < numPlanes[j]; k++)
				{
					const Plane& plane = planes[j][k];

					const float dist = plane.GetDistance(position);
					if(dist > mDesc.Radius)
						continue;

					const float distToTravelAlongNormal = plane.Normal.Dot(velocity);

					// Ignore movement parallel to the plane
					if(Math::ApproxEquals(distToTravelAlongNormal, 0.0f))
						continue;

					const float distFromBoundary = mDesc.Radius - dist;
					const float rayT = distFromBoundary / distToTravelAlongNormal;

					ParticleHitInfo hitInfo;
					hitInfo.Normal = plane.Normal;
					hitInfo.Position = position + velocity * rayT;
					hitInfo.Idx = i;

					calcCollisionResponse(position, velocity, hitInfo, mDesc);
					particles.Lifetime[i] -= mDesc.LifetimeLoss * particles.InitialLifetime[i];

					break;
				}
			}
		}

		if(objPlanes)
			bs_stack_free(objPlanes);

		if(localPlanes)
			bs_stack_free(localPlanes);
	}
	else
	{
		const u32 rayStart = startIdx;
		const u32 rayEnd = endIdx;
		const u32 numRays = rayEnd - rayStart;

		const auto segments = bs_stack_alloc<LineSegment3>(numRays);
		const auto hits = bs_stack_alloc<ParticleHitInfo>(numRays);

		for(u32 i = 0; i < numRays; i++)
		{
			const Vector3& prevPosition = particles.PrevPosition[rayStart + i];
			const Vector3& position = particles.Position[rayStart + i];

			segments[i] = LineSegment3(prevPosition, position);
		}

		if(!state.WorldSpace)
		{
			for(u32 i = 0; i < numRays; i++)
			{
				segments[i].Start = state.LocalToWorld.MultiplyAffine(segments[i].Start);
				segments[i].End = state.LocalToWorld.MultiplyAffine(segments[i].End);
			}
		}

		const PhysicsScene& physicsScene = *state.Scene->GetPhysicsScene();
		const u32 numHits = groupRaycast(physicsScene, segments, hits, numRays, mDesc.Layer);

		if(!state.WorldSpace)
		{
			for(u32 i = 0; i < numHits; i++)
			{
				hits[i].Position = state.WorldToLocal.MultiplyAffine(hits[i].Position);
				hits[i].Normal = state.WorldToLocal.MultiplyDirection(hits[i].Normal);
			}
		}

		for(u32 i = 0; i < numHits; i++)
		{
			ParticleHitInfo& hitInfo = hits[i];
			const u32 particleIdx = rayStart + hitInfo.Idx;

			Vector3& position = particles.Position[particleIdx];
			Vector3& velocity = particles.Velocity[particleIdx];

			calcCollisionResponse(position, velocity, hitInfo, mDesc);

			particles.Lifetime[particleIdx] -= mDesc.LifetimeLoss * particles.InitialLifetime[particleIdx];
		}

		bs_stack_free(hits);
		bs_stack_free(segments);
	}
}

SPtr<ParticleCollisions> ParticleCollisions::Create(const PARTICLE_COLLISIONS_DESC& desc)
{
	return bs_shared_ptr_new<ParticleCollisions>(desc);
}

SPtr<ParticleCollisions> ParticleCollisions::Create()
{
	return bs_shared_ptr_new<ParticleCollisions>();
}

RTTITypeBase* ParticleCollisions::GetRttiStatic()
{
	return ParticleCollisionsRTTI::Instance();
}

RTTITypeBase* ParticleCollisions::GetRtti() const
{
	return GetRttiStatic();
}
