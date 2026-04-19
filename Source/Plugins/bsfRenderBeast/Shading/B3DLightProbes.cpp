//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DLightProbes.h"
#include "Components/B3DLightProbeVolume.h"
#include "B3DRendererView.h"
#include "B3DRenderBeastIBLUtility.h"
#include "Mesh/B3DMesh.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DRenderTexture.h"
#include "Renderer/B3DRendererUtility.h"
#include "COmponents/B3DSkybox.h"
#include "Utility/B3DRendererTextures.h"

namespace b3d { namespace render {

TetrahedraRenderUniformDefinition gTetrahedraRenderUniformDefinition;

void TetrahedraRenderMaterial::Initialize()
{
	mGpuParameterSet->GetSampledTextureParameter("gDepthBufferTex", mDepthBufferTextureParameter);

	SamplerStateInformation pointSamplerStateCreateInformation;
	pointSamplerStateCreateInformation.MinFilter = FO_POINT;
	pointSamplerStateCreateInformation.MagFilter = FO_POINT;
	pointSamplerStateCreateInformation.MipFilter = FO_POINT;
	pointSamplerStateCreateInformation.AddressMode.U = TAM_CLAMP;
	pointSamplerStateCreateInformation.AddressMode.V = TAM_CLAMP;
	pointSamplerStateCreateInformation.AddressMode.W = TAM_CLAMP;

	SPtr<SamplerState> pointSampState = mGpuDevice->FindOrCreateSamplerState(pointSamplerStateCreateInformation);

	if(mGpuParameterSet->HasSamplerState("gDepthBufferSamp"))
		mGpuParameterSet->SetSamplerState("gDepthBufferSamp", pointSampState);
	else if(mGpuParameterSet->HasSamplerState("gDepthBufferTex"))
		mGpuParameterSet->SetSamplerState("gDepthBufferTex", pointSampState);

	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
}

void TetrahedraRenderMaterial::Prepare(const RendererView& view, const SPtr<Texture>& sceneDepth)
{
	const TextureProperties& texProps = sceneDepth->GetProperties();

	GpuBufferMappedScope uniforms = gTetrahedraRenderUniformDefinition.AllocateTransient().Map();

	Vector2I texSize(texProps.Width, texProps.Height);
	gTetrahedraRenderUniformDefinition.gDepthTexSize.Set(uniforms, texSize);

	mUniformBufferParameter.Set(uniforms);
	mDepthBufferTextureParameter.Set(sceneDepth);
	mGpuParameterSet->SetUniformBuffer("PerCamera", view.GetPerViewBuffer());
}

void TetrahedraRenderMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Mesh>& mesh, const SPtr<RenderTexture>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	RenderPassCreateInformation info(output, mGpuParameterSet);
	commandBuffer.BeginRenderPass(info);

	Bind(commandBuffer);
	GetRendererUtility().Draw(commandBuffer, mesh);

	commandBuffer.EndRenderPass();
}

void TetrahedraRenderMaterial::GetOutputDesc(const RendererView& view, PooledRenderTextureCreateInformation& colorDesc, PooledRenderTextureCreateInformation& depthDesc)
{
	const RendererViewProperties& viewProps = view.GetProperties();
	u32 width = viewProps.Target.ViewRect.Width;
	u32 height = viewProps.Target.ViewRect.Height;
	u32 numSamples = viewProps.Target.NumSamples;

	colorDesc = PooledRenderTextureCreateInformation::Create2D(PF_R16U, width, height, TextureUsageFlag::RenderTarget, numSamples);
	depthDesc = PooledRenderTextureCreateInformation::Create2D(PF_D32, width, height, TextureUsageFlag::DepthStencil, numSamples);
}

TetrahedraRenderMaterial* TetrahedraRenderMaterial::GetVariation(bool msaa, bool singleSampleMSAA)
{
	if(msaa)
	{
		if(singleSampleMSAA)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}

	return Get(GetVariation<false, false>());
}

IrradianceEvaluateUniformDefinition gIrradianceEvaluateUniformDefinition;

void IrradianceEvaluateMaterial::Initialize()
{
	mGBufferParams.Initialize(*mGpuDevice, GPT_FRAGMENT_PROGRAM, mGpuParameterSet);
	mSkyOnly = mVariationParameters.GetBool("SKY_ONLY");

	mGpuParameterSet->GetSampledTextureParameter("gSkyIrradianceTex", mSkyIrradianceTextureParameter);
	mGpuParameterSet->GetSampledTextureParameter("gAmbientOcclusionTex", mAmbientOcclusionTextureParameter);

	if(!mSkyOnly)
	{
		mGpuParameterSet->GetSampledTextureParameter("gInputTex", mInputTextureParameter);
		mGpuParameterSet->GetSampledTextureParameter("gSHCoeffs", mSHCoeffsTextureParameter);
		mGpuParameterSet->GetStorageBufferParameter("gTetrahedra", mTetrahedraBufferParameter);
		mGpuParameterSet->GetStorageBufferParameter("gTetFaces", mTetFacesBufferParameter);
	}

	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
}

void IrradianceEvaluateMaterial::Execute(GpuCommandBuffer& commandBuffer, const RendererView& view, const GBufferTextures& gbuffer, const SPtr<Texture>& lightProbeIndices, const LightProbesInfo& lightProbesInfo, const Skybox* skybox, const SPtr<Texture>& ambientOcclusion, const SPtr<RenderTexture>& output)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const RendererViewProperties& viewProps = view.GetProperties();

	mGBufferParams.Bind(gbuffer);

	float skyBrightness = 1.0f;
	SPtr<Texture> skyIrradiance;
	if(skybox != nullptr)
	{
		skyIrradiance = skybox->GetIrradiance();
		skyBrightness = skybox->GetBrightness();
	}

	if(skyIrradiance == nullptr)
		skyIrradiance = RendererTextures::defaultIndirect;

	GpuBufferMappedScope uniforms = gIrradianceEvaluateUniformDefinition.AllocateTransient().Map();

	gIrradianceEvaluateUniformDefinition.gSkyBrightness.Set(uniforms, skyBrightness);
	gIrradianceEvaluateUniformDefinition.gNumTetrahedra.Set(uniforms, lightProbesInfo.NumTetrahedra);

	mUniformBufferParameter.Set(uniforms);
	mSkyIrradianceTextureParameter.Set(skyIrradiance);
	mAmbientOcclusionTextureParameter.Set(ambientOcclusion);

	RenderSurfaceMask loadMask = RT_COLOR0;
	if(!mSkyOnly)
	{
		mInputTextureParameter.Set(lightProbeIndices);
		mSHCoeffsTextureParameter.Set(lightProbesInfo.ShCoefficients);
		mTetrahedraBufferParameter.Set(lightProbesInfo.Tetrahedra);
		mTetFacesBufferParameter.Set(lightProbesInfo.Faces);
	}
	else
	{
		// No need to load depth/stencil when rendering light probes as we'll be using a newly created intermediate
		// depth buffer
		loadMask |= RT_DEPTH_STENCIL;
	}

	mGpuParameterSet->SetUniformBuffer("PerCamera", view.GetPerViewBuffer());

	// Render
	commandBuffer.BeginRenderPass(RenderPassCreateInformation(output, mGpuParameterSet, RT_DEPTH_STENCIL, loadMask));

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0.0f, 0.0f, (float)viewProps.Target.ViewRect.Width, (float)viewProps.Target.ViewRect.Height));

	commandBuffer.EndRenderPass();
}

IrradianceEvaluateMaterial* IrradianceEvaluateMaterial::GetVariation(bool msaa, bool singleSampleMSAA, bool skyOnly)
{
	if(skyOnly)
	{
		if(msaa)
		{
			if(singleSampleMSAA)
				return Get(GetVariation<true, true, true>());

			return Get(GetVariation<true, false, true>());
		}

		return Get(GetVariation<false, false, true>());
	}
	else
	{
		if(msaa)
		{
			if(singleSampleMSAA)
				return Get(GetVariation<true, true, false>());

			return Get(GetVariation<true, false, false>());
		}

		return Get(GetVariation<false, false, false>());
	}
}

/** Hash value generator for std::pair<i32, i32>. */
struct pair_hash
{
	size_t operator()(const std::pair<i32, i32>& key) const
	{
		size_t hash = 0;
		b3d::B3DCombineHash(hash, key.first);
		b3d::B3DCombineHash(hash, key.second);

		return hash;
	}
};

/** Information about a single tetrahedron, for use on the GPU. */
struct TetrahedronDataGPU
{
	u32 Indices[4];
	Vector2I Offsets[4];
	Matrix3x4 Transform;
};

/** Information about a single tetrahedron face, for use on the GPU. */
struct TetrahedronFaceDataGPU
{
	Vector4 Corners[3];
	Vector4 Normals[3];
	u32 IsQuadratic;
	float Padding[3];
};

LightProbes::LightProbes()
	: mTetrahedronVolumeDirty(false), mMaxCoefficientRows(0), mMaxTetrahedra(0), mMaxFaces(0), mNumValidTetrahedra(0)
{
	mGpuDevice = GetApplication().GetPrimaryGpuDevice();
}

void LightProbes::NotifyAdded(LightProbeVolume* volume)
{
	u32 handle = (u32)mVolumes.size();

	VolumeInfo info;
	info.Volume = volume;
	info.IsDirty = true;

	mVolumes.push_back(info);
	volume->SetRendererId(handle);

	NotifyDirty(volume);
}

void LightProbes::NotifyDirty(LightProbeVolume* volume)
{
	u32 handle = volume->GetRendererId();
	mVolumes[handle].IsDirty = true;

	mTetrahedronVolumeDirty = true;
}

void LightProbes::NotifyRemoved(LightProbeVolume* volume)
{
	u32 handle = volume->GetRendererId();

	LightProbeVolume* lastVolume = mVolumes.back().Volume;
	u32 lastHandle = lastVolume->GetRendererId();

	if(handle != lastHandle)
	{
		// Swap current last element with the one we want to erase
		std::swap(mVolumes[handle], mVolumes[lastHandle]);
		lastVolume->SetRendererId(handle);
	}

	// Erase last (empty) element
	mVolumes.erase(mVolumes.end() - 1);

	mTetrahedronVolumeDirty = true;
}

void LightProbes::UpdateProbes(GpuCommandBuffer& commandBuffer)
{
	if(!mTetrahedronVolumeDirty)
		return;

	// Move all coefficients into the global buffer
	u32 numRows = 0;
	for(auto& entry : mVolumes)
	{
		SPtr<Texture> localTexture = entry.Volume->GetCoefficientsTexture();
		numRows += localTexture->GetProperties().Height;
	}

	if(numRows > mMaxCoefficientRows)
		ResizeCoefficientTexture(commandBuffer, numRows + 4);

	u32 rowIdx = 0;
	for(auto& entry : mVolumes)
	{
		TextureCopyInformation copyDesc;
		copyDesc.DestinationPosition = Vector3I(0, rowIdx, 0);

		SPtr<Texture> localTexture = entry.Volume->GetCoefficientsTexture();
		commandBuffer.CopyTexture(localTexture, mProbeCoefficientsGPU, copyDesc);

		rowIdx += localTexture->GetProperties().Height;
	}

	// Gather all positions
	u32 bufferOffset = 0;
	rowIdx = 0;
	for(auto& entry : mVolumes)
	{
		const Vector<LightProbeInfo>& infos = entry.Volume->GetLightProbeInfos();
		const Vector<Vector3>& positions = entry.Volume->GetLightProbePositions();

		u32 numProbes = entry.Volume->GetActiveProbeCount();

		if(numProbes == 0)
			continue;

		const Transform& tfrm = entry.Volume->GetWorldTransform();
		Vector3 offset = tfrm.GetPosition();
		Quaternion rotation = tfrm.GetRotation();

		for(u32 i = 0; i < numProbes; i++)
		{
			Vector3 localPos = positions[i];
			Vector3 transformedPos = rotation.Rotate(localPos) + offset;
			mTempTetrahedronPositions.push_back(transformedPos);

			mTempTetrahedronBufferIndices.push_back(bufferOffset + infos[i].BufferIdx);

			Vector2I offset = IBLUtility::GetShCoeffXyFromIdx(infos[i].BufferIdx, 3);
			mTempTetrahedronBufferOffsets.push_back(offset);
		}

		SPtr<Texture> localTexture = entry.Volume->GetCoefficientsTexture();
		rowIdx += localTexture->GetProperties().Height;
		bufferOffset += (u32)positions.size();
	}

	mTetrahedronInfos.clear();

	Vector<TetrahedronFaceData> outerFaces;
	GenerateTetrahedronData(mTempTetrahedronPositions, mTetrahedronInfos, outerFaces, true);

	// Find valid tetrahedrons
	u32 numTetrahedra = (u32)mTetrahedronInfos.size();

	bool* validTets = (bool*)B3DStackAllocate(sizeof(bool) * numTetrahedra);
	mNumValidTetrahedra = 0;
	for(u32 i = 0; i < (u32)mTetrahedronInfos.size(); i++)
	{
		const TetrahedronData& entry = mTetrahedronInfos[i];

		const Vector3& P1 = mTempTetrahedronPositions[entry.Volume.Vertices[0]];
		const Vector3& P2 = mTempTetrahedronPositions[entry.Volume.Vertices[1]];
		const Vector3& P3 = mTempTetrahedronPositions[entry.Volume.Vertices[2]];
		const Vector3& P4 = mTempTetrahedronPositions[entry.Volume.Vertices[3]];

		Vector3 E1 = P1 - P4;
		Vector3 E2 = P2 - P4;
		Vector3 E3 = P3 - P4;

		// If tetrahedron is co-planar just ignore it, shader will use some other nearby one instead. We can't
		// handle coplanar tetrahedrons because the matrix is not invertible, and for nearly co-planar ones the
		// math breaks down because of precision issues.
		validTets[i] = fabs(Vector3::Dot(Vector3::Normalize(Vector3::Cross(E1, E2)), E3)) > 0.0001f;

		if(validTets[i])
			mNumValidTetrahedra++;
	}

	u32 numValidFaces = 0;
	for(auto& entry : outerFaces)
	{
		if(validTets[entry.Tetrahedron])
			numValidFaces++;
	}

	// Generate a mesh out of all the tetrahedron triangles
	// Note: Currently the entire volume is rendered as a single large mesh, which will isn't optimal as we can't
	// perform frustum culling. A better option would be to split the mesh into multiple smaller volumes, do
	// frustum culling and possibly even sort by distance from camera.
	u32 numVertices = mNumValidTetrahedra * 4 * 3 + numValidFaces * 9 * 3;

	TInlineArray<VertexElement, 8> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
	vertexElements.Add(VertexElement(VET_UINT1, VES_TEXCOORD));

	SPtr<VertexDescription> vertexDesc = B3DMakeShared<VertexDescription>(vertexElements);
	SPtr<MeshData> meshData = MeshData::Create(numVertices, numVertices, vertexDesc);
	auto posIter = meshData->GetVec3DataIter(VES_POSITION);
	auto idIter = meshData->GetDwordDataIter(VES_TEXCOORD);
	u32* indices = meshData->GetIndices32();

	// Insert inner tetrahedron triangles
	u32 tetIdx = 0;
	for(u32 i = 0; i < (u32)mTetrahedronInfos.size(); i++)
	{
		if(!validTets[i])
			continue;

		const Tetrahedron& volume = mTetrahedronInfos[i].Volume;

		Vector3 center(kZeroTag);
		for(u32 j = 0; j < 4; j++)
			center += mTempTetrahedronPositions[volume.Vertices[j]];

		center /= 4.0f;

		static const u32 kPermutations[4][3] = {
			{ 0, 1, 2 },
			{ 0, 1, 3 },
			{ 0, 2, 3 },
			{ 1, 2, 3 }
		};

		for(u32 j = 0; j < 4; j++)
		{
			Vector3 A = mTempTetrahedronPositions[volume.Vertices[kPermutations[j][0]]];
			Vector3 B = mTempTetrahedronPositions[volume.Vertices[kPermutations[j][1]]];
			Vector3 C = mTempTetrahedronPositions[volume.Vertices[kPermutations[j][2]]];

			// Make sure the triangle is clockwise, facing away from the center
			Vector3 e0 = A - C;
			Vector3 e1 = B - C;

			Vector3 normal = e0.Cross(e1);
			if(normal.Dot(A - center) > 0.0f)
				std::swap(B, C);

			posIter.AddValue(A);
			posIter.AddValue(B);
			posIter.AddValue(C);

			idIter.AddValue(tetIdx);
			idIter.AddValue(tetIdx);
			idIter.AddValue(tetIdx);

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
		u32 VertInner[2];
		u32 VertOuter[2];
		u32 Face[2];
	};

	FrameUnorderedMap<std::pair<i32, i32>, Edge, pair_hash> edgeMap;
	for(u32 i = 0; i < (u32)outerFaces.size(); i++)
	{
		if(!validTets[outerFaces[i].Tetrahedron])
			continue;

		for(u32 j = 0; j < 3; ++j)
		{
			u32 v0 = outerFaces[i].InnerVertices[j];
			u32 v1 = outerFaces[i].InnerVertices[(j + 1) % 3];

			// Keep the same ordering so other faces can find the same edge
			if(v0 > v1)
				std::swap(v0, v1);

			auto iterFind = edgeMap.find(std::make_pair((i32)v0, (i32)v1));
			if(iterFind != edgeMap.end())
			{
				iterFind->second.Face[1] = i;
			}
			else
			{
				Edge edge;
				edge.VertInner[0] = outerFaces[i].InnerVertices[j];
				edge.VertInner[1] = outerFaces[i].InnerVertices[(j + 1) % 3];
				edge.VertOuter[0] = outerFaces[i].OuterVertices[j];
				edge.VertOuter[1] = outerFaces[i].OuterVertices[(j + 1) % 3];
				edge.Face[0] = i;
				edge.Face[1] = -1;

				edgeMap.insert(std::make_pair(std::make_pair((i32)v0, (i32)v1), edge));
			}
		}
	}

	// Generate front and back triangles for extruded outer faces
	u32 faceIdx = 0;
	for(u32 i = 0; i < (u32)outerFaces.size(); i++)
	{
		if(!validTets[outerFaces[i].Tetrahedron])
			continue;

		const TetrahedronFaceData& entry = outerFaces[i];

		static const u32 kPermutations[2][3] = { { 0, 1, 2 }, { 3, 4, 5 } };

		// Make sure the triangle is clockwise, facing away from the center
		Vector3 center(kZeroTag);
		for(u32 k = 0; k < 3; k++)
		{
			center += mTempTetrahedronPositions[entry.InnerVertices[k]];
			center += mTempTetrahedronPositions[entry.OuterVertices[k]];
		}

		center /= 6.0f;

		for(u32 j = 0; j < 2; ++j)
		{
			u32 idxA = kPermutations[j][0];
			u32 idxB = kPermutations[j][1];
			u32 idxC = kPermutations[j][2];

			idxA = idxA > 2 ? entry.OuterVertices[idxA - 3] : entry.InnerVertices[idxA];
			idxB = idxB > 2 ? entry.OuterVertices[idxB - 3] : entry.InnerVertices[idxB];
			idxC = idxC > 2 ? entry.OuterVertices[idxC - 3] : entry.InnerVertices[idxC];

			Vector3 A = mTempTetrahedronPositions[idxA];
			Vector3 B = mTempTetrahedronPositions[idxB];
			Vector3 C = mTempTetrahedronPositions[idxC];

			Vector3 e0 = A - C;
			Vector3 e1 = B - C;

			Vector3 normal = e0.Cross(e1);
			if(normal.Dot(A - center) > 0.0f)
				std::swap(A, B);

			posIter.AddValue(A);
			posIter.AddValue(B);
			posIter.AddValue(C);

			idIter.AddValue(tetIdx + faceIdx);
			idIter.AddValue(tetIdx + faceIdx);
			idIter.AddValue(tetIdx + faceIdx);

			indices[0] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + j * 3 + 0;
			indices[1] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + j * 3 + 1;
			indices[2] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + j * 3 + 2;

			indices += 3;
		}

		faceIdx++;
	}

	// Generate sides for extruded outer faces
	u32 sideIdx = 0;
	for(auto& entry : edgeMap)
	{
		const Edge& edge = entry.second;

		for(u32 i = 0; i < 2; i++)
		{
			const TetrahedronFaceData& face = outerFaces[edge.Face[i]];

			// Make sure the triangle is clockwise, facing away from the center
			Vector3 center(kZeroTag);
			for(u32 k = 0; k < 3; k++)
			{
				center += mTempTetrahedronPositions[face.InnerVertices[k]];
				center += mTempTetrahedronPositions[face.OuterVertices[k]];
			}

			center /= 6.0f;

			static const u32 kPermutations[2][3] = { { 0, 1, 2 }, { 1, 2, 3 } };
			for(u32 j = 0; j < 2; ++j)
			{
				u32 idxA = kPermutations[j][0];
				u32 idxB = kPermutations[j][1];
				u32 idxC = kPermutations[j][2];

				idxA = idxA > 1 ? edge.VertOuter[idxA - 2] : edge.VertInner[idxA];
				idxB = idxB > 1 ? edge.VertOuter[idxB - 2] : edge.VertInner[idxB];
				idxC = idxC > 1 ? edge.VertOuter[idxC - 2] : edge.VertInner[idxC];

				Vector3 A = mTempTetrahedronPositions[idxA];
				Vector3 B = mTempTetrahedronPositions[idxB];
				Vector3 C = mTempTetrahedronPositions[idxC];

				Vector3 e0 = A - C;
				Vector3 e1 = B - C;

				Vector3 normal = e0.Cross(e1);
				if(normal.Dot(A - center) > 0.0f)
					std::swap(A, B);

				posIter.AddValue(A);
				posIter.AddValue(B);
				posIter.AddValue(C);

				idIter.AddValue(tetIdx + edge.Face[i]);
				idIter.AddValue(tetIdx + edge.Face[i]);
				idIter.AddValue(tetIdx + edge.Face[i]);

				indices[0] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + sideIdx * 2 * 3 + j * 3 + 0;
				indices[1] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + sideIdx * 2 * 3 + j * 3 + 1;
				indices[2] = tetIdx * 4 * 3 + faceIdx * 2 * 3 + sideIdx * 2 * 3 + j * 3 + 2;

				indices += 3;
			}

			sideIdx++;
		}
	}

	// Generate "caps" on the end of the extruded volume
	u32 capIdx = 0;
	for(u32 i = 0; i < (u32)outerFaces.size(); i++)
	{
		if(!validTets[outerFaces[i].Tetrahedron])
			continue;

		const TetrahedronFaceData& entry = outerFaces[i];

		Vector3 A = mTempTetrahedronPositions[entry.OuterVertices[0]];
		Vector3 B = mTempTetrahedronPositions[entry.OuterVertices[1]];
		Vector3 C = mTempTetrahedronPositions[entry.OuterVertices[2]];

		// Make sure the triangle is clockwise, facing toward the center
		const Tetrahedron& tet = mTetrahedronInfos[entry.Tetrahedron].Volume;

		Vector3 center(kZeroTag);
		for(u32 j = 0; j < 4; j++)
			center += mTempTetrahedronPositions[tet.Vertices[j]];

		center /= 4.0f;

		Vector3 e0 = A - C;
		Vector3 e1 = B - C;

		Vector3 normal = e0.Cross(e1);
		if(normal.Dot(A - center) < 0.0f)
			std::swap(B, C);

		posIter.AddValue(A);
		posIter.AddValue(B);
		posIter.AddValue(C);

		idIter.AddValue(-1);
		idIter.AddValue(-1);
		idIter.AddValue(-1);

		indices[0] = tetIdx * 4 * 3 + faceIdx * 8 * 3 + capIdx * 3 + 0;
		indices[1] = tetIdx * 4 * 3 + faceIdx * 8 * 3 + capIdx * 3 + 1;
		indices[2] = tetIdx * 4 * 3 + faceIdx * 8 * 3 + capIdx * 3 + 2;

		indices += 3;
		capIdx++;
	}

	mVolumeMesh = Mesh::Create(meshData);

	// Map vertices to actual SH coefficient indices, and write GPU buffer with tetrahedron information
	if((mNumValidTetrahedra + numValidFaces) > mMaxTetrahedra)
	{
		u32 newSize = Math::DivideAndRoundUp(mNumValidTetrahedra + numValidFaces, 64U) * 64U;
		ResizeTetrahedronBuffer(newSize);
	}

	TetrahedronDataGPU* dst = (TetrahedronDataGPU*)B3DStackAllocate(mTetrahedronInfosGPU->GetTotalSize());

	// Write inner tetrahedron data
	for(u32 i = 0; i < (u32)mTetrahedronInfos.size(); i++)
	{
		if(!validTets[i])
			continue;

		TetrahedronData& entry = mTetrahedronInfos[i];

		Vector2I offsets[4];
		for(u32 j = 0; j < 4; ++j)
		{
			entry.Volume.Vertices[j] = mTempTetrahedronBufferIndices[entry.Volume.Vertices[j]];
			offsets[j] = mTempTetrahedronBufferOffsets[entry.Volume.Vertices[j]];
		}

		memcpy(dst->Indices, entry.Volume.Vertices, sizeof(u32) * 4);
		memcpy(dst->Offsets, &offsets, sizeof(offsets));
		memcpy(&dst->Transform, &entry.Transform, sizeof(float) * 12);

		dst++;
	}

	// Write extruded face data
	for(u32 i = 0; i < (u32)outerFaces.size(); i++)
	{
		if(!validTets[outerFaces[i].Tetrahedron])
			continue;

		const TetrahedronFaceData& entry = outerFaces[i];

		u32 indices[4];
		Vector2I offsets[4];
		for(u32 j = 0; j < 3; j++)
		{
			indices[j] = mTempTetrahedronBufferIndices[entry.InnerVertices[j]];
			offsets[j] = mTempTetrahedronBufferOffsets[entry.InnerVertices[j]];
		}

		indices[3] = -1;

		memcpy(dst->Indices, indices, sizeof(u32) * 4);
		memcpy(dst->Offsets, offsets, sizeof(offsets));
		memcpy(&dst->Transform, &entry.Transform, sizeof(float) * 12);

		dst++;
	}

	GpuBufferUtility::Write(mTetrahedronInfosGPU, 0, mTetrahedronInfosGPU->GetTotalSize(), dst, GpuBufferWriteFlag::Discard);
	B3DStackFree(dst);

	// Write data specific to faces
	if(numValidFaces > mMaxFaces)
	{
		u32 newSize = Math::DivideAndRoundUp(numValidFaces, 64U) * 64U;
		ResizeTetrahedronFaceBuffer(newSize);
	}

	TetrahedronFaceDataGPU* faceDst = (TetrahedronFaceDataGPU*)B3DStackAllocate(mTetrahedronFaceInfosGPU->GetTotalSize());

	for(u32 i = 0; i < (u32)outerFaces.size(); i++)
	{
		if(!validTets[outerFaces[i].Tetrahedron])
			continue;

		const TetrahedronFaceData& entry = outerFaces[i];

		for(u32 j = 0; j < 3; j++)
		{
			faceDst->Corners[j] = mTempTetrahedronPositions[entry.InnerVertices[j]];
			faceDst->Normals[j] = entry.Normals[j];
		}

		faceDst->IsQuadratic = entry.Quadratic ? 1 : 0;
		faceDst++;
	}

	GpuBufferUtility::Write(mTetrahedronFaceInfosGPU, 0, mTetrahedronFaceInfosGPU->GetTotalSize(), faceDst, GpuBufferWriteFlag::Discard);
	B3DStackFree(faceDst);

	B3DStackFree(validTets);

	mTempTetrahedronPositions.clear();
	mTempTetrahedronBufferIndices.clear();
	mTetrahedronVolumeDirty = false;
}

bool LightProbes::HasAnyProbes() const
{
	for(auto& entry : mVolumes)
	{
		u32 numProbes = entry.Volume->GetActiveProbeCount();
		if(numProbes > 0)
			return true;
	}

	return false;
}

LightProbesInfo LightProbes::GetInfo() const
{
	LightProbesInfo info;
	info.ShCoefficients = mProbeCoefficientsGPU;
	info.Tetrahedra = mTetrahedronInfosGPU;
	info.Faces = mTetrahedronFaceInfosGPU;
	info.TetrahedraVolume = mVolumeMesh;
	info.NumTetrahedra = mNumValidTetrahedra;

	return info;
}

void LightProbes::ResizeTetrahedronBuffer(u32 count)
{
	static constexpr u32 kElementSize = Math::DivideAndRoundUp((u32)sizeof(TetrahedronDataGPU), 4U);

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU;
	bufferCreateInformation.SimpleStorage.Count = count * kElementSize;
	bufferCreateInformation.SimpleStorage.Format = BF_32X4U;

	mTetrahedronInfosGPU = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
	mMaxTetrahedra = count;
}

void LightProbes::ResizeTetrahedronFaceBuffer(u32 count)
{
	static constexpr u32 kElementSize = Math::DivideAndRoundUp((u32)sizeof(TetrahedronFaceDataGPU), 4U);

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	bufferCreateInformation.Flags = GpuBufferFlag::StoreOnGPU;
	bufferCreateInformation.SimpleStorage.Count = count * kElementSize;
	bufferCreateInformation.SimpleStorage.Format = BF_32X4F;

	mTetrahedronFaceInfosGPU = mGpuDevice->CreateGpuBuffer(bufferCreateInformation);
	mMaxFaces = count;
}

void LightProbes::ResizeCoefficientTexture(GpuCommandBuffer& commandBuffer, u32 numRows)
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	TextureCreateInformation desc;
	desc.Width = 4096;
	desc.Height = numRows;
	desc.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU | TextureUsageFlag::RenderTarget;
	desc.Format = PF_RGBA32F;

	SPtr<Texture> newTexture = gpuDevice->CreateTexture(desc);
	if (mProbeCoefficientsGPU)
		commandBuffer.CopyTexture(mProbeCoefficientsGPU, newTexture);

	mProbeCoefficientsGPU = newTexture;
	mMaxCoefficientRows = numRows;
}

void LightProbes::GenerateTetrahedronData(Vector<Vector3>& positions, Vector<TetrahedronData>& tetrahedra, Vector<TetrahedronFaceData>& faces, bool generateExtrapolationVolume)
{
	B3DMarkAllocatorFrame();
	{
		TetrahedronVolume volume = Triangulation::Tetrahedralize(positions);

		if(generateExtrapolationVolume)
		{
			// Add geometry so we can handle the case when the interpolation position falls outside of the tetrahedra
			// volume. We use this geometry to project the position to the nearest face.
			u32 numOuterFaces = (u32)volume.OuterFaces.size();

			// Calculate face normals for outer faces
			//// Make an edge map
			struct Edge
			{
				i32 Faces[2];
				i32 OppositeVerts[2];
			};

			FrameUnorderedMap<std::pair<i32, i32>, Edge, pair_hash> edgeMap;
			for(u32 i = 0; i < numOuterFaces; ++i)
			{
				for(u32 j = 0; j < 3; ++j)
				{
					i32 v0 = volume.OuterFaces[i].Vertices[j];
					i32 v1 = volume.OuterFaces[i].Vertices[(j + 1) % 3];

					// Keep the same ordering so other faces can find the same edge
					if(v0 > v1)
						std::swap(v0, v1);

					auto iterFind = edgeMap.find(std::make_pair(v0, v1));
					if(iterFind != edgeMap.end())
					{
						iterFind->second.Faces[1] = i;
						iterFind->second.OppositeVerts[1] = (j + 2) % 3;
					}
					else
					{
						Edge edge;
						edge.Faces[0] = i;
						edge.OppositeVerts[0] = (j + 2) % 3;

						edgeMap.insert(std::make_pair(std::make_pair(v0, v1), edge));
					}
				}
			}

			//// Generate face normals
			struct FaceVertex
			{
				Vector3 Normal = Vector3::kZero;
				u32 OuterIdx = -1;
			};

			FrameVector<Vector3> faceNormals(volume.OuterFaces.size());
			for(u32 i = 0; i < (u32)volume.OuterFaces.size(); ++i)
			{
				const Vector3& v0 = positions[volume.OuterFaces[i].Vertices[0]];
				const Vector3& v1 = positions[volume.OuterFaces[i].Vertices[1]];
				const Vector3& v2 = positions[volume.OuterFaces[i].Vertices[2]];

				Vector3 e0 = v1 - v0;
				Vector3 e1 = v2 - v0;

				// Make sure the normal is facing away from the center
				const Tetrahedron& tet = volume.Tetrahedra[volume.OuterFaces[i].Tetrahedron];

				Vector3 center(kZeroTag);
				for(u32 j = 0; j < 4; j++)
					center += positions[tet.Vertices[j]];

				center /= 4.0f;

				Vector3 normal = Vector3::Normalize(e0.Cross(e1));
				if(normal.Dot(v0 - center) < 0.0f)
					normal = -normal;

				faceNormals[i] = normal;
			}

			//// Generate vertex normals
			FrameUnorderedMap<i32, FaceVertex> faceVertices;
			for(auto& entry : edgeMap)
			{
				const Edge& edge = entry.second;

				auto accumulateNormalForEdgeVertex = [&](u32 v0Idx, u32 v1Idx)
				{
					auto iter = faceVertices.insert(std::make_pair(v0Idx, FaceVertex()));

					FaceVertex& accum = iter.first->second;
					const Vector3& v0 = positions[v0Idx];

					auto accumulateNormalForFace = [&](i32 faceIdx, i32 v2LocIdx)
					{
						const TetrahedronFace& face = volume.OuterFaces[faceIdx];

						// Vertices on the face, that aren't the vertex we're calculating the normal for
						const Vector3& v1 = positions[v1Idx];
						const Vector3& v2 = positions[face.Vertices[v2LocIdx]];

						// Weight the contribution to the normal based on the angle spanned by the triangle
						Vector3 e0 = Vector3::Normalize(v1 - v0);
						Vector3 e1 = Vector3::Normalize(v2 - v0);

						float weight = acos(e0.Dot(e1));
						accum.Normal += weight * faceNormals[faceIdx];
					};

					accumulateNormalForFace(edge.Faces[0], entry.second.OppositeVerts[0]);
					accumulateNormalForFace(edge.Faces[1], entry.second.OppositeVerts[1]);
				};

				accumulateNormalForEdgeVertex(entry.first.first, entry.first.second);
				accumulateNormalForEdgeVertex(entry.first.second, entry.first.first);
			}

			for(auto& entry : faceVertices)
				entry.second.Normal.Normalize();

			// For each face vertex, generate an outer vertex along its normal
			static const float kExtrapolationDistance = 5.0f;
			for(auto& entry : faceVertices)
			{
				entry.second.OuterIdx = (u32)positions.size();

				Vector3 outerPos = positions[entry.first] + entry.second.Normal * kExtrapolationDistance;
				positions.push_back(outerPos);
			}

			// Generate face data
			for(u32 i = 0; i < numOuterFaces; ++i)
			{
				const TetrahedronFace& face = volume.OuterFaces[i];

				TetrahedronFaceData faceData;
				faceData.Tetrahedron = face.Tetrahedron;

				for(u32 j = 0; j < 3; j++)
				{
					const FaceVertex& faceVertex = faceVertices[face.Vertices[j]];

					faceData.InnerVertices[j] = face.Vertices[j];
					faceData.OuterVertices[j] = faceVertex.OuterIdx;
					faceData.Normals[j] = faceVertex.Normal;
				}

				// Add a link on the source tetrahedron to the face data
				Tetrahedron& innerTet = volume.Tetrahedra[face.Tetrahedron];
				for(u32 j = 0; j < 4; j++)
				{
					if(innerTet.Neighbors[j] == -1)
					{
						// Note: Not searching for opposite neighbor here. If tet. has multiple free faces then we
						// can't just pick the first one
						innerTet.Neighbors[j] = (u32)volume.Tetrahedra.size() + (u32)faces.size();
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

				const Vector3& p0 = positions[faceData.InnerVertices[0]];
				const Vector3& p1 = positions[faceData.InnerVertices[1]];
				const Vector3& p2 = positions[faceData.InnerVertices[2]];

				const Vector3& v0 = faceVertices[faceData.InnerVertices[0]].Normal;
				const Vector3& v1 = faceVertices[faceData.InnerVertices[1]].Normal;
				const Vector3& v2 = faceVertices[faceData.InnerVertices[2]].Normal;

				float p =
					v2.X * v1.Y * v0.Z -
					v1.X * v2.Y * v0.Z -
					v2.X * v0.Y * v1.Z +
					v0.X * v2.Y * v1.Z +
					v1.X * v0.Y * v2.Z -
					v0.X * v1.Y * v2.Z;

				float qx = -v1.Y * v0.Z + v2.Y * v0.Z + v0.Y * v1.Z - v2.Y * v1.Z - v0.Y * v2.Z + v1.Y * v2.Z;
				float qy = v1.X * v0.Z - v2.X * v0.Z - v0.X * v1.Z + v2.X * v1.Z + v0.X * v2.Z - v1.X * v2.Z;
				float qz = -v1.X * v0.Y + v2.X * v0.Y + v0.X * v1.Y - v2.X * v1.Y - v0.X * v2.Y + v1.X * v2.Y;
				float qw = v2.Y * v1.Z * p0.X - v1.Y * v2.Z * p0.X - v2.Y * v0.Z * p1.X + v0.Y * v2.Z * p1.X +
					v1.Y * v0.Z * p2.X - v0.Y * v1.Z * p2.X - v2.X * v1.Z * p0.Y + v1.X * v2.Z * p0.Y +
					v2.X * v0.Z * p1.Y - v0.X * v2.Z * p1.Y - v1.X * v0.Z * p2.Y + v0.X * v1.Z * p2.Y +
					v2.X * v1.Y * p0.Z - v1.X * v2.Y * p0.Z - v2.X * v0.Y * p1.Z + v0.X * v2.Y * p1.Z +
					v1.X * v0.Y * p2.Z - v0.X * v1.Y * p2.Z;

				float rx = v1.Z * p0.Y - v2.Z * p0.Y - v0.Z * p1.Y + v2.Z * p1.Y + v0.Z * p2.Y - v1.Z * p2.Y -
					v1.Y * p0.Z + v2.Y * p0.Z + v0.Y * p1.Z - v2.Y * p1.Z - v0.Y * p2.Z + v1.Y * p2.Z;
				float ry = -v1.Z * p0.X + v2.Z * p0.X + v0.Z * p1.X - v2.Z * p1.X - v0.Z * p2.X + v1.Z * p2.X +
					v1.X * p0.Z - v2.X * p0.Z - v0.X * p1.Z + v2.X * p1.Z + v0.X * p2.Z - v1.X * p2.Z;
				float rz = v1.Y * p0.X - v2.Y * p0.X - v0.Y * p1.X + v2.Y * p1.X + v0.Y * p2.X - v1.Y * p2.X -
					v1.X * p0.Y + v2.X * p0.Y + v0.X * p1.Y - v2.X * p1.Y - v0.X * p2.Y + v1.X * p2.Y;
				float rw = v2.Z * p1.X * p0.Y - v1.Z * p2.X * p0.Y - v2.Z * p0.X * p1.Y + v0.Z * p2.X * p1.Y +
					v1.Z * p0.X * p2.Y - v0.Z * p1.X * p2.Y - v2.Y * p1.X * p0.Z + v1.Y * p2.X * p0.Z +
					v2.X * p1.Y * p0.Z - v1.X * p2.Y * p0.Z + v2.Y * p0.X * p1.Z - v0.Y * p2.X * p1.Z -
					v2.X * p0.Y * p1.Z + v0.X * p2.Y * p1.Z - v1.Y * p0.X * p2.Z + v0.Y * p1.X * p2.Z +
					v1.X * p0.Y * p2.Z - v0.X * p1.Y * p2.Z;

				float sx = -p1.Y * p0.Z + p2.Y * p0.Z + p0.Y * p1.Z - p2.Y * p1.Z - p0.Y * p2.Z + p1.Y * p2.Z;
				float sy = p1.X * p0.Z - p2.X * p0.Z - p0.X * p1.Z + p2.X * p1.Z + p0.X * p2.Z - p1.X * p2.Z;
				float sz = -p1.X * p0.Y + p2.X * p0.Y + p0.X * p1.Y - p2.X * p1.Y - p0.X * p2.Y + p1.X * p2.Y;
				float sw = p2.X * p1.Y * p0.Z - p1.X * p2.Y * p0.Z - p2.X * p0.Y * p1.Z +
					p0.X * p2.Y * p1.Z + p1.X * p0.Y * p2.Z - p0.X * p1.Y * p2.Z;

				faceData.Transform[0][0] = qx;
				faceData.Transform[0][1] = qy;
				faceData.Transform[0][2] = qz;
				faceData.Transform[0][3] = qw;

				faceData.Transform[1][0] = rx;
				faceData.Transform[1][1] = ry;
				faceData.Transform[1][2] = rz;
				faceData.Transform[1][3] = rw;

				faceData.Transform[2][0] = sx;
				faceData.Transform[2][1] = sy;
				faceData.Transform[2][2] = sz;
				faceData.Transform[2][3] = sw;

				// Unused
				faceData.Transform[3][0] = 0.0f;
				faceData.Transform[3][1] = 0.0f;
				faceData.Transform[3][2] = 0.0f;
				faceData.Transform[3][3] = 0.0f;

				if(fabs(p) > 0.00001f)
				{
					faceData.Transform = faceData.Transform * (1.0f / p);
					faceData.Quadratic = false;
				}
				else // Quadratic
				{
					faceData.Quadratic = true;
				}

				faces.push_back(faceData);
			}
		}
		else
		{
			for(u32 i = 0; i < (u32)volume.OuterFaces.size(); ++i)
			{
				const TetrahedronFace& face = volume.OuterFaces[i];
				TetrahedronFaceData faceData;

				for(u32 j = 0; j < 3; j++)
				{
					faceData.InnerVertices[j] = face.Vertices[j];
					faceData.OuterVertices[j] = -1;
					faceData.Normals[j] = Vector3::kZero;
				}

				faceData.Tetrahedron = face.Tetrahedron;
				faceData.Transform = Matrix4::kIdentity;
				faceData.Quadratic = false;

				faces.push_back(faceData);
			}
		}

		// Generate matrices
		u32 numOutputTets = (u32)volume.Tetrahedra.size();
		tetrahedra.reserve(numOutputTets);

		//// For inner tetrahedrons
		for(u32 i = 0; i < (u32)numOutputTets; ++i)
		{
			TetrahedronData entry;
			entry.Volume = volume.Tetrahedra[i];

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

			const Vector3& P1 = positions[volume.Tetrahedra[i].Vertices[0]];
			const Vector3& P2 = positions[volume.Tetrahedra[i].Vertices[1]];
			const Vector3& P3 = positions[volume.Tetrahedra[i].Vertices[2]];
			const Vector3& P4 = positions[volume.Tetrahedra[i].Vertices[3]];

			Vector3 E1 = P1 - P4;
			Vector3 E2 = P2 - P4;
			Vector3 E3 = P3 - P4;

			Matrix4 mat;
			mat.SetColumn(0, Vector4(E1, 0.0f));
			mat.SetColumn(1, Vector4(E2, 0.0f));
			mat.SetColumn(2, Vector4(E3, 0.0f));
			mat.SetColumn(3, Vector4(P4, 1.0f));

			entry.Transform = mat.Inverse();

			tetrahedra.push_back(entry);
		}
	}
	B3DClearAllocatorFrame();
}
}}
