//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DParticleEmitter.h"
#include "Mesh/B3DMeshData.h"
#include "Mesh/B3DMeshUtility.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Math/B3DRandom.h"
#include "Components/B3DRenderable.h"
#include "Private/Particles/B3DParticleSet.h"
#include "RTTI/B3DParticleSystemRTTI.h"
#include "Animation/B3DAnimationScene.h"
#include "Components/B3DAnimation.h"
#include "Mesh/B3DMesh.h"

using namespace b3d;

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

ParticleEmitterConeShape::ParticleEmitterConeShape(const ParticleConeShapeSettings& settings)
	: mSettings(settings)
{}

u32 ParticleEmitterConeShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleMode(this, mSettings.Mode.Type, mSettings.Arc.GetValueInRadians(), mSettings.Mode.Speed * Math::kDeG2Rad, mSettings.Mode.Interval * Math::kDeG2Rad, random, particles, count, state);
}

void ParticleEmitterConeShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	Vector2 pos2D;
	if(Math::ApproxEquals(mSettings.Arc.GetValueInRadians(), 360.0f))
		pos2D = random.GetPointInCircleShell(mSettings.Thickness);
	else
		pos2D = random.GetPointInArcShell(mSettings.Arc, mSettings.Thickness);

	GetPointInCone(pos2D, random.GetUNorm() * mSettings.Length, position, normal);
}

void ParticleEmitterConeShape::SpawnInternal(float t, Vector3& position, Vector3& normal) const
{
	const Vector2 pos2D(Math::Cos(t), Math::Sin(t));

	GetPointInCone(pos2D, 0.0f, position, normal);
}

void ParticleEmitterConeShape::GetPointInCone(const Vector2& pos2D, float distance, Vector3& position, Vector3& normal) const
{
	const float angleSin = Math::Sin(mSettings.Angle);
	normal = Vector3(pos2D.X * angleSin, pos2D.Y * angleSin, Math::Cos(mSettings.Angle));
	normal.Normalize();

	position = Vector3(pos2D.X * mSettings.Radius, pos2D.Y * mSettings.Radius, 0.0f);

	if(mSettings.Type == ParticleEmitterConeType::Volume)
		position += normal * distance;
}

SPtr<ParticleEmitterConeShape> ParticleEmitterConeShape::Create(const ParticleConeShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterConeShape>(settings);
}

SPtr<ParticleEmitterConeShape> ParticleEmitterConeShape::Create()
{
	return B3DMakeShared<ParticleEmitterConeShape>();
}

void ParticleEmitterConeShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	const float sinAngle = Math::Sin(mSettings.Angle);
	const float cosAngle = Math::Cos(mSettings.Angle);

	if(mSettings.Type == ParticleEmitterConeType::Base)
	{
		shape.Minimum = Vector3(-mSettings.Radius, -mSettings.Radius, 0.0f);
		shape.Maximum = Vector3(mSettings.Radius, mSettings.Radius, 0.0f);
	}
	else
	{
		const float topRadius = mSettings.Radius + mSettings.Length * sinAngle;
		const float length = mSettings.Length * cosAngle;

		shape.Minimum = Vector3(-topRadius, -topRadius, 0.0f);
		shape.Maximum = Vector3(topRadius, topRadius, length);
	}

	velocity.Minimum = Vector3(-sinAngle, -sinAngle, 0.0f);
	velocity.Maximum = Vector3(sinAngle, sinAngle, 1.0f);
}

RTTIType* ParticleEmitterConeShape::GetRttiStatic()
{
	return ParticleEmitterConeShapeRTTI::Instance();
}

RTTIType* ParticleEmitterConeShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterSphereShape::ParticleEmitterSphereShape(const ParticleSphereShapeSettings& settings)
	: mSettings(settings)
{}

u32 ParticleEmitterSphereShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleRandom(this, random, particles, count);
}

void ParticleEmitterSphereShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	position = random.GetPointInSphereShell(mSettings.Thickness);
	normal = Vector3::Normalize(position);

	position *= mSettings.Radius;
}

void ParticleEmitterSphereShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.Minimum = Vector3::kOne * -mSettings.Radius;
	shape.Maximum = Vector3::kOne * mSettings.Radius;

	velocity.Minimum = -Vector3::kOne;
	velocity.Maximum = Vector3::kOne;
}

SPtr<ParticleEmitterSphereShape> ParticleEmitterSphereShape::Create(const ParticleSphereShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterSphereShape>(settings);
}

SPtr<ParticleEmitterSphereShape> ParticleEmitterSphereShape::Create()
{
	return B3DMakeShared<ParticleEmitterSphereShape>();
}

RTTIType* ParticleEmitterSphereShape::GetRttiStatic()
{
	return ParticleEmitterSphereShapeRTTI::Instance();
}

RTTIType* ParticleEmitterSphereShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterHemisphereShape::ParticleEmitterHemisphereShape(const ParticleHemisphereShapeSettings& settings)
	: mSettings(settings)
{}

u32 ParticleEmitterHemisphereShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleRandom(this, random, particles, count);
}

void ParticleEmitterHemisphereShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	position = random.GetPointInSphereShell(mSettings.Thickness);
	if(position.Z > 0.0f)
		position.Z *= -1.0f;

	normal = Vector3::Normalize(position);
	position *= mSettings.Radius;
}

void ParticleEmitterHemisphereShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.Minimum = Vector3(-mSettings.Radius, -mSettings.Radius, 0.0f);
	shape.Maximum = Vector3::kOne * mSettings.Radius;

	velocity.Minimum = Vector3(-1.0f, -1.0f, 0.0f);
	velocity.Maximum = Vector3::kOne;
}

SPtr<ParticleEmitterHemisphereShape> ParticleEmitterHemisphereShape::Create(const ParticleHemisphereShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterHemisphereShape>(settings);
}

SPtr<ParticleEmitterHemisphereShape> ParticleEmitterHemisphereShape::Create()
{
	return B3DMakeShared<ParticleEmitterHemisphereShape>();
}

RTTIType* ParticleEmitterHemisphereShape::GetRttiStatic()
{
	return ParticleEmitterHemisphereShapeRTTI::Instance();
}

RTTIType* ParticleEmitterHemisphereShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterBoxShape::ParticleEmitterBoxShape(const ParticleBoxShapeSettings& settings)
	: mSettings(settings)
{
	switch(mSettings.Type)
	{
	case ParticleEmitterBoxType::Surface:
		{
			float totalSurfaceArea = 0.0f;
			for(u32 i = 0; i < 3; i++)
			{
				mSurfaceArea[i] = Math::Square(settings.Extents[i]);
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
				mEdgeLengths[i] = settings.Extents[i];
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
	switch(mSettings.Type)
	{
	default:
	case ParticleEmitterBoxType::Volume:
		position.X = mSettings.Extents.X * random.GetSNorm();
		position.Y = mSettings.Extents.Y * random.GetSNorm();
		position.Z = mSettings.Extents.Z * random.GetSNorm();
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
				position.X = mSettings.Extents.X * u;
				position.Y = mSettings.Extents.Y * v;
				position.Z = random.GetUNorm() > 0.5f ? mSettings.Extents.Z : -mSettings.Extents.Z;
				break;
			case 1:
				position.X = mSettings.Extents.X * u;
				position.Y = random.GetUNorm() > 0.5f ? mSettings.Extents.Y : -mSettings.Extents.Y;
				position.Z = mSettings.Extents.Z * v;
				break;
			case 2:
				position.X = random.GetUNorm() > 0.5f ? mSettings.Extents.X : -mSettings.Extents.X;
				position.Y = mSettings.Extents.Y * v;
				position.Z = mSettings.Extents.Z * u;
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
				position.X = mSettings.Extents.X * u;
				position.Y = random.GetUNorm() > 0.5f ? mSettings.Extents.Y : -mSettings.Extents.Y;
				position.Z = random.GetUNorm() > 0.5f ? mSettings.Extents.Z : -mSettings.Extents.Z;
				break;
			case 1:
				position.X = random.GetUNorm() > 0.5f ? mSettings.Extents.X : -mSettings.Extents.X;
				position.Y = mSettings.Extents.Y * u;
				position.Z = random.GetUNorm() > 0.5f ? mSettings.Extents.Z : -mSettings.Extents.Z;
				break;
			case 2:
				position.X = random.GetUNorm() > 0.5f ? mSettings.Extents.X : -mSettings.Extents.X;
				position.Y = random.GetUNorm() > 0.5f ? mSettings.Extents.Y : -mSettings.Extents.Y;
				position.Z = mSettings.Extents.Z * u;
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
	shape.Minimum = -mSettings.Extents;
	shape.Maximum = mSettings.Extents;

	velocity.Minimum = Vector3::kZero;
	velocity.Maximum = Vector3::kUnitZ;
}

SPtr<ParticleEmitterBoxShape> ParticleEmitterBoxShape::Create(const ParticleBoxShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterBoxShape>(settings);
}

SPtr<ParticleEmitterBoxShape> ParticleEmitterBoxShape::Create()
{
	return B3DMakeShared<ParticleEmitterBoxShape>();
}

RTTIType* ParticleEmitterBoxShape::GetRttiStatic()
{
	return ParticleEmitterBoxShapeRTTI::Instance();
}

RTTIType* ParticleEmitterBoxShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterLineShape::ParticleEmitterLineShape(const ParticleLineShapeSettings& settings)
	: mSettings(settings)
{}

u32 ParticleEmitterLineShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleMode(this, mSettings.Mode.Type, mSettings.Length, mSettings.Mode.Speed, mSettings.Mode.Interval, random, particles, count, state);
}

void ParticleEmitterLineShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	position = Vector3(random.GetSNorm() * mSettings.Length * 0.5f, 0.0f, 0.0f);
	normal = Vector3::kUnitZ;
}

void ParticleEmitterLineShape::SpawnInternal(float t, Vector3& position, Vector3& normal) const
{
	position = Vector3(t * mSettings.Length - mSettings.Length * 0.5f, 0.0f, 0.0f);
	normal = Vector3::kUnitZ;
}

void ParticleEmitterLineShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.Minimum = Vector3(-mSettings.Length * 0.5f, 0.0f, 0.0f);
	shape.Maximum = Vector3(mSettings.Length * 0.5f, 0.0f, 0.0f);

	velocity.Minimum = Vector3::kZero;
	velocity.Maximum = Vector3::kUnitZ;
}

SPtr<ParticleEmitterLineShape> ParticleEmitterLineShape::Create(const ParticleLineShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterLineShape>(settings);
}

SPtr<ParticleEmitterLineShape> ParticleEmitterLineShape::Create()
{
	return B3DMakeShared<ParticleEmitterLineShape>();
}

RTTIType* ParticleEmitterLineShape::GetRttiStatic()
{
	return ParticleEmitterLineShapeRTTI::Instance();
}

RTTIType* ParticleEmitterLineShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterCircleShape::ParticleEmitterCircleShape(const ParticleCircleShapeSettings& settings)
	: mSettings(settings)
{}

u32 ParticleEmitterCircleShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleMode(this, mSettings.Mode.Type, mSettings.Arc.GetValueInRadians(), mSettings.Mode.Speed * Math::kDeG2Rad, mSettings.Mode.Interval * Math::kDeG2Rad, random, particles, count, state);
}

void ParticleEmitterCircleShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	Vector2 pos2D;
	if(Math::ApproxEquals(mSettings.Arc.GetValueInDegrees(), 360.0f))
		pos2D = random.GetPointInCircleShell(mSettings.Thickness);
	else
		pos2D = random.GetPointInArcShell(mSettings.Arc, mSettings.Thickness);

	position = Vector3(pos2D.X * mSettings.Radius, pos2D.Y * mSettings.Radius, 0.0f);
	normal = Vector3::kUnitZ;
}

void ParticleEmitterCircleShape::SpawnInternal(float t, Vector3& position, Vector3& normal) const
{
	const Vector2 pos2D(Math::Cos(t), Math::Sin(t));

	position = Vector3(pos2D.X * mSettings.Radius, pos2D.Y * mSettings.Radius, 0.0f);
	normal = Vector3::kUnitZ;
}

void ParticleEmitterCircleShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.Minimum = Vector3(-mSettings.Radius, -mSettings.Radius, 0.0f);
	shape.Maximum = Vector3(mSettings.Radius, mSettings.Radius, 0.0f);

	velocity.Minimum = Vector3::kZero;
	velocity.Maximum = Vector3::kUnitZ;
}

SPtr<ParticleEmitterCircleShape> ParticleEmitterCircleShape::Create(const ParticleCircleShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterCircleShape>(settings);
}

SPtr<ParticleEmitterCircleShape> ParticleEmitterCircleShape::Create()
{
	return B3DMakeShared<ParticleEmitterCircleShape>();
}

RTTIType* ParticleEmitterCircleShape::GetRttiStatic()
{
	return ParticleEmitterCircleShapeRTTI::Instance();
}

RTTIType* ParticleEmitterCircleShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterRectShape::ParticleEmitterRectShape(const ParticleRectangleShapeSettings& settings)
	: mSettings(settings)
{}

u32 ParticleEmitterRectShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	return SpawnMultipleRandom(this, random, particles, count);
}

void ParticleEmitterRectShape::SpawnInternal(const Random& random, Vector3& position, Vector3& normal) const
{
	position.X = random.GetSNorm() * mSettings.Extents.X;
	position.Y = random.GetSNorm() * mSettings.Extents.Y;
	position.Z = 0.0f;

	normal = Vector3::kUnitZ;
}

void ParticleEmitterRectShape::CalcBounds(AABox& shape, AABox& velocity) const
{
	shape.Minimum = Vector3(-mSettings.Extents.X, -mSettings.Extents.Y, 0.0f);
	shape.Maximum = Vector3(mSettings.Extents.X, mSettings.Extents.Y, 0.0f);

	velocity.Minimum = Vector3::kZero;
	velocity.Maximum = Vector3::kUnitZ;
}

SPtr<ParticleEmitterRectShape> ParticleEmitterRectShape::Create(const ParticleRectangleShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterRectShape>(settings);
}

SPtr<ParticleEmitterRectShape> ParticleEmitterRectShape::Create()
{
	return B3DMakeShared<ParticleEmitterRectShape>();
}

RTTIType* ParticleEmitterRectShape::GetRttiStatic()
{
	return ParticleEmitterRectShapeRTTI::Instance();
}

RTTIType* ParticleEmitterRectShape::GetRtti() const
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
			B3D_LOG(Verbose, LogParticles, "Particle emitter mesh not created with CPU caching, performing an expensive GPU read.");

			mMeshData = mesh->AllocBuffer();
			mesh->ReadData(mMeshData);

			GetRenderThread().PostCommand([] {}, "MeshEmissionHelper", true);
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
		B3D_LOG(Error, LogParticles, "Mesh particle emitter requires position vertex data to be present in the provided mesh data.");
		return false;
	}

	if(positionElement->GetType() != VET_FLOAT3)
	{
		B3D_LOG(Error, LogParticles, "Mesh particle emitter requires position vertex data to use 3D vectors for individual elements.");
		return false;
	}

	if(!perVertex && (mMeshData->GetIndexCount() % 3 != 0))
	{
		B3D_LOG(Error, LogParticles, "Unless using the per-vertex emission mode, mesh particle emitter requires the number "
								 "of indices to be divisible by three, using a triangle list layout.");
		return false;
	}

	if(skinning)
	{
		const VertexElement* blendIdxElement = vertexDesc->GetElement(VES_BLEND_INDICES);
		const VertexElement* blendWeightElement = vertexDesc->GetElement(VES_BLEND_WEIGHTS);

		if(blendIdxElement == nullptr || blendWeightElement == nullptr)
		{
			B3D_LOG(Error, LogParticles, "Skinned mesh particle emitter requires blend indices and blend weight data to be present in the "
									 "provided mesh data.");
			return false;
		}

		if(blendIdxElement->GetType() != VET_UBYTE4)
		{
			B3D_LOG(Error, LogParticles, "Skinned mesh particle emitter requires blend indices to be a 4-byte encoded format.");
			return false;
		}

		if(blendWeightElement->GetType() != VET_FLOAT4)
		{
			B3D_LOG(Error, LogParticles, "Skinned mesh particle emitter requires blend weights to be a 4D vector format.");
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

ParticleEmitterStaticMeshShape::ParticleEmitterStaticMeshShape(const ParticleStaticMeshShapeSettings& settings)
	: mSettings(settings)
{
	mIsValid = mMeshEmissionHelper.Initialize(settings.Mesh, settings.Type == ParticleEmitterMeshType::Vertex, false);
}

ParticleEmitterStaticMeshShape::ParticleEmitterStaticMeshShape()
{
	mIsValid = false;
}

void ParticleEmitterStaticMeshShape::SetSettings(const ParticleStaticMeshShapeSettings& settings)
{
	mSettings = settings;
	mIsValid = mMeshEmissionHelper.Initialize(settings.Mesh, settings.Type == ParticleEmitterMeshType::Vertex, false);
}

u32 ParticleEmitterStaticMeshShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	if(count == 0)
		return particles.GetParticleCount();

	switch(mSettings.Type)
	{
	case ParticleEmitterMeshType::Vertex:
		if(mSettings.Sequential)
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
	if(mSettings.Mesh.IsLoaded(false))
		shape = mSettings.Mesh->GetProperties().Bounds.GetBox();
	else
		shape = AABox::kEmpty;

	velocity.Minimum = -Vector3::kOne;
	velocity.Maximum = Vector3::kOne;
}

SPtr<ParticleEmitterStaticMeshShape> ParticleEmitterStaticMeshShape::Create(const ParticleStaticMeshShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterStaticMeshShape>(settings);
}

SPtr<ParticleEmitterStaticMeshShape> ParticleEmitterStaticMeshShape::Create()
{
	return B3DMakeShared<ParticleEmitterStaticMeshShape>();
}

RTTIType* ParticleEmitterStaticMeshShape::GetRttiStatic()
{
	return ParticleEmitterStaticMeshShapeRTTI::Instance();
}

RTTIType* ParticleEmitterStaticMeshShape::GetRtti() const
{
	return GetRttiStatic();
}

ParticleEmitterSkinnedMeshShape::ParticleEmitterSkinnedMeshShape()
{
	mIsValid = false;
}

ParticleEmitterSkinnedMeshShape::ParticleEmitterSkinnedMeshShape(const ParticleSkinnedMeshShapeSettings& settings)
	: mSettings(settings)
{
	HMesh mesh;
	if(settings.Renderable.IsValid())
		mesh = settings.Renderable->GetMesh();

	mIsValid = mMeshEmissionHelper.Initialize(mesh, settings.Type == ParticleEmitterMeshType::Vertex, false);
}

void ParticleEmitterSkinnedMeshShape::SetSettings(const ParticleSkinnedMeshShapeSettings& settings)
{
	mSettings = settings;

	HMesh mesh;
	if(settings.Renderable.IsValid())
		mesh = settings.Renderable->GetMesh();

	mIsValid = mMeshEmissionHelper.Initialize(mesh, settings.Type == ParticleEmitterMeshType::Vertex, false);
}

u32 ParticleEmitterSkinnedMeshShape::SpawnInternal(const Random& random, ParticleSet& particles, u32 count, const ParticleSystemState& state) const
{
	if(count == 0)
		return particles.GetParticleCount();

	const Matrix4* bones = nullptr;

	if(mSettings.Renderable.IsValid())
	{
		const HAnimation& animation = mSettings.Renderable->GetAnimation();
		if(animation.IsValid())
		{
			const u64 animId = animation->GetAnimationId();

			if(state.AnimData)
			{
				const auto iterFind = state.AnimData->Infos.find(animId);
				if(iterFind != state.AnimData->Infos.end())
					bones = &state.AnimData->Transforms[iterFind->second.PoseInfo.BoneStartIndex];
			}
		}
	}

	switch(mSettings.Type)
	{
	case ParticleEmitterMeshType::Vertex:
		if(mSettings.Sequential)
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
	if(mSettings.Renderable.IsValid())
	{
		const HRenderable& renderable = mSettings.Renderable;
		const HAnimation& animation = renderable->GetAnimation();
		if(animation.IsValid())
		{
			// No culling, make the box infinite
			if(!animation->GetEnableCull())
				shape = AABox::kInfinite;
			else
				shape = animation->GetCullingBounds();
		}
		else
		{
			const HMesh& mesh = renderable->GetMesh();
			if(mesh.IsLoaded(false))
				shape = mesh->GetProperties().Bounds.GetBox();
			else
				shape = AABox::kEmpty;
		}
	}
	else
		shape = AABox::kEmpty;

	velocity.Minimum = -Vector3::kOne;
	velocity.Maximum = Vector3::kOne;
}

SPtr<ParticleEmitterSkinnedMeshShape> ParticleEmitterSkinnedMeshShape::Create(const ParticleSkinnedMeshShapeSettings& settings)
{
	return B3DMakeShared<ParticleEmitterSkinnedMeshShape>(settings);
}

SPtr<ParticleEmitterSkinnedMeshShape> ParticleEmitterSkinnedMeshShape::Create()
{
	return B3DMakeShared<ParticleEmitterSkinnedMeshShape>();
}

RTTIType* ParticleEmitterSkinnedMeshShape::GetRttiStatic()
{
	return ParticleEmitterSkinnedMeshShapeRTTI::Instance();
}

RTTIType* ParticleEmitterSkinnedMeshShape::GetRtti() const
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

RTTIType* ParticleEmitter::GetRttiStatic()
{
	return ParticleEmitterRTTI::Instance();
}

RTTIType* ParticleEmitter::GetRtti() const
{
	return GetRttiStatic();
}
