//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Particles/B3DParticleEvolver.h"
#include "Private/Particles/B3DParticleSet.h"
#include "RTTI/B3DParticleSystemRTTI.h"
#include "Particles/B3DVectorField.h"
#include "Image/B3DSpriteTexture.h"
#include "Components/B3DCollider.h"
#include "Material/B3DMaterial.h"
#include "Math/B3DRay.h"
#include "Physics/B3DPhysics.h"
#include "Math/B3DLineSegment3.h"
#include "Material/B3DShader.h"
#include "Scene/B3DSceneObject.h"
#include "Scene/B3DSceneInstance.h"

using namespace b3d;

// Arbitrary random numbers to add variation to different random particle properties, since we use just a single
// seed value per particle
static constexpr u32 kParticleRowVariation = 0x1e8b2f4a;
static constexpr u32 kParticleOrbitVelocity = 0x24c00a5b;
static constexpr u32 kParticleOrbitRadial = 0x35978d21;
static constexpr u32 kParticleLinearVelocity = 0x0a299430;
static constexpr u32 kParticleForce = 0x1b618144;
static constexpr u32 kParticleColor = 0x378578b2;
static constexpr u32 kParticleSize = 0x91088409;
static constexpr u32 kParticleRotation = 0x4680eaa4;

/** Helper method that applies a transform to either a point or a direction. */
template <bool dir>
Vector3 ApplyTransform(const Matrix4& tfrm, const Vector3& input)
{
	return tfrm.MultiplyAffine(input);
}

template <>
Vector3 ApplyTransform<true>(const Matrix4& tfrm, const Vector3& input)
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
Vector3 EvaluateTransformed(const Vector3Distribution& distribution, const ParticleSystemState& state, float t, const Random& factor, bool inWorldSpace)
{
	const Vector3 output = distribution.Evaluate(t, factor);

	if(state.WorldSpace == inWorldSpace)
		return output;

	if(state.WorldSpace)
		return ApplyTransform<dir>(state.LocalToWorld, output);
	else
		return ApplyTransform<dir>(state.WorldToLocal, output);
}

ParticleTextureAnimation::ParticleTextureAnimation(const ParticleTextureAnimationSettings& settings)
	: mSettings(settings)
{}

void ParticleTextureAnimation::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	SpriteImage* image = nullptr;
	const HMaterial& material = state.System->GetSettings().Material;
	if(material.IsLoaded(false))
	{
		const HShader& shader = material->GetShader();
		if(shader->HasTextureParam("gTexture"))
		{
			const HSpriteImage& spriteImage = material->GetSpriteImage("gTexture");
			if(spriteImage.IsLoaded(true))
				image = spriteImage.Get();
		}

		if(shader->HasTextureParam("gAlbedoTex"))
		{
			const HSpriteImage& spriteImage = material->GetSpriteImage("gAlbedoTex");
			if(spriteImage.IsLoaded(true))
				image = spriteImage.Get();
		}
	}

	bool hasValidAnimation = image != nullptr;
	if(hasValidAnimation)
	{
		const SpriteSheetGridAnimation& gridAnim = image->GetAnimation();
		hasValidAnimation = gridAnim.RowCount > 0 && gridAnim.ColumnCount > 0 && gridAnim.FrameCount > 0;
	}

	if(!hasValidAnimation)
	{
		for(u32 i = startIdx; i < endIdx; i++)
			particles.Frame[i] = 0.0f;

		return;
	}

	const SpriteSheetGridAnimation& gridAnim = image->GetAnimation();

	for(u32 i = startIdx; i < endIdx; i++)
	{
		u32 frameOffset;
		u32 numFrames;
		if(mSettings.RandomizeRow)
		{
			const u32 rowSeed = particles.Seed[i] + kParticleRowVariation;
			const u32 row = Random(rowSeed).GetRange(0, gridAnim.RowCount);

			frameOffset = row * gridAnim.ColumnCount;
			numFrames = gridAnim.ColumnCount;
		}
		else
		{
			frameOffset = 0;
			numFrames = gridAnim.FrameCount;
		}

		float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];
		particleT = Math::Repeat(mSettings.CycleCount * particleT, 1.0f);

		const float frame = particleT * (numFrames - 1);
		particles.Frame[i] = frameOffset + Math::Clamp(frame, 0.0f, (float)(numFrames - 1));
	}
}

SPtr<ParticleTextureAnimation> ParticleTextureAnimation::Create(const ParticleTextureAnimationSettings& settings)
{
	return B3DMakeShared<ParticleTextureAnimation>(settings);
}

SPtr<ParticleTextureAnimation> ParticleTextureAnimation::Create()
{
	return B3DMakeShared<ParticleTextureAnimation>();
}

RTTIType* ParticleTextureAnimation::GetRttiStatic()
{
	return ParticleTextureAnimationRTTI::Instance();
}

RTTIType* ParticleTextureAnimation::GetRtti() const
{
	return GetRttiStatic();
}

ParticleOrbit::ParticleOrbit(const ParticleOrbitSettings& settings)
	: mSettings(settings)
{}

void ParticleOrbit::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	const Vector3 center = EvaluateTransformed(mSettings.Center, state, state.NrmTimeEnd, random, mSettings.WorldSpace);
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

		const u32 velocitySeed = particles.Seed[i] + kParticleOrbitVelocity;
		Vector3 orbitVelocity = EvaluateTransformed<true>(mSettings.Velocity, state, particleT, Random(velocitySeed), mSettings.WorldSpace);
		orbitVelocity *= Math::kTwoPi;

		orbitVelocity *= timeStep;

		const Matrix3 rotation(Radian(orbitVelocity.X), Radian(orbitVelocity.Y), Radian(orbitVelocity.Z));

		const Vector3 point = particles.Position[i] - center;
		const Vector3 newPoint = rotation.Multiply(point);

		Vector3 velocity = newPoint - point;

		const u32 radialSeed = particles.Seed[i] + kParticleOrbitRadial;
		const float radial = mSettings.Radial.Evaluate(particleT, Random(radialSeed).GetUNorm());
		if(radial != 0.0f)
			velocity += Vector3::Normalize(point) * radial * timeStep;

		particles.Position[i] += velocity;
	}
}

SPtr<ParticleOrbit> ParticleOrbit::Create(const ParticleOrbitSettings& settings)
{
	return B3DMakeShared<ParticleOrbit>(settings);
}

SPtr<ParticleOrbit> ParticleOrbit::Create()
{
	return B3DMakeShared<ParticleOrbit>();
}

RTTIType* ParticleOrbit::GetRttiStatic()
{
	return ParticleOrbitRTTI::Instance();
}

RTTIType* ParticleOrbit::GetRtti() const
{
	return GetRttiStatic();
}

ParticleVelocity::ParticleVelocity(const ParticleVelocitySettings& settings)
	: mSettings(settings)
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

		const u32 velocitySeed = particles.Seed[i] + kParticleLinearVelocity;
		const Vector3 velocity = EvaluateTransformed<true>(mSettings.Velocity, state, particleT, Random(velocitySeed), mSettings.WorldSpace) * timeStep;

		particles.Position[i] += velocity;
	}
}

SPtr<ParticleVelocity> ParticleVelocity::Create(const ParticleVelocitySettings& settings)
{
	return B3DMakeShared<ParticleVelocity>(settings);
}

SPtr<ParticleVelocity> ParticleVelocity::Create()
{
	return B3DMakeShared<ParticleVelocity>();
}

RTTIType* ParticleVelocity::GetRttiStatic()
{
	return ParticleVelocityRTTI::Instance();
}

RTTIType* ParticleVelocity::GetRtti() const
{
	return GetRttiStatic();
}

ParticleForce::ParticleForce(const ParticleForceSettings& settings)
	: mSettings(settings)
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

		const u32 forceSeed = particles.Seed[i] + kParticleForce;
		const Vector3 force = EvaluateTransformed<true>(mSettings.Force, state, particleT, Random(forceSeed), mSettings.WorldSpace) * timeStep;

		particles.Velocity[i] += force * timeStep;
	}
}

SPtr<ParticleForce> ParticleForce::Create(const ParticleForceSettings& settings)
{
	return B3DMakeShared<ParticleForce>(settings);
}

SPtr<ParticleForce> ParticleForce::Create()
{
	return B3DMakeShared<ParticleForce>();
}

RTTIType* ParticleForce::GetRttiStatic()
{
	return ParticleForceRTTI::Instance();
}

RTTIType* ParticleForce::GetRtti() const
{
	return GetRttiStatic();
}

ParticleGravity::ParticleGravity(const ParticleGravitySettings& settings)
	: mSettings(settings)
{}

void ParticleGravity::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	Vector3 gravity = state.Scene->GetPhysicsScene()->GetGravity() * mSettings.Scale;

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

SPtr<ParticleGravity> ParticleGravity::Create(const ParticleGravitySettings& settings)
{
	return B3DMakeShared<ParticleGravity>(settings);
}

SPtr<ParticleGravity> ParticleGravity::Create()
{
	return B3DMakeShared<ParticleGravity>();
}

RTTIType* ParticleGravity::GetRttiStatic()
{
	return ParticleGravityRTTI::Instance();
}

RTTIType* ParticleGravity::GetRtti() const
{
	return GetRttiStatic();
}

ParticleColor::ParticleColor(const ParticleColorSettings& settings)
	: mSettings(settings)
{}

void ParticleColor::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	for(u32 i = startIdx; i < endIdx; i++)
	{
		const u32 colorSeed = particles.Seed[i] + kParticleColor;
		const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

		particles.Color[i] = mSettings.Color.Evaluate(particleT, Random(colorSeed));
	}
}

SPtr<ParticleColor> ParticleColor::Create(const ParticleColorSettings& settings)
{
	return B3DMakeShared<ParticleColor>(settings);
}

SPtr<ParticleColor> ParticleColor::Create()
{
	return B3DMakeShared<ParticleColor>();
}

RTTIType* ParticleColor::GetRttiStatic()
{
	return ParticleColorRTTI::Instance();
}

RTTIType* ParticleColor::GetRtti() const
{
	return GetRttiStatic();
}

ParticleSize::ParticleSize(const ParticleSizeSettings& settings)
	: mSettings(settings)
{}

void ParticleSize::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	if(!mSettings.Use3DSize)
	{
		for(u32 i = startIdx; i < endIdx; i++)
		{
			const u32 sizeSeed = particles.Seed[i] + kParticleSize;
			const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

			const float size = mSettings.Size.Evaluate(particleT, Random(sizeSeed));
			particles.Size[i] = Vector3(size, size, size);
		}
	}
	else
	{
		for(u32 i = startIdx; i < endIdx; i++)
		{
			const u32 sizeSeed = particles.Seed[i] + kParticleSize;
			const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

			particles.Size[i] = mSettings.Size3D.Evaluate(particleT, Random(sizeSeed));
		}
	}
}

SPtr<ParticleSize> ParticleSize::Create(const ParticleSizeSettings& settings)
{
	return B3DMakeShared<ParticleSize>(settings);
}

SPtr<ParticleSize> ParticleSize::Create()
{
	return B3DMakeShared<ParticleSize>();
}

RTTIType* ParticleSize::GetRttiStatic()
{
	return ParticleSizeRTTI::Instance();
}

RTTIType* ParticleSize::GetRtti() const
{
	return GetRttiStatic();
}

ParticleRotation::ParticleRotation(const ParticleRotationSettings& settings)
	: mSettings(settings)
{}

void ParticleRotation::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	if(!mSettings.Use3DRotation)
	{
		for(u32 i = startIdx; i < endIdx; i++)
		{
			const u32 rotationSeed = particles.Seed[i] + kParticleRotation;
			const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

			const float rotation = mSettings.Rotation.Evaluate(particleT, Random(rotationSeed));
			particles.Rotation[i] = Vector3(rotation, 0.0f, 0.0f);
		}
	}
	else
	{
		for(u32 i = startIdx; i < endIdx; i++)
		{
			const u32 rotationSeed = particles.Seed[i] + kParticleRotation;
			const float particleT = (particles.InitialLifetime[i] - particles.Lifetime[i]) / particles.InitialLifetime[i];

			particles.Rotation[i] = mSettings.Rotation3D.Evaluate(particleT, Random(rotationSeed));
		}
	}
}

SPtr<ParticleRotation> ParticleRotation::Create(const ParticleRotationSettings& settings)
{
	return B3DMakeShared<ParticleRotation>(settings);
}

SPtr<ParticleRotation> ParticleRotation::Create()
{
	return B3DMakeShared<ParticleRotation>();
}

RTTIType* ParticleRotation::GetRttiStatic()
{
	return ParticleRotationRTTI::Instance();
}

RTTIType* ParticleRotation::GetRtti() const
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
void CalcCollisionResponse(Vector3& position, Vector3& velocity, const ParticleHitInfo& hitInfo, const ParticleCollisionSettings& settings)
{
	Vector3 diff = position - hitInfo.Position;

	// Reflect & dampen
	const float dampenFactor = 1.0f - settings.Dampening;

	Vector3 reflectedPos = diff.Reflect(hitInfo.Normal) * dampenFactor;
	Vector3 reflectedVel = velocity.Reflect(hitInfo.Normal) * dampenFactor;

	// Bounce
	const float restitutionFactor = 1.0f - settings.Restitution;

	reflectedPos -= hitInfo.Normal * reflectedPos.Dot(hitInfo.Normal) * restitutionFactor;
	reflectedVel -= hitInfo.Normal * reflectedVel.Dot(hitInfo.Normal) * restitutionFactor;

	position = hitInfo.Position + reflectedPos;
	velocity = reflectedVel;
}

u32 GroupRaycast(const PhysicsScene& physicsScene, LineSegment3* segments, ParticleHitInfo* hits, u32 numRays, u64 layer)
{
	if(numRays == 0)
		return 0;

	// Calculate bounds of all rays
	AABox groupBounds = AABox::kInfinite;
	for(u32 i = 0; i < numRays; i++)
	{
		groupBounds.Merge(segments[i].Start);
		groupBounds.Merge(segments[i].End);
	}

	Vector<ColliderShape*> hitColliderShapes = physicsScene.BoxOverlapInternal(groupBounds, Quaternion::kIdentity, layer);
	if(hitColliderShapes.empty())
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
		ray.Origin = segments[i].Start;
		ray.Direction = diff / length;

		for(auto& colliderShape : hitColliderShapes)
		{
			Collider* const collider = colliderShape->GetParentCollider();

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

ParticleCollisions::ParticleCollisions(const ParticleCollisionSettings& settings)
	: mSettings(settings)
{
	mSettings.Restitution = std::max(mSettings.Restitution, 0.0f);
	mSettings.Dampening = Math::Clamp01(mSettings.Dampening);
	mSettings.LifetimeLoss = Math::Clamp01(mSettings.LifetimeLoss);
	mSettings.Radius = std::max(mSettings.Radius, 0.0f);
}

void ParticleCollisions::Evolve(Random& random, const ParticleSystemState& state, ParticleSet& set, u32 startIdx, u32 count, bool spacing, float spacingOffset) const
{
	const u32 endIdx = startIdx + count;
	ParticleSetData& particles = set.GetParticles();

	if(mSettings.Mode == ParticleCollisionMode::Plane)
	{
		u32 numPlanes[2] = { 0, 0 };
		Plane* planes[2];

		// Extract planes from scene objects
		Plane* objPlanes = nullptr;

		if(!mCollisionPlaneObjects.empty())
		{
			objPlanes = B3DStackAllocate<Plane>((u32)mCollisionPlaneObjects.size());
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
			localPlanes = B3DStackAllocate<Plane>((u32)mCollisionPlanes.size());

			for(u32 i = 0; i < (u32)mCollisionPlanes.size(); i++)
				localPlanes[i] = worldToLocal.MultiplyAffine(mCollisionPlanes[i]);

			planes[1] = localPlanes;
		}

		numPlanes[1] = (u32)mCollisionPlanes.size();

		for(u32 i = startIdx; i < endIdx; i++)
		{
			Vector3& position = particles.Position[i];
			Vector3& velocity = particles.Velocity[i];

			for(u32 j = 0; j < B3DSize(planes); j++)
			{
				for(u32 k = 0; k < numPlanes[j]; k++)
				{
					const Plane& plane = planes[j][k];

					const float dist = plane.GetDistance(position);
					if(dist > mSettings.Radius)
						continue;

					const float distToTravelAlongNormal = plane.Normal.Dot(velocity);

					// Ignore movement parallel to the plane
					if(Math::ApproxEquals(distToTravelAlongNormal, 0.0f))
						continue;

					const float distFromBoundary = mSettings.Radius - dist;
					const float rayT = distFromBoundary / distToTravelAlongNormal;

					ParticleHitInfo hitInfo;
					hitInfo.Normal = plane.Normal;
					hitInfo.Position = position + velocity * rayT;
					hitInfo.Idx = i;

					CalcCollisionResponse(position, velocity, hitInfo, mSettings);
					particles.Lifetime[i] -= mSettings.LifetimeLoss * particles.InitialLifetime[i];

					break;
				}
			}
		}

		if(objPlanes)
			B3DStackFree(objPlanes);

		if(localPlanes)
			B3DStackFree(localPlanes);
	}
	else
	{
		const u32 rayStart = startIdx;
		const u32 rayEnd = endIdx;
		const u32 numRays = rayEnd - rayStart;

		const auto segments = B3DStackAllocate<LineSegment3>(numRays);
		const auto hits = B3DStackAllocate<ParticleHitInfo>(numRays);

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
		const u32 numHits = GroupRaycast(physicsScene, segments, hits, numRays, mSettings.Layer);

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

			CalcCollisionResponse(position, velocity, hitInfo, mSettings);

			particles.Lifetime[particleIdx] -= mSettings.LifetimeLoss * particles.InitialLifetime[particleIdx];
		}

		B3DStackFree(hits);
		B3DStackFree(segments);
	}
}

SPtr<ParticleCollisions> ParticleCollisions::Create(const ParticleCollisionSettings& settings)
{
	return B3DMakeShared<ParticleCollisions>(settings);
}

SPtr<ParticleCollisions> ParticleCollisions::Create()
{
	return B3DMakeShared<ParticleCollisions>();
}

RTTIType* ParticleCollisions::GetRttiStatic()
{
	return ParticleCollisionsRTTI::Instance();
}

RTTIType* ParticleCollisions::GetRtti() const
{
	return GetRttiStatic();
}
