//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DRendererUtility.h"

#include "Image/B3DTexture.h"
#include "Mesh/B3DMesh.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Material/B3DMaterial.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "Material/B3DPass.h"
#include "RenderAPI/B3DGpuParameterSet.h"
#include "Utility/B3DShapeMeshes3D.h"
#include "Material/B3DShader.h"
#include "Renderer/B3DIBLUtility.h"
#include "Math/B3DAABox.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "RenderAPI/B3DGpuDeviceCapabilities.h"
#include "Components/B3DLight.h"

using namespace b3d;

namespace b3d { namespace render
{
RendererUtility::RendererUtility()
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	{
		TInlineArray<VertexElement, 8> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
		vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));

		mFullscreenQuadVertexDescription = B3DMakeShared<VertexDescription>(vertexElements);

		GpuBufferCreateInformation indexBufferCreateInformation;
		indexBufferCreateInformation.Type = GpuBufferType::Index;
		indexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		indexBufferCreateInformation.Index.Type = IT_32BIT;
		indexBufferCreateInformation.Index.Count = 6;

		mFullScreenQuadIB = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		vertexBufferCreateInformation.Vertex.ElementSize = mFullscreenQuadVertexDescription->GetVertexStride(0);
		vertexBufferCreateInformation.Vertex.Count = 4 * kNumQuadVbSlots;

		mFullScreenQuadVB = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

		const u32 indices[]{ 0, 1, 2, 1, 3, 2 };

		void* destinationMemory = mFullScreenQuadIB->Lock(0, sizeof(indices), GBL_WRITE_ONLY);
		memcpy(destinationMemory, indices, sizeof(indices));
		mFullScreenQuadIB->Unlock();
	}

	{
		TInlineArray<VertexElement, 8> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

		SPtr<VertexDescription> vertexDesc = B3DMakeShared<VertexDescription>(vertexElements);

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
		TInlineArray<VertexElement, 8> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

		SPtr<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(vertexElements);

		u32 numVertices = 0;
		u32 numIndices = 0;

		ShapeMeshes3D::GetNumElementsAaBox(numVertices, numIndices);
		SPtr<MeshData> meshData = B3DMakeShared<MeshData>(numVertices, numIndices, vertexDescription);

		u32* indexData = meshData->GetIndices32();
		u8* positionData = meshData->GetElementData(VES_POSITION);

		AABox localBox(-Vector3::kOne, Vector3::kOne);
		ShapeMeshes3D::SolidAaBox(localBox, positionData, nullptr, nullptr, 0, vertexDescription->GetVertexStride(), indexData, 0);

		mUnitBoxStencilMesh = Mesh::Create(meshData);
	}

	{
		u32 numSides = Light::kLightConeNumSides;
		u32 numSlices = Light::kLightConeNumSlices;

		TInlineArray<VertexElement, 8> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

		SPtr<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(vertexElements);

		u32 numVertices = numSides * numSlices * 2;
		u32 numIndices = ((numSides * 2) * (numSlices - 1) * 2) * 3;

		SPtr<MeshData> meshData = B3DMakeShared<MeshData>(numVertices, numIndices, vertexDescription);

		u32* indexData = meshData->GetIndices32();
		u8* positionData = meshData->GetElementData(VES_POSITION);
		u32 stride = vertexDescription->GetVertexStride();

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
		TInlineArray<VertexElement, 8> vertexElements;
		vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

		SPtr<VertexDescription> vertexDescription = B3DMakeShared<VertexDescription>(vertexElements);

		u32 numVertices = 0;
		u32 numIndices = 0;

		ShapeMeshes3D::GetNumElementsAaBox(numVertices, numIndices);
		SPtr<MeshData> meshData = B3DMakeShared<MeshData>(numVertices, numIndices, vertexDescription);

		u32* indexData = meshData->GetIndices32();
		u8* positionData = meshData->GetElementData(VES_POSITION);

		AABox localBox(-Vector3::kOne * 1500.0f, Vector3::kOne * 1500.0f);
		ShapeMeshes3D::SolidAaBox(localBox, positionData, nullptr, nullptr, 0, vertexDescription->GetVertexStride(), indexData, 0);

		mSkyBoxMesh = Mesh::Create(meshData);
	}
}

void RendererUtility::SetPass(GpuCommandBuffer& commandBuffer, const SPtr<Material>& material, u32 passIdx, u32 techniqueIdx)
{
	const SPtr<Pass>& pass = material->GetPass(passIdx, techniqueIdx);
	commandBuffer.SetGpuGraphicsPipelineState(pass->GetGraphicsPipelineState());
	commandBuffer.SetStencilReferenceValue(pass->GetStencilRefValue());
}

void RendererUtility::SetComputePass(GpuCommandBuffer& commandBuffer, const SPtr<Material>& material, u32 passIdx)
{
	const SPtr<Pass>& pass = material->GetPass(passIdx);
	commandBuffer.SetGpuComputePipelineState(pass->GetComputePipelineState());
}

void RendererUtility::SetPassParams(GpuCommandBuffer& commandBuffer, const SPtr<MaterialParameterAdapter>& params, u32 passIdx)
{
	const SPtr<GpuParameterSet>& gpuParams = params->GetGpuParameterSet(passIdx);
	if(gpuParams == nullptr)
		return;

	commandBuffer.SetGpuParameterSet(gpuParams);
}

void RendererUtility::Draw(GpuCommandBuffer& commandBuffer, const SPtr<MeshBase>& mesh, u32 numInstances)
{
	Draw(commandBuffer, mesh, mesh->GetProperties().SubMeshes[0], numInstances);
}

void RendererUtility::Draw(GpuCommandBuffer& commandBuffer, const SPtr<MeshBase>& mesh, const SubMesh& subMesh, u32 numInstances)
{
	SPtr<VertexData> vertexData = mesh->GetVertexData();

	commandBuffer.SetVertexDescription(mesh->GetVertexData()->VertexDescription);

	auto& vertexBuffers = vertexData->GetBuffers();
	if(vertexBuffers.size() > 0)
	{
		SPtr<GpuBuffer> buffers[B3D_MAX_BOUND_VERTEX_BUFFERS];

		u32 endSlot = 0;
		u32 startSlot = B3D_MAX_BOUND_VERTEX_BUFFERS;
		for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
		{
			if(iter->first >= B3D_MAX_BOUND_VERTEX_BUFFERS)
				B3D_EXCEPT(InvalidParametersException, "Buffer index out of range");

			startSlot = std::min(iter->first, startSlot);
			endSlot = std::max(iter->first, endSlot);
		}

		for(auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
		{
			buffers[iter->first - startSlot] = iter->second;
		}

		commandBuffer.SetVertexBuffers(startSlot, buffers, endSlot - startSlot + 1);
	}

	SPtr<GpuBuffer> indexBuffer = mesh->GetIndexBuffer();
	commandBuffer.SetIndexBuffer(indexBuffer);

	commandBuffer.SetDrawOperation(subMesh.DrawOp);

	u32 indexCount = subMesh.IndexCount;
	commandBuffer.DrawIndexed(subMesh.IndexOffset + mesh->GetIndexOffset(), indexCount, mesh->GetVertexOffset(), vertexData->VertexCount, numInstances);

	mesh->NotifyUsedOnGPU();
}

void RendererUtility::DrawMorph(GpuCommandBuffer& commandBuffer, const SPtr<MeshBase>& mesh, const SubMesh& subMesh, const SPtr<GpuBuffer>& morphVertices, const SPtr<VertexDescription>& morphVertexDescription)
{
	// Bind buffers and draw
	SPtr<VertexData> vertexData = mesh->GetVertexData();
	commandBuffer.SetVertexDescription(morphVertexDescription);

	auto& meshBuffers = vertexData->GetBuffers();
	SPtr<GpuBuffer> allBuffers[B3D_MAX_BOUND_VERTEX_BUFFERS];

	u32 endSlot = 0;
	u32 startSlot = B3D_MAX_BOUND_VERTEX_BUFFERS;
	for(auto iter = meshBuffers.begin(); iter != meshBuffers.end(); ++iter)
	{
		if(iter->first >= B3D_MAX_BOUND_VERTEX_BUFFERS)
			B3D_EXCEPT(InvalidParametersException, "Buffer index out of range");

		startSlot = std::min(iter->first, startSlot);
		endSlot = std::max(iter->first, endSlot);
	}

	startSlot = std::min(1U, startSlot);
	endSlot = std::max(1U, endSlot);

	for(auto iter = meshBuffers.begin(); iter != meshBuffers.end(); ++iter)
		allBuffers[iter->first - startSlot] = iter->second;

	allBuffers[1] = morphVertices;
	commandBuffer.SetVertexBuffers(startSlot, allBuffers, endSlot - startSlot + 1);

	SPtr<GpuBuffer> indexBuffer = mesh->GetIndexBuffer();
	commandBuffer.SetIndexBuffer(indexBuffer);

	commandBuffer.SetDrawOperation(subMesh.DrawOp);

	u32 indexCount = subMesh.IndexCount;
	commandBuffer.DrawIndexed(subMesh.IndexOffset + mesh->GetIndexOffset(), indexCount, mesh->GetVertexOffset(), vertexData->VertexCount, 1);

	mesh->NotifyUsedOnGPU();
}

void RendererUtility::Blit(GpuCommandBuffer& commandBuffer, const BlitInformation& blitInformation)
{
	if(!B3D_ENSURE(blitInformation.InputTexture != nullptr))
		return;

	if(!B3D_ENSURE(blitInformation.OutputRenderTarget != nullptr))
		return;

	// Convert input area to float coordinates
	auto& textureProperties = blitInformation.InputTexture->GetProperties();
	const Area2I& area = blitInformation.InputArea;

	Area2 inputAreaFloat((float)area.X, (float)area.Y, (float)area.Width, (float)area.Height);
	if(area.Width == 0 || area.Height == 0)
	{
		inputAreaFloat.X = 0.0f;
		inputAreaFloat.Y = 0.0f;
		inputAreaFloat.Width = (float)textureProperties.Width;
		inputAreaFloat.Height = (float)textureProperties.Height;
	}

	// Get appropriate material variation
	BlitMat* const blitMaterial = BlitMat::GetVariation(textureProperties.SampleCount, !blitInformation.IsDepth, blitInformation.UseFiltering, blitInformation.UseBlend, blitInformation.WriteAlpha);

	// Get GPU parameters and configure source texture
	const SPtr<GpuParameterSet> gpuParameters = blitMaterial->Prepare(blitInformation.InputTexture);

	// Begin render pass
	RenderPassCreateInformation renderPassInfo(blitInformation.OutputRenderTarget, gpuParameters, blitInformation.ReadOnlyMask, blitInformation.LoadMask);
	commandBuffer.BeginRenderPass(renderPassInfo);

	if(blitInformation.OutputArea.has_value())
		commandBuffer.SetViewport(blitInformation.OutputArea.value());

	if(blitInformation.ClearMask != RT_NONE)
	{
		if(blitInformation.OutputArea.has_value())
			commandBuffer.ClearViewport(blitInformation.ClearMask);
		else
			commandBuffer.ClearRenderTarget(blitInformation.ClearMask);
	}

	blitMaterial->Execute(commandBuffer, gpuParameters, inputAreaFloat, blitInformation.FlipUV);

	commandBuffer.EndRenderPass();
}

void RendererUtility::DrawScreenQuad(GpuCommandBuffer& commandBuffer, const Area2& uv, const Vector2I& textureSize, u32 numInstances, bool flipUV)
{
	// Note: Consider drawing the quad using a single large triangle for possibly better performance
	// Note2: Consider setting quad size in shader instead of rebuilding the mesh every time

	const GpuBackendConventions& rapiConventions = commandBuffer.GetGpuDevice().GetCapabilities().Conventions;
	Vector3 vertices[4];

	if(rapiConventions.NdcYAxis == GpuBackendConventions::Axis::Down)
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
	if((rapiConventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ flipUV)
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

	SPtr<MeshData> meshData = B3DMakeShared<MeshData>(4, 6, mFullscreenQuadVertexDescription);

	auto vecIter = meshData->GetVec3DataIter(VES_POSITION);
	for(u32 i = 0; i < 4; i++)
		vecIter.AddValue(vertices[i]);

	auto uvIter = meshData->GetVec2DataIter(VES_TEXCOORD);
	for(u32 i = 0; i < 4; i++)
		uvIter.AddValue(uvs[i]);

	u32 bufferSize = meshData->GetStreamSize(0);
	u8* srcVertBufferData = meshData->GetStreamData(0);

	void* destinationMemory = mFullScreenQuadVB->Lock(mNextQuadVBSlot * bufferSize, bufferSize, GBL_WRITE_ONLY_NO_OVERWRITE);
	memcpy(destinationMemory, srcVertBufferData, bufferSize);
	mFullScreenQuadVB->Unlock();

	commandBuffer.SetVertexDescription(mFullscreenQuadVertexDescription);
	commandBuffer.SetVertexBuffers(0, &mFullScreenQuadVB, 1);
	commandBuffer.SetIndexBuffer(mFullScreenQuadIB);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);
	commandBuffer.DrawIndexed(0, 6, mNextQuadVBSlot * 4, 4, numInstances);

	mNextQuadVBSlot = (mNextQuadVBSlot + 1) % kNumQuadVbSlots;
}

void RendererUtility::Clear(GpuCommandBuffer& commandBuffer, u32 value)
{
	ClearMaterial* clearMat = ClearMaterial::Get();
	clearMat->Execute(commandBuffer, value);
}

RendererUtility& GetRendererUtility()
{
	return RendererUtility::Instance();
}

void BlitMat::Initialize()
{
	mIsFiltered = mVariationParameters.GetI32("MODE") == 1;
}

SPtr<GpuParameterSet> BlitMat::Prepare(const SPtr<Texture>& source)
{
	SPtr<GpuParameterSet> gpuParameters = CreateGpuParameterSet();
	gpuParameters->SetSampledTexture("gSource", source);

	return gpuParameters;
}

void BlitMat::Execute(GpuCommandBuffer& commandBuffer, const SPtr<GpuParameterSet>& gpuParameters, const Area2& area, bool flipUV)
{
	B3D_PROFILE_RENDERER_MATERIAL

	Bind(commandBuffer, false);
	commandBuffer.SetGpuParameterSet(gpuParameters);

	if(!mIsFiltered)
		GetRendererUtility().DrawScreenQuad(commandBuffer, area, Vector2I(1, 1), 1, flipUV);
	else
		GetRendererUtility().DrawScreenQuad(commandBuffer, Area2(0, 0, 1, 1), Vector2I(1, 1), 1, flipUV);
}

BlitMat* BlitMat::GetVariation(u32 msaaCount, bool isColor, bool isFiltered, bool blend, bool writeAlpha)
{
	if(blend)
	{
		if(writeAlpha)
		{
			if(isFiltered)
				return Get(GetVariation<1, 1, true, true>());
			else
				return Get(GetVariation<1, 0, true, true>());
		}
		else
		{
			if(isFiltered)
				return Get(GetVariation<1, 1, true, false>());
			else
				return Get(GetVariation<1, 0, true, false>());
		}
	}

	if(msaaCount > 1)
	{
		if(isColor)
		{
			switch(msaaCount)
			{
			case 2:
				return Get(GetVariation<2, 0, false, false>());
			case 4:
				return Get(GetVariation<4, 0, false, false>());
			default:
			case 8:
				return Get(GetVariation<8, 0, false, false>());
			}
		}
		else
		{
			switch(msaaCount)
			{
			case 2:
				return Get(GetVariation<2, 2, false, false>());
			case 4:
				return Get(GetVariation<4, 2, false, false>());
			default:
			case 8:
				return Get(GetVariation<8, 2, false, false>());
			}
		}
	}
	else
	{
		if(isFiltered)
			return Get(GetVariation<1, 1, false, false>());
		else
			return Get(GetVariation<1, 0, false, false>());
	}
}

ClearUniformDefinition gClearUniformDefinition;

void ClearMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Params", mUniformBufferParameter);
}

void ClearMaterial::Execute(GpuCommandBuffer& commandBuffer, u32 value)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gClearUniformDefinition.AllocateTransient().Map();
	gClearUniformDefinition.gClearValue.Set(uniforms, value);

	mUniformBufferParameter.Set(uniforms);

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);
}

CompositeUniformDefinition gCompositeUniformDefinition;

void CompositeMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gSource", mSourceTextureParameter);
}

void CompositeMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, const SPtr<RenderTarget>& target, const Color& tint)
{
	B3D_PROFILE_RENDERER_MATERIAL

	GpuBufferMappedScope uniforms = gCompositeUniformDefinition.AllocateTransient().Map();
	gCompositeUniformDefinition.gTint.Set(uniforms, tint);

	mUniformBufferParameter.Set(uniforms);
	mSourceTextureParameter.Set(source);

	// Render
	commandBuffer.BeginRenderPass(RenderPassCreateInformation(target, mGpuParameterSet));

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

BicubicUpsampleUniformDefinition gBicubicUpsampleUniformDefinition;

void BicubicUpsampleMaterial::Initialize()
{
	mGpuParameterSet->GetUniformBufferParameter("Input", mUniformBufferParameter);
	mGpuParameterSet->GetSampledTextureParameter("gSource", mSourceTextureParameter);
}

void BicubicUpsampleMaterial::Execute(GpuCommandBuffer& commandBuffer, const SPtr<Texture>& source, const SPtr<RenderTarget>& target, const Color& tint)
{
	B3D_PROFILE_RENDERER_MATERIAL

	const TextureProperties& sourceProps = source->GetProperties();

	Vector2I texSize(sourceProps.Width, sourceProps.Height);
	Vector2 invPixelSize(1.0f / texSize.X, 1.0f / texSize.Y);
	Vector2 invTwoPixelSize(2.0f / texSize.X, 2.0f / texSize.Y);

	GpuBufferMappedScope uniforms = gBicubicUpsampleUniformDefinition.AllocateTransient().Map();
	gBicubicUpsampleUniformDefinition.gTint.Set(uniforms, tint);
	gBicubicUpsampleUniformDefinition.gTextureSize.Set(uniforms, texSize);
	gBicubicUpsampleUniformDefinition.gInvPixel.Set(uniforms, invPixelSize);
	gBicubicUpsampleUniformDefinition.gInvTwoPixels.Set(uniforms, invTwoPixelSize);

	mUniformBufferParameter.Set(uniforms);
	mSourceTextureParameter.Set(source);

	// Render
	commandBuffer.BeginRenderPass(RenderPassCreateInformation(target, mGpuParameterSet));

	Bind(commandBuffer);
	GetRendererUtility().DrawScreenQuad(commandBuffer);

	commandBuffer.EndRenderPass();
}

BicubicUpsampleMaterial* BicubicUpsampleMaterial::GetVariation(bool hermite)
{
	if(hermite)
		return Get(GetVariation<true>());

	return Get(GetVariation<false>());
}

}}
