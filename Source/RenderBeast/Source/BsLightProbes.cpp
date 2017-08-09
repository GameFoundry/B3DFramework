//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLightProbes.h"
#include "BsLightProbeVolume.h"
#include "BsGpuBuffer.h"
#include "BsRendererView.h"
#include "BsRenderBeastIBLUtility.h"
#include "BsMesh.h"
#include "BsVertexDataDesc.h"
#include "BsGpuParamsSet.h"
#include "BsRendererUtility.h"
#include "BsSkybox.h"
#include "BsRendererTextures.h"

namespace bs { namespace ct 
{
	TetrahedraRenderParamDef gTetrahedraRenderParamDef;

	ShaderVariation TetrahedraRenderMat::VAR_NoMSAA = ShaderVariation({
		ShaderVariation::Param("MSAA", false)
	});

	ShaderVariation TetrahedraRenderMat::VAR_MSAA = ShaderVariation({
		ShaderVariation::Param("MSAA", true)
	});

	TetrahedraRenderMat::TetrahedraRenderMat()
	{
		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", mDepthBufferTex);

		if(params->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp"))
		{
			SAMPLER_STATE_DESC pointSampDesc;
			pointSampDesc.minFilter = FO_POINT;
			pointSampDesc.magFilter = FO_POINT;
			pointSampDesc.mipFilter = FO_POINT;
			pointSampDesc.addressMode.u = TAM_CLAMP;
			pointSampDesc.addressMode.v = TAM_CLAMP;
			pointSampDesc.addressMode.w = TAM_CLAMP;

			SPtr<SamplerState> pointSampState = SamplerState::create(pointSampDesc);
			params->setSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp", pointSampState);
		}

		mParamBuffer = gTetrahedraRenderParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Params", mParamBuffer, true);
	}

	void TetrahedraRenderMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_NoMSAA);
		variations.add(VAR_MSAA);
	}

	void TetrahedraRenderMat::execute(const RendererView& view, const SPtr<Texture>& sceneDepth, const SPtr<Mesh>& mesh, 
		const SPtr<RenderTexture>& output)
	{
		const RendererViewProperties& viewProps = view.getProperties();

		Vector4 NDCtoUV = view.getNDCToUV();
		if(mVariation.getBool("MSAA"))
		{
			NDCtoUV.x *= viewProps.viewRect.width;
			NDCtoUV.y *= viewProps.viewRect.height;
			NDCtoUV.z *= viewProps.viewRect.width;
			NDCtoUV.w *= viewProps.viewRect.height;
		}

		gTetrahedraRenderParamDef.gMatViewProj.set(mParamBuffer, viewProps.viewProjTransform);
		gTetrahedraRenderParamDef.gNDCToUV.set(mParamBuffer, NDCtoUV);
		gTetrahedraRenderParamDef.gNDCToDeviceZ.set(mParamBuffer, RendererView::getNDCZToDeviceZ());
		
		mDepthBufferTex.set(sceneDepth);
		mParamBuffer->flushToGPU();

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);
		gRendererUtility().draw(mesh);
	}

	void TetrahedraRenderMat::getOutputDesc(const RendererView& view, POOLED_RENDER_TEXTURE_DESC& colorDesc, 
		POOLED_RENDER_TEXTURE_DESC& depthDesc)
	{
		const RendererViewProperties& viewProps = view.getProperties();
		UINT32 width = viewProps.viewRect.width;
		UINT32 height = viewProps.viewRect.height;
		UINT32 numSamples = viewProps.numSamples;

		colorDesc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_R16U, width, height, TU_RENDERTARGET, numSamples);
		depthDesc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_D16, width, height, TU_DEPTHSTENCIL, numSamples);
	}

	TetrahedraRenderMat* TetrahedraRenderMat::getVariation(bool msaa)
	{
		if (msaa)
			return get(VAR_MSAA);

		return get(VAR_NoMSAA);
	}

	IrradianceEvaluateParamDef gIrradianceEvaluateParamDef;

	ShaderVariation IrradianceEvaluateMat::VAR_MSAA_Probes = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("SKY_ONLY", false)
	});

	ShaderVariation IrradianceEvaluateMat::VAR_NoMSAA_Probes = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 1),
		ShaderVariation::Param("SKY_ONLY", false)
	});

	ShaderVariation IrradianceEvaluateMat::VAR_MSAA_Sky = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 2),
		ShaderVariation::Param("SKY_ONLY", true)
	});

	ShaderVariation IrradianceEvaluateMat::VAR_NoMSAA_Sky = ShaderVariation({
		ShaderVariation::Param("MSAA_COUNT", 1),
		ShaderVariation::Param("SKY_ONLY", true)
	});

	IrradianceEvaluateMat::IrradianceEvaluateMat()
		:mGBufferParams(mMaterial, mParamsSet)
	{
		mSkyOnly = mVariation.getBool("SKY_ONLY");

		SPtr<GpuParams> params = mParamsSet->getGpuParams();

		if(mSkyOnly)
			params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSkyIrradianceTex", mParamSkyIrradianceTex);
		else
		{
			params->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mParamInputTex);
			params->getBufferParam(GPT_FRAGMENT_PROGRAM, "gSHCoeffs", mParamSHCoeffsBuffer);
			params->getBufferParam(GPT_FRAGMENT_PROGRAM, "gProbeVolumes", mParamVolumeBuffer);
		}

		mParamBuffer = gIrradianceEvaluateParamDef.createBuffer();
		mParamsSet->setParamBlockBuffer("Params", mParamBuffer, true);
	}

	void IrradianceEvaluateMat::_initVariations(ShaderVariations& variations)
	{
		variations.add(VAR_MSAA_Probes);
		variations.add(VAR_MSAA_Sky);
		variations.add(VAR_NoMSAA_Probes);
		variations.add(VAR_NoMSAA_Sky);
	}

	void IrradianceEvaluateMat::execute(const RendererView& view, const GBufferTextures& gbuffer, 
		const SPtr<Texture>& lightProbeIndices, const SPtr<GpuBuffer>& shCoeffs, const SPtr<GpuBuffer>& volumes, 
		const Skybox* skybox, const SPtr<RenderTexture>& output)
	{
		const RendererViewProperties& viewProps = view.getProperties();

		mGBufferParams.bind(gbuffer);

		float skyBrightness = 1.0f;
		if (mSkyOnly)
		{
			SPtr<Texture> skyIrradiance;
			if (skybox != nullptr)
			{
				skyIrradiance = skybox->getIrradiance();
				skyBrightness = skybox->getBrightness();
			}

			if(skyIrradiance == nullptr)
				skyIrradiance = RendererTextures::defaultIndirect;

			mParamSkyIrradianceTex.set(skyIrradiance);
		}
		else
		{
			mParamInputTex.set(lightProbeIndices);
			mParamSHCoeffsBuffer.set(shCoeffs);
			mParamVolumeBuffer.set(volumes);
		}

		gIrradianceEvaluateParamDef.gSkyBrightness.set(mParamBuffer, skyBrightness);
		mParamBuffer->flushToGPU();

		mParamsSet->setParamBlockBuffer("PerCamera", view.getPerViewBuffer(), true);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output, 0, RT_COLOR0);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParamsSet);

		gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)viewProps.viewRect.width, 
			(float)viewProps.viewRect.height));

		rapi.setRenderTarget(nullptr);
	}

	IrradianceEvaluateMat* IrradianceEvaluateMat::getVariation(UINT32 msaaCount, bool skyOnly)
	{
		if(skyOnly)
		{
			if (msaaCount > 1)
				return get(VAR_MSAA_Sky);

			return get(VAR_NoMSAA_Sky);
		}
		else
		{
			if (msaaCount > 1)
				return get(VAR_MSAA_Probes);

			return get(VAR_NoMSAA_Probes);
		}
	}

	LightProbes::LightProbes()
		:mTetrahedronVolumeDirty(false), mMaxCoefficients(0), mMaxTetrahedra(0)
	{ }

	void LightProbes::notifyAdded(LightProbeVolume* volume)
	{
		UINT32 handle = (UINT32)mVolumes.size();

		VolumeInfo info;
		info.volume = volume;
		info.isDirty = true;

		mVolumes.push_back(info);
		volume->setRendererId(handle);

		notifyDirty(volume);
	}

	void LightProbes::notifyDirty(LightProbeVolume* volume)
	{
		UINT32 handle = volume->getRendererId();
		mVolumes[handle].isDirty = true;

		mTetrahedronVolumeDirty = true;
	}

	void LightProbes::notifyRemoved(LightProbeVolume* volume)
	{
		UINT32 handle = volume->getRendererId();

		LightProbeVolume* lastVolume = mVolumes.back().volume;
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

	void LightProbes::updateProbes()
	{
		if (!mTetrahedronVolumeDirty)
			return;

		// Move all coefficients into the global buffer
		UINT32 numCoeffs = 0;
		for(auto& entry : mVolumes)
		{
			UINT32 numProbes = (UINT32)entry.volume->getLightProbePositions().size();
			numCoeffs += numProbes;
		}

		if(numCoeffs > mMaxCoefficients)
		{
			UINT32 newSize = Math::divideAndRoundUp(numCoeffs, 32U) * 32U;
			resizeCoefficientBuffer(newSize);
		}

		UINT32 writePos = 0;
		for(auto& entry : mVolumes)
		{
			UINT32 numProbes = (UINT32)entry.volume->getLightProbePositions().size();
			UINT32 size = numProbes * sizeof(LightProbeSHCoefficients);
			SPtr<GpuBuffer> localBuffer = entry.volume->getCoefficientsBuffer();
			
			// Note: Some of the coefficients might still be dirty (unrendered). Check for this and write them as black?
			mProbeCoefficientsGPU->copyData(*localBuffer, 0, writePos, size);
			writePos += size;
		}

		// Gather all positions
		UINT32 bufferOffset = 0;
		for(auto& entry : mVolumes)
		{
			const Vector<LightProbeInfo>& infos = entry.volume->getLightProbeInfos();
			const Vector<Vector3>& positions = entry.volume->getLightProbePositions();
			UINT32 numProbes = entry.volume->getNumActiveProbes();
			
			if (numProbes == 0)
				continue;

			Vector3 offset = entry.volume->getPosition();
			Quaternion rotation = entry.volume->getRotation();
			for(UINT32 i = 0; i < numProbes; i++)
			{
				Vector3 localPos = positions[i];
				Vector3 transformedPos = rotation.rotate(localPos) + offset;
				mTempTetrahedronPositions.push_back(transformedPos);
				mTempTetrahedronBufferIndices.push_back(bufferOffset + infos[i].bufferIdx);
			}

			bufferOffset += (UINT32)positions.size();
		}

		mTetrahedronInfos.clear();

		UINT32 innerVertexCount = (UINT32)mTempTetrahedronPositions.size();
		generateTetrahedronData(mTempTetrahedronPositions, mTetrahedronInfos, true);

		// Generate a mesh out of all the tetrahedron triangles
		// Note: Currently the entire volume is rendered as a single large mesh, which will isn't optimal as we can't
		// perform frustum culling. A better option would be to split the mesh into multiple smaller volumes, do
		// frustum culling and possibly even sort by distance from camera.
		UINT32 numTetrahedra = (UINT32)mTetrahedronInfos.size();

		UINT32 numVertices = numTetrahedra * 4 * 3;
		UINT32 numIndices = numTetrahedra * 4 * 3;

		SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDesc->addVertElem(VET_UINT1, VES_TEXCOORD);

		SPtr<MeshData> meshData = MeshData::create(numVertices, numIndices, vertexDesc);
		auto posIter = meshData->getVec3DataIter(VES_POSITION);
		auto idIter = meshData->getDWORDDataIter(VES_TEXCOORD);
		UINT32* indices = meshData->getIndices32();

		UINT32 tetIdx = 0;
		for(auto& entry : mTetrahedronInfos)
		{
			const Tetrahedron& volume = entry.volume;

			Vector3 center(BsZero);
			for(UINT32 i = 0; i < 4; i++)
				center += mTempTetrahedronPositions[volume.vertices[i]];

			center /= 4.0f;

			static const UINT32 Permutations[4][3] = 
			{
				{ 0, 1, 2 },
				{ 0, 1, 3 },
				{ 0, 2, 3 },
				{ 1, 2, 3 }
			};

			for(UINT32 i = 0; i < 4; i++)
			{
				Vector3 A = mTempTetrahedronPositions[volume.vertices[Permutations[i][0]]];
				Vector3 B = mTempTetrahedronPositions[volume.vertices[Permutations[i][1]]];
				Vector3 C = mTempTetrahedronPositions[volume.vertices[Permutations[i][2]]];

				// Make sure the triangle is clockwise
				Vector3 e0 = A - C;
				Vector3 e1 = B - C;

				Vector3 normal = e0.cross(e1);
				if (normal.dot(A - center) < 0.0f)
					std::swap(B, C);

				posIter.addValue(A);
				posIter.addValue(B);
				posIter.addValue(C);

				idIter.addValue(tetIdx);
				idIter.addValue(tetIdx);
				idIter.addValue(tetIdx);

				indices[0] = tetIdx * 4 * 3 + i * 3 + 0;
				indices[1] = tetIdx * 4 * 3 + i * 3 + 1;
				indices[2] = tetIdx * 4 * 3 + i * 3 + 2;

				indices += 3;
			}

			tetIdx++;
		}

		mVolumeMesh = Mesh::create(meshData);

		// Map vertices to actual SH coefficient indices, and write GPU buffer with tetrahedron information
		if (numTetrahedra > mMaxTetrahedra)
		{
			UINT32 newSize = Math::divideAndRoundUp(numTetrahedra, 64U) * 64U;
			resizeTetrahedronBuffer(newSize);
		}

		TetrahedronDataGPU* dst = (TetrahedronDataGPU*)mTetrahedronInfosGPU->lock(0, mTetrahedronInfosGPU->getSize(), 
			GBL_WRITE_ONLY_DISCARD);
		for (auto& entry : mTetrahedronInfos)
		{
			for(UINT32 i = 0; i < 4; ++i)
			{
				// Check for outer vertices, which have no SH data associated with them
				if (entry.volume.vertices[i] >= (INT32)innerVertexCount)
					entry.volume.vertices[i] = -1;
				else
					entry.volume.vertices[i] = mTempTetrahedronBufferIndices[entry.volume.vertices[i]];
			}

			memcpy(dst->indices, entry.volume.vertices, sizeof(UINT32) * 4);
			memcpy(&dst->transform, &entry.transform, sizeof(float) * 12);

			dst++;
		}

		mTetrahedronInfosGPU->unlock();

		mTempTetrahedronPositions.clear();
		mTempTetrahedronBufferIndices.clear();
		mTetrahedronVolumeDirty = false;
	}

	bool LightProbes::hasAnyProbes() const
	{
		for(auto& entry : mVolumes)
		{
			UINT32 numProbes = entry.volume->getNumActiveProbes();
			if (numProbes > 0)
				return true;
		}

		return false;
	}

	void LightProbes::resizeTetrahedronBuffer(UINT32 count)
	{
		GPU_BUFFER_DESC desc;
		desc.type = GBT_STRUCTURED;
		desc.elementSize = sizeof(TetrahedronDataGPU);
		desc.elementCount = count;
		desc.usage = GBU_STATIC;
		desc.format = BF_UNKNOWN;

		mTetrahedronInfosGPU = GpuBuffer::create(desc);
		mMaxTetrahedra = count;
	}

	void LightProbes::resizeCoefficientBuffer(UINT32 count)
	{
		GPU_BUFFER_DESC desc;
		desc.type = GBT_STRUCTURED;
		desc.elementSize = sizeof(LightProbeSHCoefficients);
		desc.elementCount = count;
		desc.usage = GBU_STATIC;
		desc.format = BF_UNKNOWN;

		mProbeCoefficientsGPU = GpuBuffer::create(desc);
		mMaxCoefficients = count;
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

	void LightProbes::generateTetrahedronData(Vector<Vector3>& positions, Vector<TetrahedronData>& output, 
		bool generateExtrapolationVolume)
	{
		bs_frame_mark();
		{
			TetrahedronVolume volume = Triangulation::tetrahedralize(positions);

			if (generateExtrapolationVolume)
			{
				// We don't want ot handle the case where the user looks up a position and it falls outside of the 
				// tetrahedron volume, as the math for projecting the point onto the volume might be too slow for the 
				// shader (which we need in order not to have a sharp cutoff in lighting where the volume ends). Therefore
				// we extend the tetrahedron volume to "infinity" (technically to some far away distance, but we treat it as
				// infinity when calculating barycentric coordinates) by adding new points along the outer face normals.
				UINT32 numOuterFaces = (UINT32)volume.outerFaces.size();

				// Calculate face normals for outer faces
				//// Make an edge map
				struct Edge
				{
					INT32 faces[2];
					INT32 oppositeVerts[2];
				};

				FrameUnorderedMap<std::pair<INT32, INT32>, Edge, pair_hash> edgeMap;
				for (UINT32 i = 0; i < numOuterFaces; ++i)
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

				//// Generate face normals
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

				//// Generate vertex normals
				struct FaceVertex
				{
					Vector3 normal = Vector3::ZERO;
					UINT32 outerIdx = -1;
				};

				FrameUnorderedMap<INT32, FaceVertex> faceVertices;
				for (auto& entry : edgeMap)
				{
					const Edge& edge = entry.second;

					auto accumulateNormalForEdgeVertex = [&](UINT32 v0Idx, UINT32 v1Idx)
					{
						auto iter = faceVertices.insert(std::make_pair(v0Idx, FaceVertex()));

						FaceVertex& accum = iter.first->second;
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
							accum.normal += weight * faceNormals[faceIdx];
						};

						accumulateNormalForFace(edge.faces[0], entry.second.oppositeVerts[0]);
						accumulateNormalForFace(edge.faces[1], entry.second.oppositeVerts[1]);
					};

					accumulateNormalForEdgeVertex(entry.first.first, entry.first.second);
					accumulateNormalForEdgeVertex(entry.first.second, entry.first.first);
				}

				for (auto& entry : faceVertices)
					entry.second.normal.normalize();

				// For each face vertex, generate an outer vertex along its normal
				static const float ExtrapolationDistance = 1000.0f;
				for(auto& entry : faceVertices)
				{
					entry.second.outerIdx = (UINT32)positions.size();

					Vector3 outerPos = positions[entry.first] + entry.second.normal * ExtrapolationDistance;
					positions.push_back(outerPos);
				}

				// For each face, generate outer tetrahedrons
				Vector<Vector3> outerVolumeVerts;
				for (UINT32 i = 0; i < numOuterFaces; ++i)
				{
					const TetrahedronFace& face = volume.outerFaces[i];
					UINT32 originalIndices[6];
					for (UINT32 j = 0; j < 3; j++)
					{
						const FaceVertex& faceVertex = faceVertices[face.vertices[j]];

						outerVolumeVerts.push_back(positions[face.vertices[j]]);
						outerVolumeVerts.push_back(positions[faceVertex.outerIdx]);

						originalIndices[j * 2 + 0] = face.vertices[j];
						originalIndices[j * 2 + 1] = faceVertex.outerIdx;
					}

					TetrahedronVolume outerVolume = Triangulation::tetrahedralize(outerVolumeVerts);
					UINT32 tetStartIdx = (UINT32)volume.tetrahedra.size();

					for (auto& entry : outerVolume.tetrahedra)
					{
						// Remap neighbors to global array
						for (UINT32 j = 0; j < 4; j++)
						{
							if (entry.neighbors[j] != -1)
							{
								// Valid neighbor, map to global array
								entry.neighbors[j] += tetStartIdx;
							}
						}
					}

					// Connect the new volume to the original face
					for (auto& entry : outerVolume.outerFaces)
					{
						// Look for the face sharing all vertices with the original face
						bool isValid = true;
						for (UINT32 j = 0; j < 3; j++)
						{
							if (entry.vertices[j] % 2 == 1)
							{
								isValid = false;
								break;
							}
						}

						if (!isValid)
							continue;

						Tetrahedron& outerTet = outerVolume.tetrahedra[entry.tetrahedron];

						UINT32 oppositeVert = -1;
						for (UINT32 j = 0; j < 4; j++)
						{
							if(outerTet.vertices[j] != entry.vertices[0] &&
								outerTet.vertices[j] != entry.vertices[1] &&
								outerTet.vertices[j] != entry.vertices[2])
							{
								oppositeVert = j;
								break;
							}
						}

						assert(outerTet.neighbors[oppositeVert] == -1);
						outerTet.neighbors[oppositeVert] = face.tetrahedron;

						Tetrahedron& innerTet = volume.tetrahedra[face.tetrahedron];
						for(UINT32 j = 0; j < 4; j++)
						{
							if (innerTet.neighbors[j] == -1)
							{
								// Note: Not searching for opposite neighbor here. If tet. has multiple free faces then we
								// can't just pick the first one
								innerTet.neighbors[j] = tetStartIdx + entry.tetrahedron;
								break;
							}
						}
					}

					for (auto& entry : outerVolume.tetrahedra)
					{
						// Remap vertices back to global array
						for (UINT32 j = 0; j < 4; j++)
							entry.vertices[j] = originalIndices[entry.vertices[j]];
					}

					outerVolumeVerts.clear();

					// Add to global tetrahedra array
					for (auto& entry : outerVolume.tetrahedra)
						volume.tetrahedra.push_back(entry);
				}

				// Note: Not forming neighbor connections between outer tetrahedrons. Since we generate them separately
				// we can't guarantee they even exist.
			}

			// Generate matrices
			UINT32 numOutputTets = (UINT32)volume.tetrahedra.size();
			output.reserve(numOutputTets);

			// Insert tetrahedrons, generate matrices
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

				Vector3 E1 = P1 - P4;
				Vector3 E2 = P2 - P4;
				Vector3 E3 = P3 - P4;

				Matrix3 mat;
				mat.setColumn(0, E1);
				mat.setColumn(1, E2);
				mat.setColumn(2, E3);

				// If tetrahedron is co-planar just ignore it, shader will use some other nearby one instead. We can't
				// handle coplanar tetrahedrons because the matrix is not invertible, and for nearly co-planar ones the
				// math breaks down because of precision issues.
				if (fabs(Vector3::dot(Vector3::normalize(Vector3::cross(E1, E2)), E3)) < 0.0001f)
					continue;

				entry.transform = Matrix4(mat.inverse());
				entry.transform.setColumn(3, Vector4(P4, 1.0f));

				output.push_back(entry);
			}
		}
		bs_frame_clear();
	}
}}
