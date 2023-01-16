//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRendererUtility.h"
#include "RenderAPI/BsRenderAPI.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Material/BsMaterial.h"
#include "Material/BsGpuParamsSet.h"
#include "Material/BsPass.h"
#include "RenderAPI/BsGpuParams.h"
#include "Utility/BsShapeMeshes3D.h"
#include "Renderer/BsLight.h"
#include "Material/BsShader.h"
#include "Renderer/BsIBLUtility.h"
#include "Math/BsAABox.h"

using namespace bs;

namespace bs { namespace ct
{
RendererUtility::RendererUtility()
{
	{
		mFullscreenQuadVDesc = B3DMakeShared<VertexDataDesc>();
		mFullscreenQuadVDesc->AddVertElem(VET_FLOAT3, VES_POSITION);
		mFullscreenQuadVDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);

		IndexBufferCreateInformation ibDesc;
		ibDesc.IndexType = IT_32BIT;
		ibDesc.IndexCount = 6;
		ibDesc.Usage = GBU_DYNAMIC;

		mFullScreenQuadIB = IndexBuffer::Create(ibDesc);
		mFullscreenQuadVDecl = VertexDeclaration::Create(mFullscreenQuadVDesc);

		VertexBufferCreateInformation vbDesc;
		vbDesc.VertexSize = mFullscreenQuadVDecl->GetProperties().GetVertexSize(0);
		vbDesc.VertexCount = 4 * kNumQuadVbSlots;
		vbDesc.Usage = GBU_DYNAMIC;

		mFullScreenQuadVB = VertexBuffer::Create(vbDesc);

		u32 indices[]{ 0, 1, 2, 1, 3, 2 };
		mFullScreenQuadIB->WriteData(0, sizeof(indices), indices, BWT_DISCARD);
	}

	{
		SPtr<VertexDataDesc> vertexDesc = B3DMakeShared<VertexDataDesc>();
		vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

		u32 numVertices = 0;
		u32 numIndices = 0;

		ShapeMeshes3D::GetNumElementsSphere(3, numVertices, numIndices);
		SPtr<MeshData> meshData = B3DMakeShared<MeshData>(numVertices, numIndices, vertexDesc);

		u32* indexData = meshData->GetIndices32();
		u8* positionData = meshData->GetElementData(VES_POSITION);

		Sphere localSphere(Vector3::kZero, 1.0f);
		ShapeMeshes3D::SolidSphere(localSphere, positionData, nullptr, nullptr, 0, vertexDesc->GetVertexStride(), indexData, 0, 3);

		mUnitSphereStencilMesh = Mesh::Create(meshData);
	}

	{
		SPtr<VertexDataDesc> vertexDesc = B3DMakeShared<VertexDataDesc>();
		vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

		u32 numVertices = 0;
		u32 numIndices = 0;

		ShapeMeshes3D::GetNumElementsAaBox(numVertices, numIndices);
		SPtr<MeshData> meshData = B3DMakeShared<MeshData>(numVertices, numIndices, vertexDesc);

		u32* indexData = meshData->GetIndices32();
		u8* positionData = meshData->GetElementData(VES_POSITION);

		AABox localBox(-Vector3::kOne, Vector3::kOne);
		ShapeMeshes3D::SolidAaBox(localBox, positionData, nullptr, nullptr, 0, vertexDesc->GetVertexStride(), indexData, 0);

		mUnitBoxStencilMesh = Mesh::Create(meshData);
	}

	{
		u32 numSides = Light::kLightConeNumSides;
		u32 numSlices = Light::kLightConeNumSlices;

		SPtr<VertexDataDesc> vertexDesc = B3DMakeShared<VertexDataDesc>();
		vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

		u32 numVertices = numSides * numSlices * 2;
		u32 numIndices = ((numSides * 2) * (numSlices - 1) * 2) * 3;

		SPtr<MeshData> meshData = B3DMakeShared<MeshData>(numVertices, numIndices, vertexDesc);

		u32* indexData = meshData->GetIndices32();
		u8* positionData = meshData->GetElementData(VES_POSITION);
		u32 stride = vertexDesc->GetVertexStride();

		// Dummy vertex positions, actual ones generated in shader
		for(u32 i = 0; i < numVertices; i++)
		{
			memcpy(positionData, &Vector3::kZero, sizeof(Vector3));
			positionData += stride;
		}

		// Cone indices
		u32 curIdx = 0;
		for(u32 sliceIdx = 0; sliceIdx < (numSlices - 1); sliceIdx++)
		{
			for(u32 sideIdx = 0; sideIdx < numSides; sideIdx++)
			{
				indexData[curIdx++] = sliceIdx * numSides + sideIdx;
				indexData[curIdx++] = sliceIdx * numSides + (sideIdx + 1) % numSides;
				indexData[curIdx++] = (sliceIdx + 1) * numSides + sideIdx;

				indexData[curIdx++] = sliceIdx * numSides + (sideIdx + 1) % numSides;
				indexData[curIdx++] = (sliceIdx + 1) * numSides + (sideIdx + 1) % numSides;
				indexData[curIdx++] = (sliceIdx + 1) * numSides + sideIdx;
			}
		}

		// Sphere cap indices
		u32 coneOffset = numSides * numSlices;
		for(u32 sliceIdx = 0; sliceIdx < (numSlices - 1); sliceIdx++)
		{
			for(u32 sideIdx = 0; sideIdx < numSides; sideIdx++)
			{
				indexData[curIdx++] = coneOffset + sliceIdx * numSides + sideIdx;
				indexData[curIdx++] = coneOffset + sliceIdx * numSides + (sideIdx + 1) % numSides;
				indexData[curIdx++] = coneOffset + (sliceIdx + 1) * numSides + sideIdx;

				indexData[curIdx++] = coneOffset + sliceIdx * numSides + (sideIdx + 1) % numSides;
				indexData[curIdx++] = coneOffset + (sliceIdx + 1) * numSides + (sideIdx + 1) % numSides;
				indexData[curIdx++] = coneOffset + (sliceIdx + 1) * numSides + sideIdx;
			}
		}

		mSpotLightStencilMesh = Mesh::Create(meshData);
	}

	{
		SPtr<VertexDataDesc> vertexDesc = B3DMakeShared<VertexDataDesc>();
		vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

		u32 numVertices = 0;
		u32 numIndices = 0;

		ShapeMeshes3D::GetNumElementsAaBox(numVertices, numIndices);
		SPtr<MeshData> meshData = B3DMakeShared<MeshData>(numVertices, numIndices, vertexDesc);

		u32* indexData = meshData->GetIndices32();
		u8* positionData = meshData->GetElementData(VES_POSITION);

		AABox localBox(-Vector3::kOne * 1500.0f, Vector3::kOne * 1500.0f);
		ShapeMeshes3D::SolidAaBox(localBox, positionData, nullptr, nullptr, 0, vertexDesc->GetVertexStride(), indexData, 0);

		mSkyBoxMesh = Mesh::Create(meshData);
	}
}

void RendererUtility::SetPass(const SPtr<Material>& material, u32 passIdx, u32 techniqueIdx)
{
	RenderAPI& rapi = RenderAPI::Instance();

	SPtr<Pass> pass = material->GetPass(passIdx, techniqueIdx);
	rapi.SetGraphicsPipeline(pass->GetGraphicsPipelineState());
	rapi.SetStencilRef(pass->GetStencilRefValue());
}

void RendererUtility::SetComputePass(const SPtr<Material>& material, u32 passIdx)
{
	RenderAPI& rapi = RenderAPI::Instance();

	SPtr<Pass> pass = material->GetPass(passIdx);
	rapi.SetComputePipeline(pass->GetComputePipelineState());
}

void RendererUtility::SetPassParams(const SPtr<GpuParamsSet>& params, u32 passIdx)
{
	SPtr<GpuParams> gpuParams = params->GetGpuParams(passIdx);
	if(gpuParams == nullptr)
		return;

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetGpuParams(gpuParams);
}

void RendererUtility::Draw(const SPtr<MeshBase>& mesh, u32 numInstances)
{
	Draw(mesh, mesh->GetProperties().SubMeshes[0], numInstances);
}

void RendererUtility::Draw(const SPtr<MeshBase>& mesh, const SubMesh& subMesh, u32 numInstances)
{
	RenderAPI& rapi = RenderAPI::Instance();
	SPtr<VertexData> vertexData = mesh->GetVertexData();

	rapi.SetVertexDeclaration(mesh->GetVertexData()->VertexDeclaration);

	auto& vertexBuffers = vertexData->GetBuffers();
	if(vertexBuffers.size() > 0)
	{
		SPtr<VertexBuffer> buffers[BS_MAX_BOUND_VERTEX_BUFFERS];

		u32 endSlot = 0;
		u32 startSlot = BS_MAX_BOUND_VERTEX_BUFFERS;
		for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
		{
			if(iter->first >= BS_MAX_BOUND_VERTEX_BUFFERS)
				B3D_EXCEPT(InvalidParametersException, "Buffer index out of range");

			startSlot = std::min(iter->first, startSlot);
			endSlot = std::max(iter->first, endSlot);
		}

		for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
		{
			buffers[iter->first - startSlot] = iter->second;
		}

		rapi.SetVertexBuffers(startSlot, buffers, endSlot - startSlot + 1);
	}

	SPtr<IndexBuffer> indexBuffer = mesh->GetIndexBuffer();
	rapi.SetIndexBuffer(indexBuffer);

	rapi.SetDrawOperation(subMesh.DrawOp);

	u32 indexCount = subMesh.IndexCount;
	rapi.DrawIndexed(subMesh.IndexOffset + mesh->GetIndexOffset(), indexCount, mesh->GetVertexOffset(), vertexData->VertexCount, numInstances);

	mesh->NotifyUsedOnGPU();
}

void RendererUtility::DrawMorph(const SPtr<MeshBase>& mesh, const SubMesh& subMesh, const SPtr<VertexBuffer>& morphVertices, const SPtr<VertexDeclaration>& morphVertexDeclaration)
{
	// Bind buffers and draw
	RenderAPI& rapi = RenderAPI::Instance();

	SPtr<VertexData> vertexData = mesh->GetVertexData();
	rapi.SetVertexDeclaration(morphVertexDeclaration);

	auto& meshBuffers = vertexData->GetBuffers();
	SPtr<VertexBuffer> allBuffers[BS_MAX_BOUND_VERTEX_BUFFERS];

	u32 endSlot = 0;
	u32 startSlot = BS_MAX_BOUND_VERTEX_BUFFERS;
	for(auto iter = meshBuffers.begin(); iter != meshBuffers.end(); ++iter)
	{
		if(iter->first >= BS_MAX_BOUND_VERTEX_BUFFERS)
			B3D_EXCEPT(InvalidParametersException, "Buffer index out of range");

		startSlot = std::min(iter->first, startSlot);
		endSlot = std::max(iter->first, endSlot);
	}

	startSlot = std::min(1U, startSlot);
	endSlot = std::max(1U, endSlot);

	for(auto iter = meshBuffers.begin(); iter != meshBuffers.end(); ++iter)
		allBuffers[iter->first - startSlot] = iter->second;

	allBuffers[1] = morphVertices;
	rapi.SetVertexBuffers(startSlot, allBuffers, endSlot - startSlot + 1);

	SPtr<IndexBuffer> indexBuffer = mesh->GetIndexBuffer();
	rapi.SetIndexBuffer(indexBuffer);

	rapi.SetDrawOperation(subMesh.DrawOp);

	u32 indexCount = subMesh.IndexCount;
	rapi.DrawIndexed(subMesh.IndexOffset + mesh->GetIndexOffset(), indexCount, mesh->GetVertexOffset(), vertexData->VertexCount, 1);

	mesh->NotifyUsedOnGPU();
}

void RendererUtility::Blit(const SPtr<Texture>& texture, const Rect2I& area, bool flipUV, bool isDepth, bool isFiltered)
{
	auto& texProps = texture->GetProperties();

	Rect2 fArea((float)area.X, (float)area.Y, (float)area.Width, (float)area.Height);
	if(area.Width == 0 || area.Height == 0)
	{
		fArea.X = 0.0f;
		fArea.Y = 0.0f;
		fArea.Width = (float)texProps.Width;
		fArea.Height = (float)texProps.Height;
	}

	BlitMat* blitMat = BlitMat::GetVariation(texProps.SampleCount, !isDepth, isFiltered);
	blitMat->Execute(texture, fArea, flipUV);
}

void RendererUtility::DrawScreenQuad(const Rect2& uv, const Vector2I& textureSize, u32 numInstances, bool flipUV)
{
	// Note: Consider drawing the quad using a single large triangle for possibly better performance
	// Note2: Consider setting quad size in shader instead of rebuilding the mesh every time

	const Conventions& rapiConventions = GetRenderBackendCapabilities().Conventions;
	Vector3 vertices[4];

	if(rapiConventions.NdcYAxis == Conventions::Axis::Down)
	{
		vertices[0] = Vector3(-1.0f, -1.0f, 0.0f);
		vertices[1] = Vector3(1.0f, -1.0f, 0.0f);
		vertices[2] = Vector3(-1.0f, 1.0f, 0.0f);
		vertices[3] = Vector3(1.0f, 1.0f, 0.0f);
	}
	else
	{
		vertices[0] = Vector3(-1.0f, 1.0f, 0.0f);
		vertices[1] = Vector3(1.0f, 1.0f, 0.0f);
		vertices[2] = Vector3(-1.0f, -1.0f, 0.0f);
		vertices[3] = Vector3(1.0f, -1.0f, 0.0f);
	}

	Vector2 uvs[4];
	if((rapiConventions.UvYAxis == Conventions::Axis::Up) ^ flipUV)
	{
		uvs[0] = Vector2(uv.X, uv.Y + uv.Height);
		uvs[1] = Vector2(uv.X + uv.Width, uv.Y + uv.Height);
		uvs[2] = Vector2(uv.X, uv.Y);
		uvs[3] = Vector2(uv.X + uv.Width, uv.Y);
	}
	else
	{
		uvs[0] = Vector2(uv.X, uv.Y);
		uvs[1] = Vector2(uv.X + uv.Width, uv.Y);
		uvs[2] = Vector2(uv.X, uv.Y + uv.Height);
		uvs[3] = Vector2(uv.X + uv.Width, uv.Y + uv.Height);
	}

	for(int i = 0; i < 4; i++)
	{
		uvs[i].X /= (float)textureSize.X;
		uvs[i].Y /= (float)textureSize.Y;
	}

	SPtr<MeshData> meshData = B3DMakeShared<MeshData>(4, 6, mFullscreenQuadVDesc);

	auto vecIter = meshData->GetVec3DataIter(VES_POSITION);
	for(u32 i = 0; i < 4; i++)
		vecIter.AddValue(vertices[i]);

	auto uvIter = meshData->GetVec2DataIter(VES_TEXCOORD);
	for(u32 i = 0; i < 4; i++)
		uvIter.AddValue(uvs[i]);

	u32 bufferSize = meshData->GetStreamSize(0);
	u8* srcVertBufferData = meshData->GetStreamData(0);

	void* dstData = mFullScreenQuadVB->Lock(mNextQuadVBSlot * bufferSize, bufferSize, GBL_WRITE_ONLY_NO_OVERWRITE);
	memcpy(dstData, srcVertBufferData, bufferSize);
	mFullScreenQuadVB->Unlock();

	RenderAPI& rapi = RenderAPI::Instance();

	rapi.SetVertexDeclaration(mFullscreenQuadVDecl);
	rapi.SetVertexBuffers(0, &mFullScreenQuadVB, 1);
	rapi.SetIndexBuffer(mFullScreenQuadIB);
	rapi.SetDrawOperation(DOT_TRIANGLE_LIST);
	rapi.DrawIndexed(0, 6, mNextQuadVBSlot * 4, 4, numInstances);

	mNextQuadVBSlot = (mNextQuadVBSlot + 1) % kNumQuadVbSlots;
}

void RendererUtility::Clear(u32 value)
{
	ClearMat* clearMat = ClearMat::Get();
	clearMat->Execute(value);
}

RendererUtility& GetRendererUtility()
{
	return RendererUtility::Instance();
}

BlitMat::BlitMat()
{
	mParams->GetTextureParameter(GPT_FRAGMENT_PROGRAM, "gSource", mSource);
	mIsFiltered = mVariation.GetInt("MODE") == 1;
}

void BlitMat::Execute(const SPtr<Texture>& source, const Rect2& area, bool flipUV)
{
	BS_RENMAT_PROFILE_BLOCK

	mSource.Set(source);
	Bind();

	if(!mIsFiltered)
		GetRendererUtility().DrawScreenQuad(area, Vector2I(1, 1), 1, flipUV);
	else
		GetRendererUtility().DrawScreenQuad(Rect2(0, 0, 1, 1), Vector2I(1, 1), 1, flipUV);
}

BlitMat* BlitMat::GetVariation(u32 msaaCount, bool isColor, bool isFiltered)
{
	if(msaaCount > 1)
	{
		if(isColor)
		{
			switch(msaaCount)
			{
			case 2:
				return Get(GetVariation<2, 0>());
			case 4:
				return Get(GetVariation<4, 0>());
			default:
			case 8:
				return Get(GetVariation<8, 0>());
			}
		}
		else
		{
			switch(msaaCount)
			{
			case 2:
				return Get(GetVariation<2, 2>());
			case 4:
				return Get(GetVariation<4, 2>());
			default:
			case 8:
				return Get(GetVariation<8, 2>());
			}
		}
	}
	else
	{
		if(isFiltered)
			return Get(GetVariation<1, 1>());
		else
			return Get(GetVariation<1, 0>());
	}
}

ClearParamDef gClearParamDef;

ClearMat::ClearMat()
{
	mParamBuffer = gClearParamDef.CreateBuffer();
	mParams->SetParameterBlockBuffer("Params", mParamBuffer);
}

void ClearMat::Execute(u32 value)
{
	BS_RENMAT_PROFILE_BLOCK

	gClearParamDef.gClearValue.Set(mParamBuffer, value);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

CompositeParamDef gCompositeParamDef;

CompositeMat::CompositeMat()
{
	mParamBuffer = gCompositeParamDef.CreateBuffer();
	mParams->SetParameterBlockBuffer("Input", mParamBuffer);

	mParams->GetTextureParameter(GPT_FRAGMENT_PROGRAM, "gSource", mSourceTex);
}

void CompositeMat::Execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& target, const Color& tint)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mSourceTex.Set(source);

	gCompositeParamDef.gTint.Set(mParamBuffer, tint);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(target);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

BicubicUpsampleParamDef gBicubicUpsampleParamDef;

BicubicUpsampleMat::BicubicUpsampleMat()
{
	mParamBuffer = gBicubicUpsampleParamDef.CreateBuffer();
	mParams->SetParameterBlockBuffer("Input", mParamBuffer);

	mParams->GetTextureParameter(GPT_FRAGMENT_PROGRAM, "gSource", mSourceTex);
}

void BicubicUpsampleMat::Execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& target, const Color& tint)
{
	BS_RENMAT_PROFILE_BLOCK

	// Set parameters
	mSourceTex.Set(source);

	const TextureProperties& sourceProps = source->GetProperties();

	Vector2I texSize(sourceProps.Width, sourceProps.Height);
	Vector2 invPixelSize(1.0f / texSize.X, 1.0f / texSize.Y);
	Vector2 invTwoPixelSize(2.0f / texSize.X, 2.0f / texSize.Y);

	gBicubicUpsampleParamDef.gTint.Set(mParamBuffer, tint);
	gBicubicUpsampleParamDef.gTextureSize.Set(mParamBuffer, texSize);
	gBicubicUpsampleParamDef.gInvPixel.Set(mParamBuffer, invPixelSize);
	gBicubicUpsampleParamDef.gInvTwoPixels.Set(mParamBuffer, invTwoPixelSize);

	// Render
	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetRenderTarget(target);

	Bind();
	GetRendererUtility().DrawScreenQuad();
}

BicubicUpsampleMat* BicubicUpsampleMat::GetVariation(bool hermite)
{
	if(hermite)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

}}
