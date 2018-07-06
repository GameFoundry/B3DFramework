//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsParticleEmitter.h"
#include "Mesh/BsMeshData.h"
#include "Mesh/BsMeshUtility.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Math/BsRandom.h"
#include "Private/Particles/BsParticleSet.h"
#include "Private/RTTI/BsParticleSystemRTTI.h"

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

	ParticleEmitterConeShape::ParticleEmitterConeShape(const PARTICLE_CONE_SHAPE_DESC& desc)
		:mInfo(desc)
	{ }

	UINT32 ParticleEmitterConeShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterConeShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		if(mInfo.type == ParticleEmitterConeType::Base)
		{
			if(Math::approxEquals(mInfo.arc.valueDegrees(), 360.0f))
			{
				const Vector2 pos2D = random.getPointInCircleShell(mInfo.thickness);

				const float angleSin = Math::sin(mInfo.angle);
				normal = Vector3(pos2D.x * angleSin, pos2D.y * angleSin, Math::cos(mInfo.angle));
				normal.normalize();

				position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
			}
			else
			{
				const Vector2 pos2D = random.getPointInArcShell(mInfo.arc, mInfo.thickness);

				const float angleSin = Math::sin(mInfo.angle);
				normal = Vector3(pos2D.x * angleSin, pos2D.y * angleSin, Math::cos(mInfo.angle));
				normal.normalize();

				position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
			}
		}
		else // Volume
		{
			if(Math::approxEquals(mInfo.arc.valueDegrees(), 360.0f))
			{
				const Vector2 pos2D = random.getPointInCircleShell(mInfo.thickness);

				const float angleSin = Math::sin(mInfo.angle);
				normal = Vector3(pos2D.x * angleSin, pos2D.y * angleSin, Math::cos(mInfo.angle));
				normal.normalize();

				position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
				position += normal * mInfo.length * random.getUNorm();
			}
		}
	}

	UPtr<ParticleEmitterConeShape> ParticleEmitterConeShape::create(const PARTICLE_CONE_SHAPE_DESC& desc)
	{
		ParticleEmitterConeShape* output = bs_new<ParticleEmitterConeShape>(desc);
		return bs_unique_ptr(output);
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

	UINT32 ParticleEmitterSphereShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterSphereShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = random.getPointInSphereShell(mInfo.thickness);
		normal = Vector3::normalize(position);

		position *= mInfo.radius;
	}

	UPtr<ParticleEmitterShape> ParticleEmitterSphereShape::create(const PARTICLE_SPHERE_SHAPE_DESC& desc)
	{
		return bs_unique_ptr<ParticleEmitterShape>(bs_new<ParticleEmitterSphereShape>(desc));
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

	UINT32 ParticleEmitterHemisphereShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterHemisphereShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = random.getPointInSphereShell(mInfo.thickness);
		if (position.z < 0.0f)
			position.z *= -1.0f;

		normal = Vector3::normalize(position);
		position *= mInfo.radius;
	}

	UPtr<ParticleEmitterShape> ParticleEmitterHemisphereShape::create(const PARTICLE_HEMISPHERE_SHAPE_DESC& desc)
	{
		return bs_unique_ptr<ParticleEmitterShape>(bs_new<ParticleEmitterHemisphereShape>(desc));
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

	UINT32 ParticleEmitterBoxShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterBoxShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
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

	UPtr<ParticleEmitterShape> ParticleEmitterBoxShape::create(const PARTICLE_BOX_SHAPE_DESC& desc)
	{
		ParticleEmitterBoxShape* output = bs_new<ParticleEmitterBoxShape>(desc);
		return bs_unique_ptr<ParticleEmitterShape>(output);
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

	UINT32 ParticleEmitterLineShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterLineShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position = random.getSNorm() * mInfo.length * 0.5f;
		normal = Vector3::UNIT_Z;
	}

	UPtr<ParticleEmitterShape> ParticleEmitterLineShape::create(const PARTICLE_LINE_SHAPE_DESC& desc)
	{
		return bs_unique_ptr<ParticleEmitterShape>(bs_new<ParticleEmitterLineShape>(desc));
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

	UINT32 ParticleEmitterCircleShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterCircleShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		Vector2 pos2D;
		if (Math::approxEquals(mInfo.arc.valueDegrees(), 360.0f))
			pos2D = random.getPointInCircleShell(mInfo.thickness);
		else
			pos2D = random.getPointInArcShell(mInfo.arc, mInfo.thickness);

		position = Vector3(pos2D.x * mInfo.radius, pos2D.y * mInfo.radius, 0.0f);
		normal = Vector3::UNIT_Z;
	}

	UPtr<ParticleEmitterShape> ParticleEmitterCircleShape::create(const PARTICLE_CIRCLE_SHAPE_DESC& desc)
	{
		ParticleEmitterCircleShape* output = bs_new<ParticleEmitterCircleShape>(desc);
		return bs_unique_ptr<ParticleEmitterShape>(output);
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

	UINT32 ParticleEmitterRectShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterRectShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		position.x = random.getSNorm() * mInfo.extents.x;
		position.y = random.getSNorm() * mInfo.extents.y;
		position.z = 0.0f;

		normal = Vector3::UNIT_Z;
	}

	UPtr<ParticleEmitterShape> ParticleEmitterRectShape::create(const PARTICLE_RECT_SHAPE_DESC& desc)
	{
		return bs_unique_ptr<ParticleEmitterShape>(bs_new<ParticleEmitterRectShape>(desc));
	}

	RTTITypeBase* ParticleEmitterRectShape::getRTTIStatic()
	{
		return ParticleEmitterRectShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterRectShape::getRTTI() const
	{
		return getRTTIStatic();
	}

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

		switch(mInfo.type)
		{
		case ParticleEmitterMeshType::Edge:
		case ParticleEmitterMeshType::Triangle: 
			mWeightedTriangles.calculate(*desc.meshData);
			break;
		default:
		case ParticleEmitterMeshType::Vertex: 
			break;
		}
	}

	UINT32 ParticleEmitterStaticMeshShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterStaticMeshShape::spawn(const Random& random, Vector3& position, Vector3& normal) const
	{
		switch(mInfo.type)
		{
		case ParticleEmitterMeshType::Vertex: 
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
			break;
		case ParticleEmitterMeshType::Edge:
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
			break;
		default:
		case ParticleEmitterMeshType::Triangle:
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
			break;
		}
	}

	UPtr<ParticleEmitterShape> ParticleEmitterStaticMeshShape::create(const PARTICLE_MESH_SHAPE_DESC& desc)
	{
		// TODO - Only support TRIANGLE_LIST draw operation
		// TODO - Ensure mesh-data at least has position (with correct type)

		ParticleEmitterStaticMeshShape* output = bs_new<ParticleEmitterStaticMeshShape>(desc);
		return bs_unique_ptr<ParticleEmitterShape>(output);
	}

	RTTITypeBase* ParticleEmitterStaticMeshShape::getRTTIStatic()
	{
		return ParticleEmitterStaticMeshShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterStaticMeshShape::getRTTI() const
	{
		return getRTTIStatic();
	}

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

		switch(mInfo.type)
		{
		case ParticleEmitterMeshType::Edge:
		case ParticleEmitterMeshType::Triangle: 
			mWeightedTriangles.calculate(*desc.meshData);
			break;
		default:
		case ParticleEmitterMeshType::Vertex: 
			break;
		}
	}

	Matrix4 ParticleEmitterSkinnedMeshShape::getBlendMatrix(const ParticleSystemState& state, UINT32 vertexIdx) const
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

	UINT32 ParticleEmitterSkinnedMeshShape::spawn(const Random& random, ParticleSet& particles, UINT32 count,
		const ParticleSystemState& state) const
	{
		const UINT32 index = particles.allocParticles(count);
		ParticleSetData& particleData = particles.getParticles();

		const UINT32 end = index + count;
		for (UINT32 i = index; i < end; i++)
			spawn(random, state, particleData.position[i], particleData.velocity[i]);

		return index;
	}

	void ParticleEmitterSkinnedMeshShape::spawn(const Random& random, const ParticleSystemState& state, Vector3& position,
		Vector3& normal) const
	{
		switch(mInfo.type)
		{
		case ParticleEmitterMeshType::Vertex:
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
			break;
		case ParticleEmitterMeshType::Edge:
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
			break;
		default:
		case ParticleEmitterMeshType::Triangle:
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
			break;
		}
	}

	UPtr<ParticleEmitterShape> ParticleEmitterSkinnedMeshShape::create(const PARTICLE_MESH_SHAPE_DESC& desc)
	{
		// TODO - Only support TRIANGLE_LIST draw operation
		// TODO - Ensure mesh-data at least has position (with correct type)
		// TODO - Ensure mesh data has bone weights and indices (with correct types)

		ParticleEmitterSkinnedMeshShape* output = bs_new<ParticleEmitterSkinnedMeshShape>(desc);
		return bs_unique_ptr<ParticleEmitterShape>(output);
	}

	RTTITypeBase* ParticleEmitterSkinnedMeshShape::getRTTIStatic()
	{
		return ParticleEmitterSkinnedMeshShapeRTTI::instance();
	}

	RTTITypeBase* ParticleEmitterSkinnedMeshShape::getRTTI() const
	{
		return getRTTIStatic();
	}

	ParticleEmitter::ParticleEmitter()
		:mShape(bs_unique_ptr<ParticleEmitterShape>(nullptr))
	{ }

	void ParticleEmitter::spawn(Random& random, const ParticleSystemState& state, ParticleSet& set) const
	{
		if(!mShape)
			return;

		const float t = state.time / state.length;
		const float rate = mEmissionRate.evaluate(t, random);

		mEmitAccumulator += rate * state.timeStep;
		auto numToSpawn = (UINT32)mEmitAccumulator;
		mEmitAccumulator -= (float)numToSpawn;

		const UINT32 numPartices = set.getParticleCount() + numToSpawn;
		if(numPartices > state.maxParticles)
			numToSpawn = state.maxParticles - set.getParticleCount();

		const UINT32 firstIdx = mShape->spawn(random, set, numToSpawn, state);
		const UINT32 endIdx = firstIdx + numToSpawn;

		ParticleSetData& particles = set.getParticles();

		for(UINT32 i = firstIdx; i < endIdx; i++)
		{
			const float lifetime = mInitialLifetime.evaluate(t, random);

			particles.initialLifetime[i] = lifetime;
			particles.lifetime[i] = lifetime;
		}

		for(UINT32 i = firstIdx; i < endIdx; i++)
			particles.velocity[i] *= mInitialSpeed.evaluate(t, random);

		if(!mUse3DSize)
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
			{
				const float size = mInitialSize.evaluate(t, random);

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
				Vector3 size = mInitialSize3D.evaluate(t, random);

				// Encode UV flip in size XY as sign
				size.x *= random.getUNorm() < mFlipU ? -1.0f : 1.0f;
				size.y *= random.getUNorm() < mFlipV ? -1.0f : 1.0f;

				particles.size[i] = size;
			}
		}

		if(!mUse3DRotation)
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
			{
				const float rotation = mInitialRotation.evaluate(t, random);
				particles.rotation[i] = Vector3(rotation, 0.0f, 0.0f);
			}
		}
		else
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
			{
				const Vector3 rotation = mInitialRotation3D.evaluate(t, random);
				particles.rotation[i] = rotation;
			}
		}

		for(UINT32 i = firstIdx; i < endIdx; i++)
			particles.color[i] = mInitialColor.evaluate(t, random);

		for(UINT32 i = firstIdx; i < endIdx; i++)
			particles.seed[i] = random.get();

		for(UINT32 i = firstIdx; i < endIdx; i++)
			particles.frame[i] = 0.0f;

		// If in world-space we apply the transform here, otherwise we apply it in the rendering code
		if(state.worldSpace)
		{
			for (UINT32 i = firstIdx; i < endIdx; i++)
				particles.position[i] = state.localToWorld.multiplyAffine(particles.position[i]);
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
