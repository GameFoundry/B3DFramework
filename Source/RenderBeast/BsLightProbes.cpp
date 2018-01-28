//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLightProbes.h"
#include "Renderer/BsLightProbeVolume.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "BsRendererView.h"
#include "BsRenderBeastIBLUtility.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Material/BsGpuParamsSet.h"
#include "Renderer/BsRendererUtility.h"
#include "Renderer/BsSkybox.h"
#include "BsRendererTextures.h"

namespace bs { namespace ct 
{
	TetrahedraRenderParamDef gTetrahedraRenderParamDef;

	TetrahedraRenderMat::TetrahedraRenderMat()
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", mDepthBufferTex);

		SAMPLER_STATE_DESC pointSampDesc;
		pointSampDesc.minFilter = FO_POINT;
		pointSampDesc.magFilter = FO_POINT;
		pointSampDesc.mipFilter = FO_POINT;
		pointSampDesc.addressMode.u = TAM_CLAMP;
		pointSampDesc.addressMode.v = TAM_CLAMP;
		pointSampDesc.addressMode.w = TAM_CLAMP;

		SPtr<SamplerState> pointSampState = SamplerState::create(pointSampDesc);

		if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferSamp", pointSampState);
		else if(mParams->hasSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex"))
			mParams->setSamplerState(GPT_FRAGMENT_PROGRAM, "gDepthBufferTex", pointSampState);

		mParamBuffer = gTetrahedraRenderParamDef.createBuffer();
		mParams->setParamBlockBuffer("Params", mParamBuffer);
	}

	void TetrahedraRenderMat::execute(const RendererView& view, const SPtr<Texture>& sceneDepth, const SPtr<Mesh>& mesh, 
		const SPtr<RenderTexture>& output)
	{
		const TextureProperties& texProps = sceneDepth->getProperties();

		Vector2I texSize(texProps.getWidth(), texProps.getHeight());
		gTetrahedraRenderParamDef.gDepthTexSize.set(mParamBuffer, texSize);

		mDepthBufferTex.set(sceneDepth);
		mParams->setParamBlockBuffer("PerCamera", view.getPerViewBuffer());

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output);

		bind();
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
		depthDesc = POOLED_RENDER_TEXTURE_DESC::create2D(PF_D32, width, height, TU_DEPTHSTENCIL, numSamples);
	}

	TetrahedraRenderMat* TetrahedraRenderMat::getVariation(bool msaa, bool singleSampleMSAA)
	{
		if (msaa)
		{
			if (singleSampleMSAA)
				return get(getVariation<true, true>());

			return get(getVariation<true, false>());
		}

		return get(getVariation<false, false>());
	}

	IrradianceEvaluateParamDef gIrradianceEvaluateParamDef;

	IrradianceEvaluateMat::IrradianceEvaluateMat()
		:mGBufferParams(GPT_FRAGMENT_PROGRAM, mParams)
	{
		mSkyOnly = mVariation.getBool("SKY_ONLY");

		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSkyIrradianceTex", mParamSkyIrradianceTex);
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gAmbientOcclusionTex", mParamAmbientOcclusionTex);

		if(!mSkyOnly)
		{
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gInputTex", mParamInputTex);
			mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSHCoeffs", mParamSHCoeffsTexture);
			mParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gTetrahedra", mParamTetrahedraBuffer);
			mParams->getBufferParam(GPT_FRAGMENT_PROGRAM, "gTetFaces", mParamTetFacesBuffer);
		}

		mParamBuffer = gIrradianceEvaluateParamDef.createBuffer();
		mParams->setParamBlockBuffer("Params", mParamBuffer);
	}

	void IrradianceEvaluateMat::execute(const RendererView& view, const GBufferTextures& gbuffer, 
		const SPtr<Texture>& lightProbeIndices, const LightProbesInfo& lightProbesInfo, const Skybox* skybox, 
		const SPtr<Texture>& ambientOcclusion, const SPtr<RenderTexture>& output)
	{
		const RendererViewProperties& viewProps = view.getProperties();

		mGBufferParams.bind(gbuffer);

		float skyBrightness = 1.0f;
		SPtr<Texture> skyIrradiance;
		if (skybox != nullptr)
		{
			skyIrradiance = skybox->getIrradiance();
			skyBrightness = skybox->getBrightness();
		}

		if(skyIrradiance == nullptr)
			skyIrradiance = RendererTextures::defaultIndirect;

		mParamSkyIrradianceTex.set(skyIrradiance);
		mParamAmbientOcclusionTex.set(ambientOcclusion);

		if(!mSkyOnly)
		{
			mParamInputTex.set(lightProbeIndices);
			mParamSHCoeffsTexture.set(lightProbesInfo.shCoefficients);
			mParamTetrahedraBuffer.set(lightProbesInfo.tetrahedra);
			mParamTetFacesBuffer.set(lightProbesInfo.faces);
		}

		gIrradianceEvaluateParamDef.gSkyBrightness.set(mParamBuffer, skyBrightness);
		gIrradianceEvaluateParamDef.gNumTetrahedra.set(mParamBuffer, lightProbesInfo.numTetrahedra);
		mParamBuffer->flushToGPU();

		mParams->setParamBlockBuffer("PerCamera", view.getPerViewBuffer());

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.setRenderTarget(output, FBT_DEPTH | FBT_STENCIL, RT_COLOR0);

		bind();

		gRendererUtility().drawScreenQuad(Rect2(0.0f, 0.0f, (float)viewProps.viewRect.width, 
			(float)viewProps.viewRect.height));

		rapi.setRenderTarget(nullptr);
	}

	IrradianceEvaluateMat* IrradianceEvaluateMat::getVariation(bool msaa, bool singleSampleMSAA, bool skyOnly)
	{
		if(skyOnly)
		{
			if (msaa)
			{
				if (singleSampleMSAA)
					return get(getVariation<true, true, true>());

				return get(getVariation<true, false, true>());
			}

			return get(getVariation<false, false, true>());
		}
		else
		{
			if (msaa)
			{
				if (singleSampleMSAA)
					return get(getVariation<true, true, false>());

				return get(getVariation<true, false, false>());
			}

			return get(getVariation<false, false, false>());
		}
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

	/** Information about a single tetrahedron, for use on the GPU. */
	struct TetrahedronDataGPU
	{
		UINT32 indices[4];
		Vector2I offsets[4];
		Matrix3x4 transform;
	};

	/** Information about a single tetrahedron face, for use on the GPU. */
	struct TetrahedronFaceDataGPU
	{
		Vector4 corners[3];
		Vector4 normals[3];
		UINT32 isQuadratic;
		float padding[3];
	};

	LightProbes::LightProbes()
		:mTetrahedronVolumeDirty(false), mMaxCoefficientRows(0), mMaxTetrahedra(0), mMaxFaces(0), mNumValidTetrahedra(0)
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
		UINT32 numRows = 0;
		for(auto& entry : mVolumes)
		{
			SPtr<Texture> localTexture = entry.volume->getCoefficientsTexture();
			numRows += localTexture->getProperties().getHeight();
		}

		if(numRows > mMaxCoefficientRows)
			resizeCoefficientTexture(numRows + 4);

		UINT32 rowIdx = 0;
		for(auto& entry : mVolumes)
		{
			TEXTURE_COPY_DESC copyDesc;
			copyDesc.dstPosition = Vector3I(0, rowIdx, 0);

			SPtr<Texture> localTexture = entry.volume->getCoefficientsTexture();
			localTexture->copy(mProbeCoefficientsGPU, copyDesc);
			
			rowIdx += localTexture->getProperties().getHeight();
		}

		// Gather all positions
		UINT32 bufferOffset = 0;
		rowIdx = 0;
		for(auto& entry : mVolumes)
		{
			const Vector<LightProbeInfo>& infos = entry.volume->getLightProbeInfos();
			const Vector<Vector3>& positions = entry.volume->getLightProbePositions();

			UINT32 numProbes = entry.volume->getNumActiveProbes();

			if (numProbes == 0)
				continue;

			const Transform& tfrm = entry.volume->getTransform();
			Vector3 offset = tfrm.getPosition();
			Quaternion rotation = tfrm.getRotation();

			for (UINT32 i = 0; i < numProbes; i++)
			{
				Vector3 localPos = positions[i];
				Vector3 transformedPos = rotation.rotate(localPos) + offset;
				mTempTetrahedronPositions.push_back(transformedPos);

				mTempTetrahedronBufferIndices.push_back(bufferOffset + infos[i].bufferIdx);

				Vector2I offset = IBLUtility::getSHCoeffXYFromIdx(infos[i].bufferIdx, 3);
				mTempTetrahedronBufferOffsets.push_back(offset);
			}

			SPtr<Texture> localTexture = entry.volume->getCoefficientsTexture();
			rowIdx += localTexture->getProperties().getHeight();
			bufferOffset += (UINT32)positions.size();
		}

		mTetrahedronInfos.clear();

		Vector<TetrahedronFaceData> outerFaces;
		generateTetrahedronData(mTempTetrahedronPositions, mTetrahedronInfos, outerFaces, true);

		// Find valid tetrahedrons
		UINT32 numTetrahedra = (UINT32)mTetrahedronInfos.size();

		bool* validTets = (bool*)bs_stack_alloc(sizeof(bool) * numTetrahedra);
		mNumValidTetrahedra = 0;
		for (UINT32 i = 0; i < (UINT32)mTetrahedronInfos.size(); i++)
		{
			const TetrahedronData& entry = mTetrahedronInfos[i];

			const Vector3& P1 = mTempTetrahedronPositions[entry.volume.vertices[0]];
			const Vector3& P2 = mTempTetrahedronPositions[entry.volume.vertices[1]];
			const Vector3& P3 = mTempTetrahedronPositions[entry.volume.vertices[2]];
			const Vector3& P4 = mTempTetrahedronPositions[entry.volume.vertices[3]];

			Vector3 E1 = P1 - P4;
			Vector3 E2 = P2 - P4;
			Vector3 E3 = P3 - P4;

			// If tetrahedron is co-planar just ignore it, shader will use some other nearby one instead. We can't
			// handle coplanar tetrahedrons because the matrix is not invertible, and for nearly co-planar ones the
			// math breaks down because of precision issues.
			validTets[i] = fabs(Vector3::dot(Vector3::normalize(Vector3::cross(E1, E2)), E3)) > 0.0001f;

			if (validTets[i])
				mNumValidTetrahedra++;
		}

		UINT32 numValidFaces = 0;
		for(auto& entry : outerFaces)
		{
			if (validTets[entry.tetrahedron])
				numValidFaces++;
		}

		// Generate a mesh out of all the tetrahedron triangles
		// Note: Currently the entire volume is rendered as a single large mesh, which will isn't optimal as we can't
		// perform frustum culling. A better option would be to split the mesh into multiple smaller volumes, do
		// frustum culling and possibly even sort by distance from camera.
		UINT32 numVertices = mNumValidTetrahedra * 4 * 3 + numValidFaces * 9 * 3;

		SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDesc->addVertElem(VET_UINT1, VES_TEXCOORD);

		SPtr<MeshData> meshData = MeshData::create(numVertices, numVertices, vertexDesc);
		auto posIter = meshData->getVec3DataIter(VES_POSITION);
		auto idIter = meshData->getDWORDDataIter(VES_TEXCOORD);
		UINT32* indices = meshData->getIndices32();

		// Insert inner tetrahedron triangles
		UINT32 tetIdx = 0;
		for (UINT32 i = 0; i < (UINT32)mTetrahedronInfos.size(); i++)
		{
			if (!validTets[i])
				continue;

			const Tetrahedron& volume = mTetrahedronInfos[i].volume;

			Vector3 center(BsZero);
			for(UINT32 j = 0; j < 4; j++)
				center += mTempTetrahedronPositions[volume.vertices[j]];

			center /= 4.0f;

			static const UINT32 Permutations[4][3] = 
			{
				{ 0, 1, 2 },
				{ 0, 1, 3 },
				{ 0, 2, 3 },
				{ 1, 2, 3 }
			};

			for(UINT32 j = 0; j < 4; j++)
			{
				Vector3 A = mTempTetrahedronPositions[volume.vertices[Permutations[j][0]]];
				Vector3 B = mTempTetrahedronPositions[volume.vertices[Permutations[j][1]]];
				Vector3 C = mTempTetrahedronPositions[volume.vertices[Permutations[j][2]]];

				// Make sure the triangle is clockwise, facing away from the center
				Vector3 e0 = A - C;
				Vector3 e1 = B - C;

				Vector3 normal = e0.cross(e1);
				if (normal.dot(A - center) > 0.0f)
					std::swap(B, C);

				posIter.addValue(A);
				posIter.addValue(B);
				posIter.addValue(C);

				idIter.addValue(tetIdx);
				idIter.addValue(tetIdx);
				idIter.addValue(tetIdx);

				indices[0] = tetIdx * 4 * 3 + j * 3 + 0;
				indices[1] = tetIdx * 4 * 3 + j * 3 + 1;
				indices[2] = tetIdx * 4 * 3 + j * 3 + 2;

				indices += 3;
			}

			tetIdx++;
		}

		// Generate an edge map for outer faces (required for step below)
		struct Edge
		{
			UINT32 vertInner[2];
			UINT32 vertOuter[2];
			UINT32 face[2];
		};

		FrameUnorderedMap<std::pair<INT32, INT32>, Edge, pair_hash> edgeMap;
		for(UINT32 i = 0; i < (UINT32)outerFaces.size(); i++)
		{
			if (!validTets[outerFaces[i].tetrahedron])
				continue;

			for (UINT32 j = 0; j < 3; ++j)
			{
				UINT32 v0 = outerFaces[i].innerVertices[j];
				UINT32 v1 = outerFaces[i].innerVertices[(j + 1) % 3];

				// Keep the same ordering so other faces can find the same edge
				if (v0 > v1)
					std::swap(v0, v1);

				auto iterFind = edgeMap.find(std::make_pair((INT32)v0, (INT32)v1));
				if (iterFind != edgeMap.end())
				{
					iterFind->second.face[1] = i;
				}
				else
				{
					Edge edge;
					edge.vertInner[0] = outerFaces[i].innerVertices[j];
					edge.vertInner[1] = outerFaces[i].innerVertices[(j + 1) % 3];
					edge.vertOuter[0] = outerFaces[i].outerVertices[j];
					edge.vertOuter[1] = outerFaces[i].outerVertices[(j + 1) % 3];
					edge.face[0] = i;
					edge.face[1] = -1;

					edgeMap.insert(std::make_pair(std::make_pair((INT32)v0, (INT32)v1), edge));
				}
			}
		}

		// Generate front and back triangles for extruded outer faces
		UINT32 faceIdx = 0;
		for(UINT32 i = 0; i < (UINT32)outerFaces.size(); i++)
		{
			if (!validTets[outerFaces[i].tetrahedron])
				continue;

			const TetrahedronFaceData& entry = outerFaces[i];

			static const UINT32 Permutations[2][3] = { {0, 1, 2 }, { 3, 4, 5} };

			// Make sure the triangle is clockwise, facing away from the center
			Vector3 center(BsZero);
			for (UINT32 k = 0; k < 3; k++)
			{
				center += mTempTetrahedronPositions[entry.innerVertices[k]];
				center += mTempTetrahedronPositions[entry.outerVertices[k]];
			}

			center /= 6.0f;

			for(UINT32 j = 0; j < 2; ++j)
			{
				UINT32 idxA = Permutations[j][0];
				UINT32 idxB = Permutations[j][1];
				UINT32 idxC = Permutations[j][2];

				idxA = idxA > 2 ? entry.outerVertices[idxA - 3] : entry.innerVertices[idxA];
				idxB = idxB > 2 ? entry.outerVertices[idxB - 3] : entry.innerVertices[idxB];
				idxC = idxC > 2 ? entry.outerVertices[idxC - 3] : entry.innerVertices[idxC];
				
				Vector3 A = mTempTetrahedronPositions[idxA];
				Vector3 B = mTempTetrahedronPositions[idxB];
				Vector3 C = mTempTetrahedronPositions[idxC];

				Vector3 e0 = A - C;
				Vector3 e1 = B - C;

				Vector3 normal = e0.cross(e1);
				if (normal.dot(A - center) > 0.0f)
					std::swap(A, B);

				posIter.addValue(A);
				posIter.addValue(B);
				posIter.addValue(C);

				idIter.addValue(tetIdx + faceIdx);
				idIter.addValue(tetIdx + faceIdx);
				idIter.addValue(tetIdx + faceIdx);

				indices[0] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + j * 3 + 0;
				indices[1] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + j * 3 + 1;
				indices[2] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + j * 3 + 2;

				indices += 3;
			}

			faceIdx++;
		}

		// Generate sides for extruded outer faces
		UINT32 sideIdx = 0;
		for(auto& entry : edgeMap)
		{
			const Edge& edge = entry.second;

			for (UINT32 i = 0; i < 2; i++)
			{
				const TetrahedronFaceData& face = outerFaces[edge.face[i]];

				// Make sure the triangle is clockwise, facing away from the center
				Vector3 center(BsZero);
				for (UINT32 k = 0; k < 3; k++)
				{
					center += mTempTetrahedronPositions[face.innerVertices[k]];
					center += mTempTetrahedronPositions[face.outerVertices[k]];
				}

				center /= 6.0f;

				static const UINT32 Permutations[2][3] = { {0, 1, 2 }, { 1, 2, 3} };
				for(UINT32 j = 0; j < 2; ++j)
				{
					UINT32 idxA = Permutations[j][0];
					UINT32 idxB = Permutations[j][1];
					UINT32 idxC = Permutations[j][2];

					idxA = idxA > 1 ? edge.vertOuter[idxA - 2] : edge.vertInner[idxA];
					idxB = idxB > 1 ? edge.vertOuter[idxB - 2] : edge.vertInner[idxB];
					idxC = idxC > 1 ? edge.vertOuter[idxC - 2] : edge.vertInner[idxC];
					
					Vector3 A = mTempTetrahedronPositions[idxA];
					Vector3 B = mTempTetrahedronPositions[idxB];
					Vector3 C = mTempTetrahedronPositions[idxC];

					Vector3 e0 = A - C;
					Vector3 e1 = B - C;

					Vector3 normal = e0.cross(e1);
					if (normal.dot(A - center) > 0.0f)
						std::swap(A, B);

					posIter.addValue(A);
					posIter.addValue(B);
					posIter.addValue(C);

					idIter.addValue(tetIdx + edge.face[i]);
					idIter.addValue(tetIdx + edge.face[i]);
					idIter.addValue(tetIdx + edge.face[i]);

					indices[0] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + sideIdx * 2 * 3 + j * 3 + 0;
					indices[1] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + sideIdx * 2 * 3 + j * 3 + 1;
					indices[2] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + sideIdx * 2 * 3 + j * 3 + 2;

					indices += 3;
				}

				sideIdx++;
			}
		}

		// Generate "caps" on the end of the extruded volume
		UINT32 capIdx = 0;
		for(UINT32 i = 0; i < (UINT32)outerFaces.size(); i++)
		{
			if (!validTets[outerFaces[i].tetrahedron])
				continue;

			const TetrahedronFaceData& entry = outerFaces[i];

			Vector3 A = mTempTetrahedronPositions[entry.outerVertices[0]];
			Vector3 B = mTempTetrahedronPositions[entry.outerVertices[1]];
			Vector3 C = mTempTetrahedronPositions[entry.outerVertices[2]];

			// Make sure the triangle is clockwise, facing toward the center
			const Tetrahedron& tet = mTetrahedronInfos[entry.tetrahedron].volume;

			Vector3 center(BsZero);
			for(UINT32 j = 0; j < 4; j++)
				center += mTempTetrahedronPositions[tet.vertices[j]];

			center /= 4.0f;

			Vector3 e0 = A - C;
			Vector3 e1 = B - C;

			Vector3 normal = e0.cross(e1);
			if (normal.dot(A - center) < 0.0f)
				std::swap(B, C);

			posIter.addValue(A);
			posIter.addValue(B);
			posIter.addValue(C);

			idIter.addValue(-1);
			idIter.addValue(-1);
			idIter.addValue(-1);

			indices[0] = tetIdx * 4 * 3 + faceIdx * 8 * 3 + capIdx * 3 + 0;
			indices[1] = tetIdx * 4 * 3 + faceIdx * 8 * 3 + capIdx * 3 + 1;
			indices[2] = tetIdx * 4 * 3 + faceIdx * 8 * 3 + capIdx * 3 + 2;

			indices += 3;
			capIdx++;
		}

		mVolumeMesh = Mesh::create(meshData);

		// Map vertices to actual SH coefficient indices, and write GPU buffer with tetrahedron information
		if ((mNumValidTetrahedra + numValidFaces) > mMaxTetrahedra)
		{
			UINT32 newSize = Math::divideAndRoundUp(mNumValidTetrahedra + numValidFaces, 64U) * 64U;
			resizeTetrahedronBuffer(newSize);
		}

		TetrahedronDataGPU* dst = (TetrahedronDataGPU*)mTetrahedronInfosGPU->lock(0, mTetrahedronInfosGPU->getSize(), 
			GBL_WRITE_ONLY_DISCARD);

		// Write inner tetrahedron data
		for (UINT32 i = 0; i < (UINT32)mTetrahedronInfos.size(); i++)
		{
			if (!validTets[i])
				continue;

			TetrahedronData& entry = mTetrahedronInfos[i];

			Vector2I offsets[4];
			for(UINT32 j = 0; j < 4; ++j)
			{
				entry.volume.vertices[j] = mTempTetrahedronBufferIndices[entry.volume.vertices[j]];
				offsets[j] = mTempTetrahedronBufferOffsets[entry.volume.vertices[j]];
			}

			memcpy(dst->indices, entry.volume.vertices, sizeof(UINT32) * 4);
			memcpy(dst->offsets, &offsets, sizeof(offsets));
			memcpy(&dst->transform, &entry.transform, sizeof(float) * 12);

			dst++;
		}

		// Write extruded face data
		for (UINT32 i = 0; i < (UINT32)outerFaces.size(); i++)
		{
			if (!validTets[outerFaces[i].tetrahedron])
				continue;

			const TetrahedronFaceData& entry = outerFaces[i];

			UINT32 indices[4];
			Vector2I offsets[4];
			for(UINT32 j = 0; j < 3; j++)
			{
				indices[j] = mTempTetrahedronBufferIndices[entry.innerVertices[j]];
				offsets[j] = mTempTetrahedronBufferOffsets[entry.innerVertices[j]];
			}

			indices[3] = -1;

			memcpy(dst->indices, indices, sizeof(UINT32) * 4);
			memcpy(dst->offsets, offsets, sizeof(offsets));
			memcpy(&dst->transform, &entry.transform, sizeof(float) * 12);

			dst++;
		}

		mTetrahedronInfosGPU->unlock();

		// Write data specific to faces
		if (numValidFaces > mMaxFaces)
		{
			UINT32 newSize = Math::divideAndRoundUp(numValidFaces, 64U) * 64U;
			resizeTetrahedronFaceBuffer(newSize);
		}

		TetrahedronFaceDataGPU* faceDst = (TetrahedronFaceDataGPU*)mTetrahedronFaceInfosGPU->lock(0, 
			mTetrahedronFaceInfosGPU->getSize(), GBL_WRITE_ONLY_DISCARD);

		for (UINT32 i = 0; i < (UINT32)outerFaces.size(); i++)
		{
			if (!validTets[outerFaces[i].tetrahedron])
				continue;

			const TetrahedronFaceData& entry = outerFaces[i];

			for (UINT32 j = 0; j < 3; j++)
			{
				faceDst->corners[j] = mTempTetrahedronPositions[entry.innerVertices[j]];
				faceDst->normals[j] = entry.normals[j];
			}

			faceDst->isQuadratic = entry.quadratic ? 1 : 0;
			faceDst++;
		}

		mTetrahedronFaceInfosGPU->unlock();

		bs_stack_free(validTets);

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

	LightProbesInfo LightProbes::getInfo() const
	{
		LightProbesInfo info;
		info.shCoefficients = mProbeCoefficientsGPU;
		info.tetrahedra = mTetrahedronInfosGPU;
		info.faces = mTetrahedronFaceInfosGPU;
		info.tetrahedraVolume = mVolumeMesh;
		info.numTetrahedra = mNumValidTetrahedra;

		return info;
	}

	void LightProbes::resizeTetrahedronBuffer(UINT32 count)
	{
		static constexpr UINT32 ELEMENT_SIZE = Math::divideAndRoundUp((UINT32)sizeof(TetrahedronDataGPU), 4U);

		GPU_BUFFER_DESC desc;
		desc.type = GBT_STANDARD;
		desc.elementSize = 0;
		desc.elementCount = count * ELEMENT_SIZE;
		desc.usage = GBU_STATIC;
		desc.format = BF_32X4U;

		mTetrahedronInfosGPU = GpuBuffer::create(desc);
		mMaxTetrahedra = count;
	}

	void LightProbes::resizeTetrahedronFaceBuffer(UINT32 count)
	{
		static constexpr UINT32 ELEMENT_SIZE = Math::divideAndRoundUp((UINT32)sizeof(TetrahedronFaceDataGPU), 4U);

		GPU_BUFFER_DESC desc;
		desc.type = GBT_STANDARD;
		desc.elementSize = 0;
		desc.elementCount = count * ELEMENT_SIZE;
		desc.usage = GBU_STATIC;
		desc.format = BF_32X4F;

		mTetrahedronFaceInfosGPU = GpuBuffer::create(desc);
		mMaxFaces = count;
	}

	void LightProbes::resizeCoefficientTexture(UINT32 numRows)
	{
		TEXTURE_DESC desc;
		desc.width = 4096;
		desc.height = numRows;
		desc.usage = TU_LOADSTORE | TU_RENDERTARGET;
		desc.format = PF_RGBA32F;

		SPtr<Texture> newTexture = Texture::create(desc);
		if (mProbeCoefficientsGPU)
			mProbeCoefficientsGPU->copy(newTexture);

		mProbeCoefficientsGPU = newTexture;
		mMaxCoefficientRows = numRows;
	}

	void LightProbes::generateTetrahedronData(Vector<Vector3>& positions, Vector<TetrahedronData>& tetrahedra,
		Vector<TetrahedronFaceData>& faces,	bool generateExtrapolationVolume)
	{
		bs_frame_mark();
		{
			TetrahedronVolume volume = Triangulation::tetrahedralize(positions);

			if (generateExtrapolationVolume)
			{
				// Add geometry so we can handle the case when the interpolation position falls outside of the tetrahedra
				// volume. We use this geometry to project the position to the nearest face.
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
				struct FaceVertex
				{
					Vector3 normal = Vector3::ZERO;
					UINT32 outerIdx = -1;
				};

				FrameVector<Vector3> faceNormals(volume.outerFaces.size());
				for (UINT32 i = 0; i < (UINT32)volume.outerFaces.size(); ++i)
				{
					const Vector3& v0 = positions[volume.outerFaces[i].vertices[0]];
					const Vector3& v1 = positions[volume.outerFaces[i].vertices[1]];
					const Vector3& v2 = positions[volume.outerFaces[i].vertices[2]];
					
					Vector3 e0 = v1 - v0;
					Vector3 e1 = v2 - v0;

					// Make sure the normal is facing away from the center
					const Tetrahedron& tet = volume.tetrahedra[volume.outerFaces[i].tetrahedron];

					Vector3 center(BsZero);
					for(UINT32 j = 0; j < 4; j++)
						center += positions[tet.vertices[j]];

					center /= 4.0f;

					Vector3 normal = Vector3::normalize(e0.cross(e1));
					if (normal.dot(v0 - center) < 0.0f)
						normal = -normal;

					faceNormals[i] = normal;
				}

				//// Generate vertex normals
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
				static const float ExtrapolationDistance = 5.0f;
				for(auto& entry : faceVertices)
				{
					entry.second.outerIdx = (UINT32)positions.size();

					Vector3 outerPos = positions[entry.first] + entry.second.normal * ExtrapolationDistance;
					positions.push_back(outerPos);
				}

				// Generate face data
				for (UINT32 i = 0; i < numOuterFaces; ++i)
				{
					const TetrahedronFace& face = volume.outerFaces[i];

					TetrahedronFaceData faceData;
					faceData.tetrahedron = face.tetrahedron;

					for (UINT32 j = 0; j < 3; j++)
					{
						const FaceVertex& faceVertex = faceVertices[face.vertices[j]];

						faceData.innerVertices[j] = face.vertices[j];
						faceData.outerVertices[j] = faceVertex.outerIdx;
						faceData.normals[j] = faceVertex.normal;
					}

					// Add a link on the source tetrahedron to the face data
					Tetrahedron& innerTet = volume.tetrahedra[face.tetrahedron];
					for(UINT32 j = 0; j < 4; j++)
					{
						if (innerTet.neighbors[j] == -1)
						{
							// Note: Not searching for opposite neighbor here. If tet. has multiple free faces then we
							// can't just pick the first one
							innerTet.neighbors[j] = (UINT32)volume.tetrahedra.size() + (UINT32)faces.size();
							break;
						}
					}

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

					const Vector3& p0 = positions[faceData.innerVertices[0]];
					const Vector3& p1 = positions[faceData.innerVertices[1]];
					const Vector3& p2 = positions[faceData.innerVertices[2]];

					const Vector3& v0 = faceVertices[faceData.innerVertices[0]].normal;
					const Vector3& v1 = faceVertices[faceData.innerVertices[1]].normal;
					const Vector3& v2 = faceVertices[faceData.innerVertices[2]].normal;

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

					faceData.transform[0][0] = qx;
					faceData.transform[0][1] = qy;
					faceData.transform[0][2] = qz;
					faceData.transform[0][3] = qw;

					faceData.transform[1][0] = rx;
					faceData.transform[1][1] = ry;
					faceData.transform[1][2] = rz;
					faceData.transform[1][3] = rw;

					faceData.transform[2][0] = sx;
					faceData.transform[2][1] = sy;
					faceData.transform[2][2] = sz;
					faceData.transform[2][3] = sw;

					// Unused
					faceData.transform[3][0] = 0.0f;
					faceData.transform[3][1] = 0.0f;
					faceData.transform[3][2] = 0.0f;
					faceData.transform[3][3] = 0.0f;

					if (fabs(p) > 0.00001f)
					{
						faceData.transform = faceData.transform * (1.0f / p);
						faceData.quadratic = false;
					}
					else // Quadratic
					{
						faceData.quadratic = true;
					}

					faces.push_back(faceData);
				}
			}
			else
			{
				for (UINT32 i = 0; i < (UINT32)volume.outerFaces.size(); ++i)
				{
					const TetrahedronFace& face = volume.outerFaces[i];
					TetrahedronFaceData faceData;

					for (UINT32 j = 0; j < 3; j++)
					{
						faceData.innerVertices[j] = face.vertices[j];
						faceData.outerVertices[j] = -1;
						faceData.normals[j] = Vector3::ZERO;
					}

					faceData.tetrahedron = face.tetrahedron;
					faceData.transform = Matrix4::IDENTITY;
					faceData.quadratic = false;

					faces.push_back(faceData);
				}
			}

			// Generate matrices
			UINT32 numOutputTets = (UINT32)volume.tetrahedra.size();
			tetrahedra.reserve(numOutputTets);

			//// For inner tetrahedrons
			for(UINT32 i = 0; i < (UINT32)numOutputTets; ++i)
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

				Matrix4 mat;
				mat.setColumn(0, Vector4(E1, 0.0f));
				mat.setColumn(1, Vector4(E2, 0.0f));
				mat.setColumn(2, Vector4(E3, 0.0f));
				mat.setColumn(3, Vector4(P4, 1.0f));

				entry.transform = mat.inverse();

				tetrahedra.push_back(entry);
			}
		}
		bs_frame_clear();
	}
}}
