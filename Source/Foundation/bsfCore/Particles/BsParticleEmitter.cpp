//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsParticleEmitter.h"
#include "Mesh/BsMeshData.h"
#include "Mesh/BsMeshUtility.h"
#include "RenderAPI/BsVertexDescription.h"
#include "Math/BsRandom.h"
#include "Components/BsCRenderable.h"
#include "Private/Particles/BsParticleSet.h"
#include "Private/RTTI/BsParticleSystemRTTI.h"
#include "Animation/BsAnimation.h"
#include "Animation/BsAnimationManager.h"
#include "Mesh/BsMesh.h"

using namespace bs;

MeshWeightedTriangles::MeshWeightedTriangles(const MeshData& meshData)
{
	Calculate(meshData);
}

void MeshWeightedTriangles::Calculate(const MeshData& meshData)
{
	const u32 numIndices = meshData.GetIndexCount();
	B3D_ASSERT(numIndices % 3 == 0);

	const u32 numTriangles = numIndices / 3;
	mWeights.resize(numTriangles);

	u8* vertices = meshData.GetElementData(VES_POSITION);

	const SPtr<VertexDescription>& vertexDesc = meshData.GetVertexDescription();
	const u32 stride = vertexDesc->GetVertexStride();

	float totalArea = 0.0f;
	if(meshData.GetIndexType() == IT_32BIT)
	{
		u32* indices = meshData.GetIndices32();

		for(u32 i = 0; i < numTriangles; i++)
		{
			TriangleWeight& weight = mWeights[i];

			weight.Indices[0] = indices[i * 3 + 0];
			weight.Indices[1] = indices[i * 3 + 1];
			weight.Indices[2] = indices[i * 3 + 2];
		}
	}
	else
	{
		u16* indices = meshData.GetIndices16();

		for(u32 i = 0; i < numTriangles; i++)
		{
			TriangleWeight& weight = mWeights[i];

			weight.Indices[0] = indices[i * 3 + 0];
			weight.Indices[1] = indices[i * 3 + 1];
			weight.Indices[2] = indices[i * 3 + 2];
		}
	}

	for(u32 i = 0; i < numTriangles; i++)
	{
		TriangleWeight& weight = mWeights[i];
		const Vector3& a = *(Vector3*)(vertices + weight.Indices[0] * stride);
		const Vector3& b = *(Vector3*)(vertices + weight.Indices[1] * stride);
		const Vector3& c = *(Vector3*)(vertices + weight.Indices[2] * stride);

		// Note: Using squared length here would be faster, but the weights can be small and squaring them just
		// makes them smaller, causing precision issues
		weight.CumulativeWeight = Vector3::Cross(b - a, c - a).Length();
		totalArea += weight.CumulativeWeight;
	}

	const float invTotalArea = 1.0f / totalArea;
	for(u32 i = 0; i < numTriangles; i++)
		mWeights[i].CumulativeWeight *= invTotalArea;

	for(u32 i = 1; i < numTriangles; i++)
		mWeights[i].CumulativeWeight += mWeights[i - 1].CumulativeWeight;

	mWeights[numTriangles - 1].CumulativeWeight = 1.0f;
}

void MeshWeightedTriangles::GetTriangle(const Random& random, std::array<u32, 3>& indices) const
{
	struct Comp
	{
		bool operator()(float a, const TriangleWeight& b) const
		{
			return a < b.CumulativeWeight;
		}

		bool operator()(const TriangleWeight& a, float b) const
		{
			return a.CumulativeWeight < b;
		}
	};

	const float val = random.GetUNorm();

	const auto findIter = std::lower_bound(mWeights.begin(), mWeights.end(), val, Comp());
	if(findIter != mWeights.end())
		memcpy(indices.data(), findIter->Indices, sizeof(indices));
	else
		B3DZeroOut(indices);
}

template <class Pr>
u32 SpawnMultiple(ParticleSet& particles, u32 count, Pr predicate)
{
	const u32 index = particles.AllocParticles(count);
	ParticleSetData& particleData = particles.GetParticles();

	const u32 end = index + count;
	for(u32 i = index; i < end; i++)
		predicate(i - index, particleData.Position[i], particleData.Velocity[i]);

	return index;
}

template <class T>
u32 SpawnMultipleRandom(T* spawner, const Random& random, ParticleSet& particles, u32 count)
{
	const u32 index = particles.AllocParticles(count);
	ParticleSetData& particleData = particles.GetParticles();

	const u32 end = index + count;
	for(u32 i = index; i < end; i++)
		spawner->SpawnInternal(random, particleData.Position[i], particleData.Velocity[i]);

	return index;
}

template <class T>
u32 SpawnMultipleSpread(T* spawner, float length, float interval, ParticleSet& particles, u32 count)
{
	const u32 index = particles.AllocParticles(count);
	ParticleSetData& particleData = particles.GetParticles();

	const float dt = length / (float)count;

	float accum = 0.0f;
	for(u32 i = 0; i < count; i++)
	{
		float t = accum;
		if(interval > 0)
			t = Math::RoundToMultiple(accum, interval);

		const u32 particleIdx = index + i;
		spawner->SpawnInternal(t, particleData.Position[particleIdx], particleData.Velocity[particleIdx]);

		accum += dt;
	}

	return index;
}

template <class T>
u32 SpawnMultipleLoop(T* spawner, float length, float speed, float interval, ParticleSet& particles, u32 count, const ParticleSystemState& state)
{
	const u32 index = particles.AllocParticles(count);
	ParticleSetData& particleData = particles.GetParticles();

	const float dt = state.TimeStep / (float)count;

	for(u32 i = 0; i < count; i++)
	{
		float t = (state.TimeStart + dt * i) * speed;
		t = fmod(t, length);

		if(interval > 0.0f)
			t = Math::RoundToMultiple(t, interval);

		const u32 particleIdx = index + i;
		spawner->SpawnInternal(t, particleData.Position[particleIdx], particleData.Velocity[particleIdx]);
	}

	return index;
}

template <class T>
u32 SpawnMultiplePingPong(T* spawner, float length, float speed, float interval, ParticleSet& particles, u32 count, const ParticleSystemState& state)
{
	const u32 index = particles.AllocParticles(count);
	ParticleSetData& particleData = particles.GetParticles();

	const float dt = state.TimeStep / (float)count;

	for(u32 i = 0; i < count; i++)
	{
		float t = (state.TimeStart + dt * i) * speed;

		const auto loop = (u32)(t / length);
		if(loop % 2 == 1)
			t = length - fmod(t, length);
		else
			t = fmod(t, length);

		if(interval > 0.0f)
			t = Math::RoundToMultiple(t, interval);

		const u32 particleIdx = index + i;
		spawner->SpawnInternal(t, particleData.Position[particleIdx], particleData.Velocity[particleIdx]);
	}

	return index;
}

template <class T>
u32 SpawnMultipleMode(T* spawner, ParticleEmissionModeType type, float length, float speed, float interval, const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state)
{
	if(count > 0)
	{
		switch(type)
		{
		case ParticleEmissionModeType::Random:
			return SpawnMultipleRandom(spawner, random, particles, count);
		case ParticleEmissionModeType::Loop:
			return SpawnMultipleLoop(spawner, length, speed, interval, particles, count, state);
		case ParticleEmissionModeType::PingPong:
			return SpawnMultiplePingPong(spawner, length, speed, interval, particles, count, state);
		case ParticleEmissionModeType::Spread:
			return SpawnMultipleSpread(spawner, length, interval, particles, count);
		default:
			break;
		}
	}

	return particles.GetParticleCount();
}

ParticleEmitterConeShape::ParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc)
	: mInfo(desc)
{}

u32 ParticleEmitterConeShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleMode(this, mInfo.Mode.Type, mInfo.Arc.GetValueInRadians(), mInfo.Mode.Speed * Math::kDeG2Rad, mInfo.Mode.Interval * Math::kDeG2Rad, random, particles, count, state);
}

void ParticleEmitterConeShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	Vector2 pos2D;
	if(Math::ApproxEquals(mInfo.Arc.GetValueInRadians(), 360.0f))
		pos2D = random.GetPointInCircleShell(mInfo.Thickness);
	else
		pos2D = random.GetPointInArcShell(mInfo.Arc, mInfo.Thickness);

	GetPointInCone(pos2D, random.GetUNorm() * mInfo.Length, position, normal);
}

void ParticleEmitterConeShape::SpawnInternal(float t, Vector3& position, Vector3& normal) const
{
	const Vector2 pos2D(Math::Cos(t), Math::Sin(t));

	GetPointInCone(pos2D, 0.0f, position, normal);
}

void ParticleEmitterConeShape::GetPointInCone(const Vector2& pos2D, float distance, Vector3& position, Vector3& normal) const
{
	const float angleSin = Math::Sin(mInfo.Angle);
	normal = Vector3(pos2D.X * angleSin, pos2D.Y * angleSin, Math::Cos(mInfo.Angle));
	normal.Normalize();

	position = Vector3(pos2D.X * mInfo.Radius, pos2D.Y * mInfo.Radius, 0.0f);

	if(mInfo.Type == ParticleEmitterConeType::Volume)
		position += normal * distance;
}

SPtr<ParticleEmitterConeShape> ParticleEmitterConeShape::Create(const PARTICLE_CONE_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterConeShape>(desc);
}

SPtr<ParticleEmitterConeShape> ParticleEmitterConeShape::Create()
{
	return B3DMakeShared<ParticleEmitterConeShape>();
}

void ParticleEmitterConeShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	const float sinAngle = Math::Sin(mInfo.Angle);
	const float cosAngle = Math::Cos(mInfo.Angle);

	if(mInfo.Type == ParticleEmitterConeType::Base)
	{
		shape.SetMin(Vector3(-mInfo.Radius, -mInfo.Radius, 0.0f));
		shape.SetMax(Vector3(mInfo.Radius, mInfo.Radius, 0.0f));
	}
	else
	{
		const float topRadius = mInfo.Radius + mInfo.Length * sinAngle;
		const float length = mInfo.Length * cosAngle;

		shape.SetMin(Vector3(-topRadius, -topRadius, 0.0f));
		shape.SetMax(Vector3(topRadius, topRadius, length));
	}

	velocity.SetMin(Vector3(-sinAngle, -sinAngle, 0.0f));
	velocity.SetMax(Vector3(sinAngle, sinAngle, 1.0f));
}

RTTITypeBase* ParticleEmitterConeShape::GetRttiStatic()
{
	return ParticleEmitterConeShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterConeShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterSphereShape::ParticleEmitterSphereShape(const PARTICLE_SPHERE_SHAPE_DESC& desc)
	: mInfo(desc)
{}

u32 ParticleEmitterSphereShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleRandom(this, random, particles, count);
}

void ParticleEmitterSphereShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	position = random.GetPointInSphereShell(mInfo.Thickness);
	normal = Vector3::Normalize(position);

	position *= mInfo.Radius;
}

void ParticleEmitterSphereShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.SetMin(Vector3::kOne * -mInfo.Radius);
	shape.SetMax(Vector3::kOne * mInfo.Radius);

	velocity.SetMin(-Vector3::kOne);
	velocity.SetMax(Vector3::kOne);
}

SPtr<ParticleEmitterSphereShape> ParticleEmitterSphereShape::Create(const PARTICLE_SPHERE_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterSphereShape>(desc);
}

SPtr<ParticleEmitterSphereShape> ParticleEmitterSphereShape::Create()
{
	return B3DMakeShared<ParticleEmitterSphereShape>();
}

RTTITypeBase* ParticleEmitterSphereShape::GetRttiStatic()
{
	return ParticleEmitterSphereShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterSphereShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterHemisphereShape::ParticleEmitterHemisphereShape(const PARTICLE_HEMISPHERE_SHAPE_DESC& desc)
	: mInfo(desc)
{}

u32 ParticleEmitterHemisphereShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleRandom(this, random, particles, count);
}

void ParticleEmitterHemisphereShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	position = random.GetPointInSphereShell(mInfo.Thickness);
	if(position.Z > 0.0f)
		position.Z *= -1.0f;

	normal = Vector3::Normalize(position);
	position *= mInfo.Radius;
}

void ParticleEmitterHemisphereShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.SetMin(Vector3(-mInfo.Radius, -mInfo.Radius, 0.0f));
	shape.SetMax(Vector3::kOne * mInfo.Radius);

	velocity.SetMin(Vector3(-1.0f, -1.0f, 0.0f));
	velocity.SetMax(Vector3::kOne);
}

SPtr<ParticleEmitterHemisphereShape> ParticleEmitterHemisphereShape::Create(const PARTICLE_HEMISPHERE_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterHemisphereShape>(desc);
}

SPtr<ParticleEmitterHemisphereShape> ParticleEmitterHemisphereShape::Create()
{
	return B3DMakeShared<ParticleEmitterHemisphereShape>();
}

RTTITypeBase* ParticleEmitterHemisphereShape::GetRttiStatic()
{
	return ParticleEmitterHemisphereShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterHemisphereShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterBoxShape::ParticleEmitterBoxShape(const PARTICLE_BOX_SHAPE_DESC& desc)
	: mInfo(desc)
{
	switch(mInfo.Type)
	{
	case ParticleEmitterBoxType::Surface:
		{
			float totalSurfaceArea = 0.0f;
			for(u32 i = 0; i < 3; i++)
			{
				mSurfaceArea[i] = Math::Square(desc.Extents[i]);
				totalSurfaceArea += mSurfaceArea[i];
			}

			if(totalSurfaceArea > 0.0f)
			{
				const float invTotalSurfaceArea = 1.0f / totalSurfaceArea;
				for(u32 i = 0; i < 3; i++)
					mSurfaceArea[i] *= invTotalSurfaceArea;

				mSurfaceArea[1] += mSurfaceArea[0];
				mSurfaceArea[2] = 1.0f;
			}
		}
		break;
	case ParticleEmitterBoxType::Edge:
		{
			float totalEdgeLength = 0.0f;
			for(u32 i = 0; i < 3; i++)
			{
				mEdgeLengths[i] = desc.Extents[i];
				totalEdgeLength += mEdgeLengths[i];
			}

			if(totalEdgeLength > 0.0f)
			{
				const float invTotalEdgeLength = 1.0f / totalEdgeLength;
				for(u32 i = 0; i < 3; i++)
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

u32 ParticleEmitterBoxShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleRandom(this, random, particles, count);
}

void ParticleEmitterBoxShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	switch(mInfo.Type)
	{
	default:
	case ParticleEmitterBoxType::Volume:
		position.X = mInfo.Extents.X * random.GetSNorm();
		position.Y = mInfo.Extents.Y * random.GetSNorm();
		position.Z = mInfo.Extents.Z * random.GetSNorm();
		normal = Vector3::kUnitZ;
		break;
	case ParticleEmitterBoxType::Surface:
		{
			const float u = random.GetSNorm();
			const float v = random.GetSNorm();

			// Determine an axis (based on their size, larger being more likely)
			const float axisRnd = random.GetUNorm();
			u32 axis = 0;
			for(; axis < 3; axis++)
			{
				if(axisRnd <= mSurfaceArea[axis])
					break;
			}

			switch(axis)
			{
			case 0:
				position.X = mInfo.Extents.X * u;
				position.Y = mInfo.Extents.Y * v;
				position.Z = random.GetUNorm() > 0.5f ? mInfo.Extents.Z : -mInfo.Extents.Z;
				break;
			case 1:
				position.X = mInfo.Extents.X * u;
				position.Y = random.GetUNorm() > 0.5f ? mInfo.Extents.Y : -mInfo.Extents.Y;
				position.Z = mInfo.Extents.Z * v;
				break;
			case 2:
				position.X = random.GetUNorm() > 0.5f ? mInfo.Extents.X : -mInfo.Extents.X;
				position.Y = mInfo.Extents.Y * v;
				position.Z = mInfo.Extents.Z * u;
				break;
			default:
				break;
			}

			normal = Vector3::kUnitZ;
		}
		break;
	case ParticleEmitterBoxType::Edge:
		{
			const float u = random.GetSNorm();

			// Determine an axis (based on their length, longer being more likely)
			const float axisRnd = random.GetUNorm();
			u32 axis = 0;
			for(; axis < 3; axis++)
			{
				if(axisRnd <= mEdgeLengths[axis])
					break;
			}

			switch(axis)
			{
			case 0:
				position.X = mInfo.Extents.X * u;
				position.Y = random.GetUNorm() > 0.5f ? mInfo.Extents.Y : -mInfo.Extents.Y;
				position.Z = random.GetUNorm() > 0.5f ? mInfo.Extents.Z : -mInfo.Extents.Z;
				break;
			case 1:
				position.X = random.GetUNorm() > 0.5f ? mInfo.Extents.X : -mInfo.Extents.X;
				position.Y = mInfo.Extents.Y * u;
				position.Z = random.GetUNorm() > 0.5f ? mInfo.Extents.Z : -mInfo.Extents.Z;
				break;
			case 2:
				position.X = random.GetUNorm() > 0.5f ? mInfo.Extents.X : -mInfo.Extents.X;
				position.Y = random.GetUNorm() > 0.5f ? mInfo.Extents.Y : -mInfo.Extents.Y;
				position.Z = mInfo.Extents.Z * u;
				break;
			default:
				break;
			}

			normal = Vector3::kUnitZ;
		}
		break;
	}
}

void ParticleEmitterBoxShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.SetMin(-mInfo.Extents);
	shape.SetMax(mInfo.Extents);

	velocity.SetMin(Vector3::kZero);
	velocity.SetMax(Vector3::kUnitZ);
}

SPtr<ParticleEmitterBoxShape> ParticleEmitterBoxShape::Create(const PARTICLE_BOX_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterBoxShape>(desc);
}

SPtr<ParticleEmitterBoxShape> ParticleEmitterBoxShape::Create()
{
	return B3DMakeShared<ParticleEmitterBoxShape>();
}

RTTITypeBase* ParticleEmitterBoxShape::GetRttiStatic()
{
	return ParticleEmitterBoxShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterBoxShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterLineShape::ParticleEmitterLineShape(const PARTICLE_LINE_SHAPE_DESC& desc)
	: mInfo(desc)
{}

u32 ParticleEmitterLineShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleMode(this, mInfo.Mode.Type, mInfo.Length, mInfo.Mode.Speed, mInfo.Mode.Interval, random, particles, count, state);
}

void ParticleEmitterLineShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	position = Vector3(random.GetSNorm() * mInfo.Length * 0.5f, 0.0f, 0.0f);
	normal = Vector3::kUnitZ;
}

void ParticleEmitterLineShape::SpawnInternal(float t, Vector3& position, Vector3& normal) const
{
	position = Vector3(t * mInfo.Length - mInfo.Length * 0.5f, 0.0f, 0.0f);
	normal = Vector3::kUnitZ;
}

void ParticleEmitterLineShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.SetMin(Vector3(-mInfo.Length * 0.5f, 0.0f, 0.0f));
	shape.SetMax(Vector3(mInfo.Length * 0.5f, 0.0f, 0.0f));

	velocity.SetMin(Vector3::kZero);
	velocity.SetMax(Vector3::kUnitZ);
}

SPtr<ParticleEmitterLineShape> ParticleEmitterLineShape::Create(const PARTICLE_LINE_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterLineShape>(desc);
}

SPtr<ParticleEmitterLineShape> ParticleEmitterLineShape::Create()
{
	return B3DMakeShared<ParticleEmitterLineShape>();
}

RTTITypeBase* ParticleEmitterLineShape::GetRttiStatic()
{
	return ParticleEmitterLineShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterLineShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterCircleShape::ParticleEmitterCircleShape(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
	: mInfo(desc)
{}

u32 ParticleEmitterCircleShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleMode(this, mInfo.Mode.Type, mInfo.Arc.GetValueInRadians(), mInfo.Mode.Speed * Math::kDeG2Rad, mInfo.Mode.Interval * Math::kDeG2Rad, random, particles, count, state);
}

void ParticleEmitterCircleShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	Vector2 pos2D;
	if(Math::ApproxEquals(mInfo.Arc.GetValueInDegrees(), 360.0f))
		pos2D = random.GetPointInCircleShell(mInfo.Thickness);
	else
		pos2D = random.GetPointInArcShell(mInfo.Arc, mInfo.Thickness);

	position = Vector3(pos2D.X * mInfo.Radius, pos2D.Y * mInfo.Radius, 0.0f);
	normal = Vector3::kUnitZ;
}

void ParticleEmitterCircleShape::SpawnInternal(float t, Vector3& position, Vector3& normal) const
{
	const Vector2 pos2D(Math::Cos(t), Math::Sin(t));

	position = Vector3(pos2D.X * mInfo.Radius, pos2D.Y * mInfo.Radius, 0.0f);
	normal = Vector3::kUnitZ;
}

void ParticleEmitterCircleShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.SetMin(Vector3(-mInfo.Radius, -mInfo.Radius, 0.0f));
	shape.SetMax(Vector3(mInfo.Radius, mInfo.Radius, 0.0f));

	velocity.SetMin(Vector3::kZero);
	velocity.SetMax(Vector3::kUnitZ);
}

SPtr<ParticleEmitterCircleShape> ParticleEmitterCircleShape::Create(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterCircleShape>(desc);
}

SPtr<ParticleEmitterCircleShape> ParticleEmitterCircleShape::Create()
{
	return B3DMakeShared<ParticleEmitterCircleShape>();
}

RTTITypeBase* ParticleEmitterCircleShape::GetRttiStatic()
{
	return ParticleEmitterCircleShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterCircleShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterRectShape::ParticleEmitterRectShape(const PARTICLE_RECT_SHAPE_DESC& desc)
	: mInfo(desc)
{}

u32 ParticleEmitterRectShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleRandom(this, random, particles, count);
}

void ParticleEmitterRectShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	position.X = random.GetSNorm() * mInfo.Extents.X;
	position.Y = random.GetSNorm() * mInfo.Extents.Y;
	position.Z = 0.0f;

	normal = Vector3::kUnitZ;
}

void ParticleEmitterRectShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.SetMin(Vector3(-mInfo.Extents.X, -mInfo.Extents.Y, 0.0f));
	shape.SetMax(Vector3(mInfo.Extents.X, mInfo.Extents.Y, 0.0f));

	velocity.SetMin(Vector3::kZero);
	velocity.SetMax(Vector3::kUnitZ);
}

SPtr<ParticleEmitterRectShape> ParticleEmitterRectShape::Create(const PARTICLE_RECT_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterRectShape>(desc);
}

SPtr<ParticleEmitterRectShape> ParticleEmitterRectShape::Create()
{
	return B3DMakeShared<ParticleEmitterRectShape>();
}

RTTITypeBase* ParticleEmitterRectShape::GetRttiStatic()
{
	return ParticleEmitterRectShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterRectShape::GetRtti() const
{
	return GetRttiStatic();
}

bool MeshEmissionHelper::Initialize(const HMesh& mesh, bool perVertex, bool skinning)
{
	// Validate
	if(mesh)
	{
		mMeshData = mesh->GetCachedData();

		if(!mMeshData)
		{
			B3D_LOG(Verbose, Particles, "Particle emitter mesh not created with CPU caching, performing an expensive GPU read.");

			mMeshData = mesh->AllocBuffer();
			mesh->ReadData(mMeshData);

			GetCoreThread().Submit(true);
		}
	}

	if(!mMeshData)
	{
		// No warning as user could want to add mesh data later
		return false;
	}

	const SPtr<VertexDescription>& vertexDesc = mMeshData->GetVertexDescription();
	const VertexElement* positionElement = vertexDesc->GetElement(VES_POSITION);
	if(positionElement == nullptr)
	{
		B3D_LOG(Error, Particles, "Mesh particle emitter requires position vertex data to be present in the provided mesh data.");
		return false;
	}

	if(positionElement->GetType() != VET_FLOAT3)
	{
		B3D_LOG(Error, Particles, "Mesh particle emitter requires position vertex data to use 3D vectors for individual elements.");
		return false;
	}

	if(!perVertex && (mMeshData->GetIndexCount() % 3 != 0))
	{
		B3D_LOG(Error, Particles, "Unless using the per-vertex emission mode, mesh particle emitter requires the number "
								 "of indices to be divisible by three, using a triangle list layout.");
		return false;
	}

	if(skinning)
	{
		const VertexElement* blendIdxElement = vertexDesc->GetElement(VES_BLEND_INDICES);
		const VertexElement* blendWeightElement = vertexDesc->GetElement(VES_BLEND_WEIGHTS);

		if(blendIdxElement == nullptr || blendWeightElement == nullptr)
		{
			B3D_LOG(Error, Particles, "Skinned mesh particle emitter requires blend indices and blend weight data to be present in the "
									 "provided mesh data.");
			return false;
		}

		if(blendIdxElement->GetType() != VET_UBYTE4)
		{
			B3D_LOG(Error, Particles, "Skinned mesh particle emitter requires blend indices to be a 4-byte encoded format.");
			return false;
		}

		if(blendWeightElement->GetType() != VET_FLOAT4)
		{
			B3D_LOG(Error, Particles, "Skinned mesh particle emitter requires blend weights to be a 4D vector format.");
			return false;
		}
	}

	// Initialize
	mVertices = mMeshData->GetElementData(VES_POSITION);
	mNumVertices = mMeshData->GetVertexCount();
	mVertexStride = vertexDesc->GetVertexStride();

	const VertexElement* normalElement = vertexDesc->GetElement(VES_NORMAL);

	mNormals = nullptr;
	if(normalElement)
	{
		if(normalElement->GetType() == VET_UBYTE4_NORM)
		{
			mNormals = mMeshData->GetElementData(VES_NORMAL);
			m32BitNormals = true;
		}
		else if(normalElement->GetType() == VET_FLOAT3)
		{
			mNormals = mMeshData->GetElementData(VES_NORMAL);
			m32BitNormals = false;
		}
	}

	if(skinning)
	{
		mBoneIndices = mMeshData->GetElementData(VES_BLEND_INDICES);
		mBoneWeights = mMeshData->GetElementData(VES_BLEND_WEIGHTS);
	}

	if(!perVertex)
		mWeightedTriangles.Calculate(*mMeshData);

	return true;
}

void MeshEmissionHelper::GetSequentialVertex(Vector3& position, Vector3& normal, u32& idx) const
{
	idx = mNextSequentialIdx;
	position = *(Vector3*)(mVertices + mVertexStride * idx);

	if(mNormals)
	{
		if(m32BitNormals)
			normal = MeshUtility::UnpackNormal(mNormals + mVertexStride * idx);
		else
			normal = *(Vector3*)(mNormals + mVertexStride * idx);
	}
	else
		normal = Vector3::kUnitZ;

	mNextSequentialIdx = (mNextSequentialIdx + 1) % mNumVertices;
}

void MeshEmissionHelper::GetRandomVertex(const Random& random, Vector3& position, Vector3& normal, u32& idx) const
{
	idx = random.Get() % mNumVertices;
	position = *(Vector3*)(mVertices + mVertexStride * idx);

	if(mNormals)
	{
		if(m32BitNormals)
			normal = MeshUtility::UnpackNormal(mNormals + mVertexStride * idx);
		else
			normal = *(Vector3*)(mNormals + mVertexStride * idx);
	}
	else
		normal = Vector3::kUnitZ;
}

void MeshEmissionHelper::GetRandomEdge(const Random& random, std::array<Vector3, 2>& position, std::array<Vector3, 2>& normal, std::array<u32, 2>& idx) const
{
	std::array<u32, 3> triIndices;
	mWeightedTriangles.GetTriangle(random, triIndices);

	// Pick edge
	// Note: Longer edges should be given higher chance, but we're assuming they are all equal length for performance
	const int32_t edge = random.GetRange(0, 2);
	switch(edge)
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

	if(mNormals)
	{
		if(m32BitNormals)
		{
			normal[0] = MeshUtility::UnpackNormal(mNormals + mVertexStride * idx[0]);
			normal[1] = MeshUtility::UnpackNormal(mNormals + mVertexStride * idx[1]);
		}
		else
		{
			normal[0] = *(Vector3*)(mNormals + mVertexStride * idx[0]);
			normal[1] = *(Vector3*)(mNormals + mVertexStride * idx[1]);
		}
	}
	else
	{
		normal[0] = Vector3::kUnitZ;
		normal[1] = Vector3::kUnitZ;
	}
}

void MeshEmissionHelper::GetRandomTriangle(const Random& random, std::array<Vector3, 3>& position, std::array<Vector3, 3>& normal, std::array<u32, 3>& idx) const
{
	mWeightedTriangles.GetTriangle(random, idx);

	for(uint32_t i = 0; i < 3; i++)
	{
		position[i] = *(Vector3*)(mVertices + mVertexStride * idx[i]);

		if(mNormals)
		{
			if(m32BitNormals)
				normal[i] = MeshUtility::UnpackNormal(mNormals + mVertexStride * idx[i]);
			else
				normal[i] = *(Vector3*)(mNormals + mVertexStride * idx[i]);
		}
		else
			normal[i] = Vector3::kUnitZ;
	}
}

Matrix4 MeshEmissionHelper::GetBlendMatrix(const Matrix4* bones, u32 vertexIdx) const
{
	if(bones)
	{
		const u32 boneIndices = *(u32*)(mBoneIndices + vertexIdx * mVertexStride);
		const Vector4& boneWeights = *(Vector4*)(mBoneWeights + vertexIdx * mVertexStride);

		return bones[boneIndices & 0xFF] * boneWeights[0] +
			bones[(boneIndices >> 8) & 0xFF] * boneWeights[1] +
			bones[(boneIndices >> 16) & 0xFF] * boneWeights[2] +
			bones[(boneIndices >> 24) & 0xFF] * boneWeights[3];
	}
	else
		return Matrix4::kIdentity;
}

ParticleEmitterStaticMeshShape::ParticleEmitterStaticMeshShape(const PARTICLE_STATIC_MESH_SHAPE_DESC& desc)
	: mInfo(desc)
{
	mIsValid = mMeshEmissionHelper.Initialize(desc.Mesh, desc.Type == ParticleEmitterMeshType::Vertex, false);
}

ParticleEmitterStaticMeshShape::ParticleEmitterStaticMeshShape()
{
	mIsValid = false;
}

void ParticleEmitterStaticMeshShape::SetOptions(const PARTICLE_STATIC_MESH_SHAPE_DESC& options)
{
	mInfo = options;
	mIsValid = mMeshEmissionHelper.Initialize(options.Mesh, options.Type == ParticleEmitterMeshType::Vertex, false);
}

u32 ParticleEmitterStaticMeshShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	if(count == 0)
		return particles.GetParticleCount();

	switch(mInfo.Type)
	{
	case ParticleEmitterMeshType::Vertex:
		if(mInfo.Sequential)
		{
			return SpawnMultiple(particles, count, [this](u32 idx, Vector3& position, Vector3& normal)
								 {
					u32 vertexIdx;
					mMeshEmissionHelper.GetSequentialVertex(position, normal, vertexIdx); });
		}
		else
		{
			return SpawnMultiple(particles, count, [this, &random](u32 idx, Vector3& position, Vector3& normal)
								 {
					u32 vertexIdx;
					mMeshEmissionHelper.GetRandomVertex(random, position, normal, vertexIdx); });
		}
	case ParticleEmitterMeshType::Edge:
		return SpawnMultiple(particles, count, [this, &random](u32 idx, Vector3& position, Vector3& normal)
							 {
				std::array<Vector3, 2> edgePositions, edgeNormals;
				std::array<u32, 2> edgeIndices;

				mMeshEmissionHelper.GetRandomEdge(random, edgePositions, edgeNormals, edgeIndices);

				const float rnd = random.GetUNorm();
				position = Math::Lerp(rnd, edgePositions[0], edgePositions[1]);
				normal = Math::Lerp(rnd, edgeNormals[0], edgeNormals[1]); });
	default:
	case ParticleEmitterMeshType::Triangle:
		return SpawnMultiple(particles, count, [this, &random](u32 idx, Vector3& position, Vector3& normal)
							 {
				std::array<Vector3, 3> triPositions, triNormals;
				std::array<u32, 3> triIndices;

				mMeshEmissionHelper.GetRandomTriangle(random, triPositions, triNormals, triIndices);

				position = Vector3::kZero;
				normal = Vector3::kZero;
				Vector3 barycenter = random.GetBarycentric();

				for (uint32_t i = 0; i < 3; i++)
				{
					position += triPositions[i] * barycenter[i];
					normal += triNormals[i] * barycenter[i];
				} });
	}
}

void ParticleEmitterStaticMeshShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	if(mInfo.Mesh.IsLoaded(false))
		shape = mInfo.Mesh->GetProperties().Bounds.GetBox();
	else
		shape = AABox::kBoxEmpty;

	velocity.SetMin(-Vector3::kOne);
	velocity.SetMax(Vector3::kOne);
}

SPtr<ParticleEmitterStaticMeshShape> ParticleEmitterStaticMeshShape::Create(const PARTICLE_STATIC_MESH_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterStaticMeshShape>(desc);
}

SPtr<ParticleEmitterStaticMeshShape> ParticleEmitterStaticMeshShape::Create()
{
	return B3DMakeShared<ParticleEmitterStaticMeshShape>();
}

RTTITypeBase* ParticleEmitterStaticMeshShape::GetRttiStatic()
{
	return ParticleEmitterStaticMeshShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterStaticMeshShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterSkinnedMeshShape::ParticleEmitterSkinnedMeshShape()
{
	mIsValid = false;
}

ParticleEmitterSkinnedMeshShape::ParticleEmitterSkinnedMeshShape(const PARTICLE_SKINNED_MESH_SHAPE_DESC& desc)
	: mInfo(desc)
{
	HMesh mesh;
	if(!desc.Renderable.Empty())
		mesh = desc.Renderable.GetActor()->GetMesh();

	mIsValid = mMeshEmissionHelper.Initialize(mesh, desc.Type == ParticleEmitterMeshType::Vertex, false);
}

void ParticleEmitterSkinnedMeshShape::SetOptions(const PARTICLE_SKINNED_MESH_SHAPE_DESC& options)
{
	mInfo = options;

	HMesh mesh;
	if(!options.Renderable.Empty())
		mesh = options.Renderable.GetActor()->GetMesh();

	mIsValid = mMeshEmissionHelper.Initialize(mesh, options.Type == ParticleEmitterMeshType::Vertex, false);
}

u32 ParticleEmitterSkinnedMeshShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	if(count == 0)
		return particles.GetParticleCount();

	const Matrix4* bones = nullptr;

	if(!mInfo.Renderable.Empty())
	{
		const SPtr<Renderable>& renderable = mInfo.Renderable.GetActor();
		const SPtr<Animation>& animation = renderable->GetAnimation();
		;
		if(animation)
		{
			const u64 animId = animation->GetIdInternal();

			if(state.AnimData)
			{
				const auto iterFind = state.AnimData->Infos.find(animId);
				if(iterFind != state.AnimData->Infos.end())
					bones = &state.AnimData->Transforms[iterFind->second.PoseInfo.StartIdx];
			}
		}
	}

	switch(mInfo.Type)
	{
	case ParticleEmitterMeshType::Vertex:
		if(mInfo.Sequential)
		{
			return SpawnMultiple(particles, count, [this, bones](u32 idx, Vector3& position, Vector3& normal)
								 {
					u32 vertexIdx;
					mMeshEmissionHelper.GetSequentialVertex(position, normal, vertexIdx);

					Matrix4 blendMatrix = mMeshEmissionHelper.GetBlendMatrix(bones, vertexIdx);
					position = blendMatrix.MultiplyAffine(position);
					normal = blendMatrix.MultiplyDirection(normal); });
		}
		else
		{
			return SpawnMultiple(particles, count, [this, &random, bones](u32 idx, Vector3& position, Vector3& normal)
								 {
					u32 vertexIdx;
					mMeshEmissionHelper.GetRandomVertex(random, position, normal, vertexIdx);

					Matrix4 blendMatrix = mMeshEmissionHelper.GetBlendMatrix(bones, vertexIdx);
					position = blendMatrix.MultiplyAffine(position);
					normal = blendMatrix.MultiplyDirection(normal); });
		}
	case ParticleEmitterMeshType::Edge:
		return SpawnMultiple(particles, count, [this, &random, bones](u32 idx, Vector3& position, Vector3& normal)
							 {
				std::array<Vector3, 2> edgePositions, edgeNormals;
				std::array<u32, 2> edgeIndices;

				mMeshEmissionHelper.GetRandomEdge(random, edgePositions, edgeNormals, edgeIndices);

				for(uint32_t i = 0; i < 2; i++)
				{
					Matrix4 blendMatrix = mMeshEmissionHelper.GetBlendMatrix(bones, edgeIndices[i]);
					edgePositions[i] = blendMatrix.MultiplyAffine(edgePositions[i]);
					edgeNormals[i] = blendMatrix.MultiplyAffine(edgeNormals[i]);
				}

				const float rnd = random.GetUNorm();
				position = Math::Lerp(rnd, edgePositions[0], edgePositions[1]);
				normal = Math::Lerp(rnd, edgeNormals[0], edgeNormals[1]); });
	default:
	case ParticleEmitterMeshType::Triangle:
		return SpawnMultiple(particles, count, [this, &random, bones](u32 idx, Vector3& position, Vector3& normal)
							 {
				std::array<Vector3, 3> triPositions, triNormals;
				std::array<u32, 3> triIndices;

				mMeshEmissionHelper.GetRandomTriangle(random, triPositions, triNormals, triIndices);

				position = Vector3::kZero;
				normal = Vector3::kZero;
				Vector3 barycenter = random.GetBarycentric();

				for(uint32_t i = 0; i < 3; i++)
				{
					Matrix4 blendMatrix = mMeshEmissionHelper.GetBlendMatrix(bones, triIndices[i]);
					triPositions[i] = blendMatrix.MultiplyAffine(triPositions[i]);
					triNormals[i] = blendMatrix.MultiplyAffine(triNormals[i]);
				}

				for (uint32_t i = 0; i < 3; i++)
				{
					position += triPositions[i] * barycenter[i];
					normal += triNormals[i] * barycenter[i];
				} });
	};
}

void ParticleEmitterSkinnedMeshShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	if(!mInfo.Renderable.Empty())
	{
		const SPtr<Renderable>& renderable = mInfo.Renderable.GetActor();
		const SPtr<Animation>& anim = renderable->GetAnimation();
		if(anim)
		{
			// No culling, make the box infinite
			if(!anim->GetCulling())
				shape = AABox::kInfBox;
			else
				shape = anim->GetBounds();
		}
		else
		{
			const HMesh& mesh = renderable->GetMesh();
			if(mesh.IsLoaded(false))
				shape = mesh->GetProperties().Bounds.GetBox();
			else
				shape = AABox::kBoxEmpty;
		}
	}
	else
		shape = AABox::kBoxEmpty;

	velocity.SetMin(-Vector3::kOne);
	velocity.SetMax(Vector3::kOne);
}

SPtr<ParticleEmitterSkinnedMeshShape> ParticleEmitterSkinnedMeshShape::Create(const PARTICLE_SKINNED_MESH_SHAPE_DESC& desc)
{
	return B3DMakeShared<ParticleEmitterSkinnedMeshShape>(desc);
}

SPtr<ParticleEmitterSkinnedMeshShape> ParticleEmitterSkinnedMeshShape::Create()
{
	return B3DMakeShared<ParticleEmitterSkinnedMeshShape>();
}

RTTITypeBase* ParticleEmitterSkinnedMeshShape::GetRttiStatic()
{
	return ParticleEmitterSkinnedMeshShapeRTTI::Instance();
}

RTTITypeBase* ParticleEmitterSkinnedMeshShape::GetRtti() const
{
	return GetRttiStatic();
}

void ParticleEmitter::SetEmissionBursts(Vector<ParticleBurst> bursts)
{
	mBursts = std::move(bursts);
	mBurstAccumulator.resize(mBursts.size());

	for(auto& entry : mBurstAccumulator)
		entry = 0.0f;
}

void ParticleEmitter::Spawn(Random& random, const ParticleSystemState& state, ParticleSet& set) const
{
	if(!mShape || !mShape->IsValid())
		return;

	const float emitterT = state.NrmTimeEnd;

	// Continous emission rate
	const float rate = mEmissionRate.Evaluate(emitterT, random);

	mEmitAccumulator += rate * state.TimeStep;
	auto numContinous = (u32)mEmitAccumulator;
	mEmitAccumulator -= (float)numContinous;

	// Bursts
	u32 numBurst = 0;
	const auto emitBursts = [this, &emitterT, &random](float start, float end)
	{
		constexpr float MIN_BURST_INTERVAL = 0.01f;

		u32 numBurst = 0;
		for(u32 i = 0; i < (u32)mBursts.size(); i++)
		{
			const ParticleBurst& burst = mBursts[i];

			const float relT0 = std::max(0.0f, start - burst.Time);
			const float relT1 = end - burst.Time;
			if(relT1 <= 0.0f)
				continue;

			// Handle initial burst cycle
			if(relT0 == 0.0f)
				numBurst += (u32)burst.Count.Evaluate(emitterT, random);

			// Handle remaining cycles
			const float dt = relT1 - relT0;
			const float interval = std::max(burst.Interval, MIN_BURST_INTERVAL);

			const float emitDuration = dt + mBurstAccumulator[i];
			const u32 emitCycles = Math::FloorToPosInt(emitDuration / interval);
			mBurstAccumulator[i] = emitDuration - emitCycles * interval;

			for(u32 j = 0; j < emitCycles; j++)
				numBurst += (u32)burst.Count.Evaluate(emitterT, random);
		}

		return numBurst;
	};

	// Handle loop
	if(state.TimeEnd < state.TimeStart)
	{
		numBurst += emitBursts(state.TimeStart, state.Length);

		// Reset accumulator
		for(auto& entry : mBurstAccumulator)
			entry = 0.0f;

		numBurst += emitBursts(0.0f, state.TimeEnd);
	}
	else
		numBurst += emitBursts(state.TimeStart, state.TimeEnd);

	const u32 startIdx = set.GetParticleCount();
	numContinous = Spawn(numContinous, random, state, set, true);

	state.System->PreSimulate(state, startIdx, numContinous, true, mEmitAccumulator);
	state.System->Simulate(state, startIdx, numContinous, true, mEmitAccumulator);

	Spawn(numBurst, random, state, set, false);
}

u32 ParticleEmitter::Spawn(u32 count, Random& random, const ParticleSystemState& state, ParticleSet& set, bool spacing) const
{
	const float subFrameSpacing = count > 0 ? 1.0f / count : 1.0f;

	const u32 numPartices = set.GetParticleCount() + count;
	if(!state.GpuSimulated)
	{
		if(numPartices > state.MaxParticles)
			count = state.MaxParticles - set.GetParticleCount();
	}

	const u32 firstIdx = mShape->SpawnInternal(random, set, count, state);
	const u32 endIdx = firstIdx + count;

	ParticleSetData& particles = set.GetParticles();
	float* emitterT = B3DStackAllocate<float>(sizeof(float) * count);

	if(spacing)
	{
		for(u32 i = 0; i < count; i++)
		{
			const float subFrameOffset = (i + mEmitAccumulator) * subFrameSpacing;
			emitterT[i] = state.NrmTimeStart + state.TimeStep * subFrameOffset;
		}
	}
	else
	{
		for(u32 i = 0; i < count; i++)
			emitterT[i] = state.NrmTimeEnd;
	}

	for(u32 i = firstIdx; i < endIdx; i++)
	{
		const float lifetime = mInitialLifetime.Evaluate(emitterT[i - firstIdx], random);

		particles.InitialLifetime[i] = lifetime;
		particles.Lifetime[i] = lifetime;
	}

	for(u32 i = firstIdx; i < endIdx; i++)
		particles.Velocity[i] *= mInitialSpeed.Evaluate(emitterT[i - firstIdx], random);

	if(!mUse3DSize)
	{
		for(u32 i = firstIdx; i < endIdx; i++)
		{
			const float size = mInitialSize.Evaluate(emitterT[i - firstIdx], random);

			// Encode UV flip in size XY as sign
			const float flipU = random.GetUNorm() < mFlipU ? -1.0f : 1.0f;
			const float flipV = random.GetUNorm() < mFlipV ? -1.0f : 1.0f;

			particles.Size[i] = Vector3(size * flipU, size * flipV, size);
		}
	}
	else
	{
		for(u32 i = firstIdx; i < endIdx; i++)
		{
			Vector3 size = mInitialSize3D.Evaluate(emitterT[i - firstIdx], random);

			// Encode UV flip in size XY as sign
			size.X *= random.GetUNorm() < mFlipU ? -1.0f : 1.0f;
			size.Y *= random.GetUNorm() < mFlipV ? -1.0f : 1.0f;

			particles.Size[i] = size;
		}
	}

	if(mRandomOffset > 0.0f)
	{
		for(u32 i = firstIdx; i < endIdx; i++)
			particles.Position[i] += Vector3(random.GetSNorm(), random.GetSNorm(), random.GetSNorm()) * mRandomOffset;
	}

	if(!mUse3DRotation)
	{
		for(u32 i = firstIdx; i < endIdx; i++)
		{
			const float rotation = mInitialRotation.Evaluate(emitterT[i - firstIdx], random);
			particles.Rotation[i] = Vector3(rotation, 0.0f, 0.0f);
		}
	}
	else
	{
		for(u32 i = firstIdx; i < endIdx; i++)
		{
			const Vector3 rotation = mInitialRotation3D.Evaluate(emitterT[i - firstIdx], random);
			particles.Rotation[i] = rotation;
		}
	}

	for(u32 i = firstIdx; i < endIdx; i++)
		particles.Color[i] = mInitialColor.Evaluate(emitterT[i - firstIdx], random);

	for(u32 i = firstIdx; i < endIdx; i++)
		particles.Seed[i] = random.Get();

	for(u32 i = firstIdx; i < endIdx; i++)
		particles.Frame[i] = 0.0f;

	// If in world-space we apply the transform here, otherwise we apply it in the rendering code
	if(state.WorldSpace)
	{
		for(u32 i = firstIdx; i < endIdx; i++)
			particles.Position[i] = state.LocalToWorld.MultiplyAffine(particles.Position[i]);

		for(u32 i = firstIdx; i < endIdx; i++)
			particles.Velocity[i] = state.LocalToWorld.MultiplyDirection(particles.Velocity[i]);
	}

	B3DStackFree(emitterT);

	return count;
}

SPtr<ParticleEmitter> ParticleEmitter::Create()
{
	return B3DMakeShared<ParticleEmitter>();
}

RTTITypeBase* ParticleEmitter::GetRttiStatic()
{
	return ParticleEmitterRTTI::Instance();
}

RTTITypeBase* ParticleEmitter::GetRtti() const
{
	return GetRttiStatic();
}
