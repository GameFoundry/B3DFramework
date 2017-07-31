//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLightProbes.h"
#include "BsLightProbeVolume.h"
#include "BsGpuBuffer.h"
#include "BsRendererView.h"
#include "BsRenderBeastIBLUtility.h"
#include "BsRendererManager.h"
#include "BsRenderBeast.h"

namespace bs { namespace ct 
{
	LightProbes::LightProbes()
		:mTetrahedronVolumeDirty(false), mNumAllocatedEntries(0), mNumUsedEntries(0)
	{
		resizeCoefficientBuffer(512);
	}

	void LightProbes::notifyAdded(const SPtr<LightProbeVolume>& volume)
	{
		UINT32 handle = (UINT32)mVolumes.size();

		VolumeInfo info;
		info.volume = volume;
		info.isDirty = true;
		info.lastUpdatedProbe = 0;

		mVolumes.push_back(info);
		volume->setRendererId(handle);

		notifyDirty(volume);
	}

	void LightProbes::notifyDirty(const SPtr<LightProbeVolume>& volume)
	{
		volume->prune(mEmptyEntries);
		
		UINT32 handle = volume->getRendererId();
		mVolumes[handle].isDirty = true;
		mVolumes[handle].lastUpdatedProbe = 0;

		mTetrahedronVolumeDirty = true;
	}

	void LightProbes::notifyRemoved(const SPtr<LightProbeVolume>& volume)
	{
		volume->prune(mEmptyEntries, true);

		UINT32 handle = volume->getRendererId();

		LightProbeVolume* lastVolume = mVolumes.back().volume.get();
		UINT32 lastHandle = lastVolume->getRendererId();
		
		if (handle != lastHandle)
		{
			// Swap current last element with the one we want to erase
			std::swap(mVolumes[handle], mVolumes[lastHandle]);
			lastVolume->setRendererId(handle);
		}
		
		// Erase last (empty) element
		mVolumes.erase(mVolumes.end() - 1);

		mTetrahedronVolumeDirty = true;
	}

	void LightProbes::updateProbes(const FrameInfo& frameInfo, UINT32 maxProbes)
	{
		if(mTetrahedronVolumeDirty)
		{
			// Gather all positions
			for(auto& entry : mVolumes)
			{
				const Vector<Vector3>& positions = entry.volume->getLightProbePositions();
				
				Vector3 offset = entry.volume->getPosition();
				Quaternion rotation = entry.volume->getRotation();
				for(auto& localPos : positions)
				{
					Vector3 transformedPos = rotation.rotate(localPos) + offset;
					mTempTetrahedronPositions.push_back(transformedPos);
				}
			}

			mTetrahedronInfos.clear();
			mTetrahedronBounds.clear();

			generateTetrahedronData(mTempTetrahedronPositions, mTetrahedronInfos, false);

			// Generate bounds
			for(auto& entry : mTetrahedronInfos)
			{
				// Skipping outer faces
				if (entry.volume.neighbors[3] < 0)
					continue;

				AABox aabox = AABox(Vector3::INF, -Vector3::INF);
				for (int i = 0; i < 4; ++i)
					aabox.merge(mTempTetrahedronPositions[entry.volume.neighbors[i]]);

				mTetrahedronBounds.push_back(aabox);
			}

			mTempTetrahedronPositions.clear();
			mTetrahedronVolumeDirty = false;
		}

		// Render dirty probes
		UINT32 numProbeUpdates = 0;
		for(auto& entry : mVolumes)
		{
			if (!entry.isDirty)
				continue;

			Vector<LightProbeInfo>& probes = entry.volume->getLightProbeInfos();
			const Vector<Vector3>& probePositions = entry.volume->getLightProbePositions();
			for (; entry.lastUpdatedProbe < (UINT32)probes.size(); ++entry.lastUpdatedProbe)
			{
				LightProbeInfo& probeInfo = probes[entry.lastUpdatedProbe];

				// Assign buffer idx, if not assigned
				if(probeInfo.bufferIdx == -1)
				{
					if(!mEmptyEntries.empty())
					{
						probeInfo.bufferIdx = mEmptyEntries.back();
						mEmptyEntries.erase(mEmptyEntries.end() - 1);
					}
					else
					{
						if(mNumUsedEntries >= mNumAllocatedEntries)
							resizeCoefficientBuffer(mNumAllocatedEntries * 2);

						probeInfo.bufferIdx = mNumUsedEntries++;
					}
				}

				if(probeInfo.flags == LightProbeFlags::Dirty)
				{
					TEXTURE_DESC cubemapDesc;
					cubemapDesc.type = TEX_TYPE_CUBE_MAP;
					cubemapDesc.format = PF_FLOAT16_RGB;
					cubemapDesc.width = IBLUtility::IRRADIANCE_CUBEMAP_SIZE;
					cubemapDesc.height = IBLUtility::REFLECTION_CUBEMAP_SIZE;
					cubemapDesc.usage = TU_STATIC | TU_RENDERTARGET;

					SPtr<Texture> cubemap = Texture::create(cubemapDesc);

					RenderBeast& renderer = static_cast<RenderBeast&>(*RendererManager::instance().getActive());
					renderer.captureSceneCubeMap(cubemap, probePositions[entry.lastUpdatedProbe], true);

					gIBLUtility().filterCubemapForIrradiance(cubemap, mProbeCoefficientsGPU, probeInfo.bufferIdx);

					probeInfo.flags = LightProbeFlags::Clean;
					numProbeUpdates++;
				}

				if (maxProbes != 0 && numProbeUpdates >= numProbeUpdates)
					break;
			}

			if (entry.lastUpdatedProbe == (UINT32)probes.size())
				entry.isDirty = false;
		}
	}

	void LightProbes::resizeTetrahedronBuffers(VisibleLightProbeData& data, UINT32 count)
	{
		{
			GPU_BUFFER_DESC desc;
			desc.type = GBT_STRUCTURED;
			desc.elementSize = sizeof(TetrahedronBoundsGPU);
			desc.elementCount = count;
			desc.usage = GBU_STATIC;
			desc.format = BF_UNKNOWN;

			SPtr<GpuBuffer> newBuffer = GpuBuffer::create(desc);
			if (data.tetrahedronBounds)
				newBuffer->copyData(*data.tetrahedronBounds, 0, 0, data.tetrahedronBounds->getSize(), true);

			data.tetrahedronBounds = newBuffer;
		}

		{
			GPU_BUFFER_DESC desc;
			desc.type = GBT_STRUCTURED;
			desc.elementSize = sizeof(TetrahedronDataGPU);
			desc.elementCount = count;
			desc.usage = GBU_STATIC;
			desc.format = BF_UNKNOWN;

			SPtr<GpuBuffer> newBuffer = GpuBuffer::create(desc);
			if (data.tetrahedronInfos)
				newBuffer->copyData(*data.tetrahedronInfos, 0, 0, data.tetrahedronInfos->getSize(), true);

			data.tetrahedronInfos = newBuffer;
		}

		data.maxNumEntries = count;
	}

	void LightProbes::resizeCoefficientBuffer(UINT32 count)
	{
		GPU_BUFFER_DESC desc;
		desc.type = GBT_STRUCTURED;
		desc.elementSize = sizeof(SHVector3RGB);
		desc.elementCount = count;
		desc.usage = GBU_STATIC;
		desc.format = BF_UNKNOWN;

		SPtr<GpuBuffer> newBuffer = GpuBuffer::create(desc);
		if (mProbeCoefficientsGPU)
			newBuffer->copyData(*mProbeCoefficientsGPU, 0, 0, mProbeCoefficientsGPU->getSize(), true);

		mProbeCoefficientsGPU = newBuffer;
		mNumAllocatedEntries = count;
	}

	void LightProbes::updateVisibleProbes(const RendererView& view, VisibleLightProbeData& output)
	{
		// Ignore all probes past this point
		static const float MAX_PROBE_DISTANCE = 100.0f;

		const RendererViewProperties& viewProps = view.getProperties();
		const ConvexVolume& worldFrustum = viewProps.cullFrustum;

		const float maxProbeDistance2 = MAX_PROBE_DISTANCE * MAX_PROBE_DISTANCE;
		for (UINT32 i = 0; i < (UINT32)mTetrahedronBounds.size(); i++)
		{
			float distance2 = viewProps.viewOrigin.squaredDistance(mTetrahedronBounds[i].getCenter());
			if (distance2 > maxProbeDistance2)
				continue;

			if (worldFrustum.intersects(mTetrahedronBounds[i]))
				mTempTetrahedronVisibility.push_back(i);
		}

		UINT32 numVisibleTets = (UINT32)mTempTetrahedronVisibility.size();
		if (numVisibleTets > output.maxNumEntries)
		{
			UINT32 newBufferSize = 256;
			if(output.maxNumEntries > 0)
				newBufferSize = Math::divideAndRoundUp(numVisibleTets, output.maxNumEntries) * output.maxNumEntries;

			resizeTetrahedronBuffers(output, newBufferSize);
		}

		// Write bounds
		{
			TetrahedronBoundsGPU* dst = (TetrahedronBoundsGPU*)output.tetrahedronBounds->lock(0, 
				output.tetrahedronBounds->getSize(), GBL_WRITE_ONLY_DISCARD);

			for (auto& entry : mTempTetrahedronVisibility)
			{
				const AABox& aabox = mTetrahedronBounds[entry];

				dst->center = aabox.getCenter();
				dst->extents = aabox.getHalfSize();

				dst++;
			}

			output.tetrahedronBounds->unlock();
		}

		// Write other information
		{
			TetrahedronDataGPU* dst = (TetrahedronDataGPU*)output.tetrahedronInfos->lock(0, 
				output.tetrahedronInfos->getSize(), GBL_WRITE_ONLY_DISCARD);

			for (auto& entry : mTempTetrahedronVisibility)
			{
				const TetrahedronData& data = mTetrahedronInfos[entry];

				memcpy(dst->indices, data.volume.vertices, sizeof(UINT32) * 4);
				memcpy(&dst->transform, &data.transform, sizeof(float) * 12);

				dst++;
			}

			output.tetrahedronBounds->unlock();
		}

		mTempTetrahedronVisibility.clear();
	}

	/** Hash value generator for std::pair<INT32, INT32>. */
	struct pair_hash
	{
		size_t operator()(const std::pair<INT32, INT32>& key) const
		{
			size_t hash = 0;
			bs::hash_combine(hash, key.first);
			bs::hash_combine(hash, key.second);

			return hash;
		}
	};

	void LightProbes::generateTetrahedronData(const Vector<Vector3>& positions, Vector<TetrahedronData>& output, 
		bool includeOuterFaces)
	{
		bs_frame_mark();
		{
			TetrahedronVolume volume = Triangulation::tetrahedralize(positions);

			// Generate matrices
			UINT32 numOutputTets = (UINT32)volume.tetrahedra.size();
			if (includeOuterFaces)
				numOutputTets += (UINT32)volume.outerFaces.size();

			output.reserve(includeOuterFaces);

			// Insert innert tetrahedrons, generate matrices
			for(UINT32 i = 0; i < (UINT32)volume.tetrahedra.size(); ++i)
			{
				TetrahedronData entry;
				entry.volume = volume.tetrahedra[i];

				// Generate a matrix that can be used for calculating barycentric coordinates
				// To determine a point within a tetrahedron, using barycentric coordinates, we use:
				// P = (P1 - P4) * a + (P2 - P4) * b + (P3 - P4) * c + P4
				//
				// Where P1, P2, P3, P4 are the corners of the tetrahedron.
				//
				// Expanded for each coordinate this is:
				// x = (x1 - x4) * a + (x2 - x4) * b + (x3 - x4) * c + x4
				// y = (y1 - y4) * a + (y2 - y4) * b + (y3 - y4) * c + y4
				// z = (z1 - z4) * a + (z2 - z4) * b + (z3 - z4) * c + z4
				//
				// In matrix form this is:
				//                                      a
				// P = [P1 - P4, P2 - P4, P3 - P4, P4] [b]
				//                                      c
				//                                      1
				//
				// Solved for barycentric coordinates:
				//  a
				// [b] = Minv * P 
				//  c
				//  1
				//
				// Where Minv is the inverse of the matrix above.

				const Vector3& P1 = positions[volume.tetrahedra[i].vertices[0]];
				const Vector3& P2 = positions[volume.tetrahedra[i].vertices[1]];
				const Vector3& P3 = positions[volume.tetrahedra[i].vertices[2]];
				const Vector3& P4 = positions[volume.tetrahedra[i].vertices[3]];

				Matrix4 mat;
				mat.setColumn(0, Vector4(P1 - P4, 0.0f));
				mat.setColumn(1, Vector4(P2 - P4, 0.0f));
				mat.setColumn(2, Vector4(P3 - P4, 0.0f));
				mat.setColumn(3, Vector4(P4, 1.0f));

				entry.transform = mat.inverse();

				output.push_back(entry);
			}

			if (includeOuterFaces)
			{
				// Put outer faces into the Tetrahedron structure, for convenience
				UINT32 outerFaceOffset = (UINT32)volume.tetrahedra.size();
				FrameVector<Tetrahedron> outerTetrahedrons;

				outerTetrahedrons.resize(volume.outerFaces.size());

				for (UINT32 i = 0; i < (UINT32)volume.outerFaces.size(); ++i)
				{
					Tetrahedron outerTetrahedron;
					memcpy(outerTetrahedron.vertices, volume.outerFaces[i].vertices, sizeof(INT32) * 3);
					memset(outerTetrahedron.neighbors, -1, sizeof(INT32) * 3);

					outerTetrahedron.vertices[4] = -1; // Marks the tetrahedron as an outer face
					outerTetrahedron.neighbors[4] = volume.outerFaces[i].tetrahedron;

					outerTetrahedrons[i] = outerTetrahedron;
				}

				// Connect boundary tetrahedrons with these new outer tetrahedrons
				for (UINT32 i = 0; i < (UINT32)volume.outerFaces.size(); ++i)
				{
					Tetrahedron& tet = volume.tetrahedra[volume.outerFaces[i].tetrahedron];
					for (UINT32 j = 0; j < 4; j++)
					{
						if (tet.neighbors[j] == -1)
							tet.neighbors[j] = outerFaceOffset + i;
					}
				}

				// Make a map between outer edges and faces, used in the following algorithms
				struct Edge
				{
					INT32 faces[2];
					INT32 oppositeVerts[2];
				};

				FrameUnorderedMap<std::pair<INT32, INT32>, Edge, pair_hash> edgeMap;
				for (UINT32 i = 0; i < (UINT32)volume.outerFaces.size(); ++i)
				{
					for (UINT32 j = 0; j < 3; ++j)
					{
						INT32 v0 = volume.outerFaces[i].vertices[j];
						INT32 v1 = volume.outerFaces[i].vertices[(j + 1) % 3];

						// Keep the same ordering so other faces can find the same edge
						if (v0 > v1)
							std::swap(v0, v1);

						auto iterFind = edgeMap.find(std::make_pair(v0, v1));
						if (iterFind != edgeMap.end())
						{
							iterFind->second.faces[1] = i;
							iterFind->second.oppositeVerts[1] = (j + 2) % 3;
						}
						else
						{
							Edge edge;
							edge.faces[0] = i;
							edge.oppositeVerts[0] = (j + 2) % 3;

							edgeMap.insert(std::make_pair(std::make_pair(v0, v1), edge));
						}
					}
				}

				// Form connections between outer tetrahedrons
				for (auto& entry : edgeMap)
				{
					const Edge& edge = entry.second;

					Tetrahedron& tet0 = outerTetrahedrons[outerFaceOffset + edge.faces[0]];
					tet0.neighbors[edge.oppositeVerts[0]] = outerFaceOffset + edge.faces[1];

					Tetrahedron& tet1 = outerTetrahedrons[outerFaceOffset + edge.faces[1]];
					tet1.neighbors[edge.oppositeVerts[1]] = outerFaceOffset + edge.faces[0];
				}

				// Generate face normals
				FrameVector<Vector3> faceNormals(volume.outerFaces.size());
				for (UINT32 i = 0; i < (UINT32)volume.outerFaces.size(); ++i)
				{
					const Vector3& v0 = positions[volume.outerFaces[i].vertices[0]];
					const Vector3& v1 = positions[volume.outerFaces[i].vertices[1]];
					const Vector3& v2 = positions[volume.outerFaces[i].vertices[2]];
					
					Vector3 e0 = v1 - v0;
					Vector3 e1 = v2 - v0;

					faceNormals[i] = Vector3::normalize(e1.cross(e0));
				}

				// Generate vertex normals
				struct VertexAccum
				{
					Vector3 sum;
					float weight;
				};

				FrameUnorderedMap<INT32, Vector3> vertexNormals;
				for (auto& entry : edgeMap)
				{
					const Edge& edge = entry.second;

					auto accumulateNormalForEdgeVertex = [&](UINT32 v0Idx, UINT32 v1Idx)
					{
						auto iter = vertexNormals.insert(std::make_pair(v0Idx, Vector3(BsZero)));

						Vector3& accum = iter.first->second;
						const Vector3& v0 = positions[v0Idx];

						auto accumulateNormalForFace = [&](INT32 faceIdx, INT32 v2LocIdx)
						{
							const TetrahedronFace& face = volume.outerFaces[faceIdx];

							// Vertices on the face, that aren't the vertex we're calculating the normal for
							const Vector3& v1 = positions[v1Idx];
							const Vector3& v2 = positions[face.vertices[v2LocIdx]];

							// Weight the contribution to the normal based on the angle spanned by the triangle
							Vector3 e0 = Vector3::normalize(v1 - v0);
							Vector3 e1 = Vector3::normalize(v2 - v0);

							float weight = acos(e0.dot(e1));
							accum += weight * faceNormals[faceIdx];
						};

						accumulateNormalForFace(edge.faces[0], entry.second.oppositeVerts[0]);
						accumulateNormalForFace(edge.faces[1], entry.second.oppositeVerts[1]);
					};

					accumulateNormalForEdgeVertex(entry.first.first, entry.first.second);
					accumulateNormalForEdgeVertex(entry.first.second, entry.first.first);
				}

				for (auto& entry : vertexNormals)
					entry.second.normalize();

				// Insert outer tetrahedrons, generate matrices
				for(UINT32 i = 0; i < (UINT32)outerTetrahedrons.size(); ++i)
				{
					TetrahedronData entry;
					entry.volume = outerTetrahedrons[i];

					// We need a way to project a point outside the tetrahedron volume onto an outer face, then calculate
					// triangle's barycentric coordinates. Use use the per-vertex normals to extrude the triangle face into
					// infinity.

					// Our point can be represented as:
					// p == a (p0 + t*v0) + b (p1 + t*v1) + c (p2 + t*v2)
					//
					// where a, b and c are barycentric coordinates,
					// p0, p1, p2 are the corners of the face
					// v0, v1, v2 are the vertex normals, per corner
					// t is the distance from the triangle to the point
					//
					// Essentially we're calculating the corners of a bigger triangle that's "t" units away from the
					// face, and its corners lie along the per-vertex normals. Point "p" will lie on that triangle, for which
					// we can then calculate barycentric coordinates normally.
					//
					// First we substitute: c = 1 - a - b
					// p == a (p0 + t v0) + b (p1 + t v1) + (1 - a - b) (p2 + t v2)
					// p == a (p0 + t v0) + b (p1 + t v1) + (p2 + t v2) - a (p2 + t v2) - b (p2 + t v2)
					// p == a (p0 - p2 + t v0 - t v2) + b (p1 - p2 + t v1 - t v2) + (p2 + t v2)
					//
					// And move everything to one side:
					// p - p2 - t v2 == a (p0 - p2 + t ( v0 - v2)) + b (p1 - p2 + t ( v1 - v2))
					// a (p0 - p2 + t ( v0 - v2)) + b (p1 - p2 + t ( v1 - v2)) - (p - p2 - t v2) == 0
					//
					// We rewrite it using:
					// Ap = p0 - p2
					// Av = v0 - v2
					// Bp = p1 - p2
					// Bv = v1 - v2
					// Cp = p - p2
					// Cv = -v2
					//
					// Which yields:
					// a (Ap + t Av) + b (Bp + t Bv) - (Cp + t Cv) == 0
					//
					// Which can be written in matrix form:
					//
					// M = {Ap + t Av, Bp + t Bv, Cp + t Cv}
					//       a      0
					// M * [ b ] = [0]
					//      -1      0
					//
					// From that we can tell that matrix M cannot be inverted, because if we multiply the zero vector with the
					// inverted matrix the result would be zero, and not [a, b, -1]. Since the matrix cannot be inverted
					// det(M) == 0.
					//
					// We can use that fact to calculate "t". After we have "t" we can calculate barycentric coordinates
					// normally.
					//
					// Solving equation det(M) == 0 yields a cubic in form:
					// p t^3 + q t^2 + r t + s = 0
					//
					// We'll convert this to monic form, by dividing by p:
					// t^3 + q/p t^2 + r/p t + s/p = 0
					//
					// Or if p ends up being zero, we end up with a quadratic instead:
					// q t^2 + r t + s = 0
					// 
					// We want to create a matrix that when multiplied with the position, yields us the three coefficients,
					// which we can then use to solve for "t". For this we create a 4x3 matrix, where each row represents
					// a solution for one of the coefficients. We factor contributons to each coefficient whether they depend on
					// position x, y, z, or don't depend on position (row columns, in that order respectively).

					const Vector3& p0 = positions[entry.volume.vertices[0]];
					const Vector3& p1 = positions[entry.volume.vertices[1]];
					const Vector3& p2 = positions[entry.volume.vertices[2]];

					const Vector3& v0 = vertexNormals[entry.volume.vertices[0]];
					const Vector3& v1 = vertexNormals[entry.volume.vertices[1]];
					const Vector3& v2 = vertexNormals[entry.volume.vertices[2]];

					float p =
						v2.x * v1.y * v0.z -
						v1.x * v2.y * v0.z -
						v2.x * v0.y * v1.z +
						v0.x * v2.y * v1.z +
						v1.x * v0.y * v2.z -
						v0.x * v1.y * v2.z;
					
					float qx = -v1.y * v0.z + v2.y * v0.z + v0.y * v1.z - v2.y * v1.z - v0.y * v2.z + v1.y * v2.z;
					float qy = v1.x * v0.z - v2.x * v0.z - v0.x * v1.z + v2.x * v1.z + v0.x * v2.z - v1.x * v2.z;
					float qz = -v1.x * v0.y + v2.x * v0.y + v0.x * v1.y - v2.x * v1.y - v0.x * v2.y + v1.x * v2.y;
					float qw = v2.y * v1.z * p0.x - v1.y * v2.z * p0.x - v2.y * v0.z * p1.x + v0.y * v2.z * p1.x + 
						v1.y * v0.z * p2.x - v0.y * v1.z * p2.x - v2.x * v1.z * p0.y + v1.x * v2.z * p0.y + 
						v2.x * v0.z * p1.y - v0.x * v2.z * p1.y - v1.x * v0.z * p2.y + v0.x * v1.z * p2.y + 
						v2.x * v1.y * p0.z - v1.x * v2.y * p0.z - v2.x * v0.y * p1.z + v0.x * v2.y * p1.z + 
						v1.x * v0.y * p2.z - v0.x * v1.y * p2.z;

					float rx = v1.z * p0.y - v2.z * p0.y - v0.z * p1.y + v2.z * p1.y + v0.z * p2.y - v1.z * p2.y -
						v1.y * p0.z + v2.y * p0.z + v0.y * p1.z - v2.y * p1.z - v0.y * p2.z + v1.y * p2.z;
					float ry = -v1.z * p0.x + v2.z * p0.x + v0.z * p1.x - v2.z * p1.x - v0.z * p2.x + v1.z * p2.x +
						v1.x * p0.z - v2.x * p0.z - v0.x * p1.z + v2.x * p1.z + v0.x * p2.z - v1.x * p2.z;
					float rz = v1.y * p0.x - v2.y * p0.x - v0.y * p1.x + v2.y * p1.x + v0.y * p2.x - v1.y * p2.x -
						v1.x * p0.y + v2.x * p0.y + v0.x * p1.y - v2.x * p1.y - v0.x * p2.y + v1.x * p2.y;
					float rw = v2.z * p1.x * p0.y - v1.z * p2.x * p0.y - v2.z * p0.x * p1.y + v0.z * p2.x * p1.y +
						v1.z * p0.x * p2.y - v0.z * p1.x * p2.y - v2.y * p1.x * p0.z + v1.y * p2.x * p0.z +
						v2.x * p1.y * p0.z - v1.x * p2.y * p0.z + v2.y * p0.x * p1.z - v0.y * p2.x * p1.z -
						v2.x * p0.y * p1.z + v0.x * p2.y * p1.z - v1.y * p0.x * p2.z + v0.y * p1.x * p2.z +
						v1.x * p0.y * p2.z - v0.x * p1.y * p2.z;

					float sx = -p1.y * p0.z + p2.y * p0.z + p0.y * p1.z - p2.y * p1.z - p0.y * p2.z + p1.y * p2.z;
					float sy = p1.x * p0.z - p2.x * p0.z - p0.x * p1.z + p2.x * p1.z + p0.x * p2.z - p1.x * p2.z;
					float sz = -p1.x * p0.y + p2.x * p0.y + p0.x * p1.y - p2.x * p1.y - p0.x * p2.y + p1.x * p2.y;
					float sw = p2.x * p1.y * p0.z - p1.x * p2.y * p0.z - p2.x * p0.y * p1.z + 
						p0.x * p2.y * p1.z + p1.x * p0.y * p2.z - p0.x * p1.y * p2.z;

					entry.transform[0][0] = qx;
					entry.transform[0][1] = qy;
					entry.transform[0][2] = qz;
					entry.transform[0][3] = qw;

					entry.transform[1][0] = rx;
					entry.transform[1][1] = ry;
					entry.transform[1][2] = rz;
					entry.transform[1][3] = rw;

					entry.transform[2][0] = sx;
					entry.transform[2][1] = sy;
					entry.transform[2][2] = sz;
					entry.transform[2][3] = sw;

					// Unused
					entry.transform[3][0] = 0.0f;
					entry.transform[3][1] = 0.0f;
					entry.transform[3][2] = 0.0f;
					entry.transform[3][3] = 0.0f;

					if (fabs(p) > 0.00001f)
						entry.transform = entry.transform * (1.0f / p);
					else // Quadratic
						entry.volume.neighbors[3] = -2;

					output.push_back(entry);
				}
			}
		}
		bs_frame_clear();
	}
}}
