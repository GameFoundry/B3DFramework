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

namespace bs { namespace ct
{
	RendererUtility::RendererUtility()
	{
		{
			mFullscreenQuadVDesc = bs_shared_ptr_new<VertexDataDesc>();
			mFullscreenQuadVDesc->AddVertElem(VET_FLOAT3, VES_POSITION);
			mFullscreenQuadVDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);

			INDEX_BUFFER_DESC ibDesc;
			ibDesc.indexType = IT_32BIT;
			ibDesc.numIndices = 6;
			ibDesc.usage = GBU_DYNAMIC;

			mFullScreenQuadIB = IndexBuffer::create(ibDesc);
			mFullscreenQuadVDecl = VertexDeclaration::create(mFullscreenQuadVDesc);

			VERTEX_BUFFER_DESC vbDesc;
			vbDesc.vertexSize = mFullscreenQuadVDecl->GetProperties().GetVertexSize(0);
			vbDesc.numVerts = 4 * NUM_QUAD_VB_SLOTS;
			vbDesc.usage = GBU_DYNAMIC;

			mFullScreenQuadVB = VertexBuffer::create(vbDesc);

			UINT32 indices[] { 0, 1, 2, 1, 3, 2 };
			mFullScreenQuadIB->WriteData(0, sizeof(indices), indices, BWT_DISCARD);
		}

		{
			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

			UINT32 numVertices = 0;
			UINT32 numIndices = 0;

			ShapeMeshes3D::getNumElementsSphere(3, numVertices, numIndices);
			SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc);

			UINT32* indexData = meshData->GetIndices32();
			UINT8* positionData = meshData->GetElementData(VES_POSITION);

			Sphere LocalSphere(Vector3::ZERO, 1.0f);
			ShapeMeshes3D::solidSphere(localSphere, positionData, nullptr, nullptr, 0,
				vertexDesc->GetVertexStride(), indexData, 0, 3);

			mUnitSphereStencilMesh = Mesh::create(meshData);
		}

		{
			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

			UINT32 numVertices = 0;
			UINT32 numIndices = 0;

			ShapeMeshes3D::getNumElementsAABox(numVertices, numIndices);
			SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc);

			UINT32* indexData = meshData->GetIndices32();
			UINT8* positionData = meshData->GetElementData(VES_POSITION);

			AABox LocalBox(-Vector3::ONE, Vector3::ONE);
			ShapeMeshes3D::solidAABox(localBox, positionData, nullptr, nullptr, 0,
				vertexDesc->GetVertexStride(), indexData, 0);

			mUnitBoxStencilMesh = Mesh::create(meshData);
		}

		{
			UINT32 numSides = Light::LIGHT_CONE_NUM_SIDES;
			UINT32 numSlices = Light::LIGHT_CONE_NUM_SLICES;

			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

			UINT32 numVertices = numSides * numSlices * 2;
			UINT32 numIndices = ((numSides * 2) * (numSlices - 1) * 2) * 3;

			SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc);

			UINT32* indexData = meshData->GetIndices32();
			UINT8* positionData = meshData->GetElementData(VES_POSITION);
			UINT32 stride = vertexDesc->GetVertexStride();

			// Dummy vertex positions, actual ones generated in shader
			for (UINT32 i = 0; i < numVertices; i++)
			{
				memcpy(positionData, &Vector3::ZERO, sizeof(Vector3));
				positionData += stride;
			}

			// Cone indices
			UINT32 curIdx = 0;
			for (UINT32 sliceIdx = 0; sliceIdx < (numSlices - 1); sliceIdx++)
			{
				for (UINT32 sideIdx = 0; sideIdx < numSides; sideIdx++)
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
			UINT32 coneOffset = numSides * numSlices;
			for (UINT32 sliceIdx = 0; sliceIdx < (numSlices - 1); sliceIdx++)
			{
				for (UINT32 sideIdx = 0; sideIdx < numSides; sideIdx++)
				{
					indexData[curIdx++] = coneOffset + sliceIdx * numSides + sideIdx;
					indexData[curIdx++] = coneOffset + sliceIdx * numSides + (sideIdx + 1) % numSides;
					indexData[curIdx++] = coneOffset + (sliceIdx + 1) * numSides + sideIdx;

					indexData[curIdx++] = coneOffset + sliceIdx * numSides + (sideIdx + 1) % numSides;
					indexData[curIdx++] = coneOffset + (sliceIdx + 1) * numSides + (sideIdx + 1) % numSides;
					indexData[curIdx++] = coneOffset + (sliceIdx + 1) * numSides + sideIdx;
				}
			}

			mSpotLightStencilMesh = Mesh::create(meshData);
		}

		{
			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);

			UINT32 numVertices = 0;
			UINT32 numIndices = 0;

			ShapeMeshes3D::getNumElementsAABox(numVertices, numIndices);
			SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc);

			UINT32* indexData = meshData->GetIndices32();
			UINT8* positionData = meshData->GetElementData(VES_POSITION);

			AABox LocalBox(-Vector3::ONE * 1500.0f, Vector3::ONE * 1500.0f);
			ShapeMeshes3D::solidAABox(localBox, positionData, nullptr, nullptr, 0,
									   vertexDesc->GetVertexStride(), indexData, 0);

			mSkyBoxMesh = Mesh::create(meshData);
		}
	}

	void RendererUtility::SetPass(const SPtr<Material>& material, UINT32 passIdx, UINT32 techniqueIdx)
	{
		RenderAPI& rapi = RenderAPI::instance();

		SPtr<Pass> pass = material->GetPass(passIdx, techniqueIdx);
		rapi.SetGraphicsPipeline(pass->GetGraphicsPipelineState());
		rapi.SetStencilRef(pass->GetStencilRefValue());
	}

	void RendererUtility::SetComputePass(const SPtr<Material>& material, UINT32 passIdx)
	{
		RenderAPI& rapi = RenderAPI::instance();

		SPtr<Pass> pass = material->GetPass(passIdx);
		rapi.SetComputePipeline(pass->GetComputePipelineState());
	}

	void RendererUtility::SetPassParams(const SPtr<GpuParamsSet>& params, UINT32 passIdx)
	{
		SPtr<GpuParams> gpuParams = params->GetGpuParams(passIdx);
		if (gpuParams == nullptr)
			return;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetGpuParams(gpuParams);
	}

	void RendererUtility::Draw(const SPtr<MeshBase>& mesh, UINT32 numInstances)
	{
		draw(mesh, mesh->GetProperties().GetSubMesh(0), numInstances);
	}

	void RendererUtility::Draw(const SPtr<MeshBase>& mesh, const SubMesh& subMesh, UINT32 numInstances)
	{
		RenderAPI& rapi = RenderAPI::instance();
		SPtr<VertexData> vertexData = mesh->GetVertexData();

		rapi.SetVertexDeclaration(mesh->GetVertexData()->vertexDeclaration);

		auto& vertexBuffers = vertexData->GetBuffers();
		if (vertexBuffers.Size() > 0)
		{
			SPtr<VertexBuffer> buffers[BS_MAX_BOUND_VERTEX_BUFFERS];

			UINT32 endSlot = 0;
			UINT32 startSlot = BS_MAX_BOUND_VERTEX_BUFFERS;
			for (auto iter = vertexBuffers.Begin(); iter != vertexBuffers.end(); ++iter)
			{
				if (iter->first >= BS_MAX_BOUND_VERTEX_BUFFERS)
					BS_EXCEPT(InvalidParametersException, "Buffer index out of range");

				startSlot = std::min(iter->first, startSlot);
				endSlot = std::max(iter->first, endSlot);
			}

			for (auto iter = vertexBuffers.Begin(); iter != vertexBuffers.end(); ++iter)
			{
				buffers[iter->first - startSlot] = iter->second;
			}

			rapi.SetVertexBuffers(startSlot, buffers, endSlot - startSlot + 1);
		}

		SPtr<IndexBuffer> indexBuffer = mesh->GetIndexBuffer();
		rapi.SetIndexBuffer(indexBuffer);

		rapi.SetDrawOperation(subMesh.drawOp);

		UINT32 indexCount = subMesh.indexCount;
		rapi.DrawIndexed(subMesh.indexOffset + mesh->GetIndexOffset(), indexCount, mesh->getVertexOffset(),
			vertexData->vertexCount, numInstances);

		mesh->_notifyUsedOnGPU();
	}

	void RendererUtility::drawMorph(const SPtr<MeshBase>& mesh, const SubMesh& subMesh,
		const SPtr<VertexBuffer>& morphVertices, const SPtr<VertexDeclaration>& morphVertexDeclaration)
	{
		// Bind buffers and draw
		RenderAPI& rapi = RenderAPI::instance();

		SPtr<VertexData> vertexData = mesh->GetVertexData();
		rapi.SetVertexDeclaration(morphVertexDeclaration);

		auto& meshBuffers = vertexData->GetBuffers();
		SPtr<VertexBuffer> allBuffers[BS_MAX_BOUND_VERTEX_BUFFERS];

		UINT32 endSlot = 0;
		UINT32 startSlot = BS_MAX_BOUND_VERTEX_BUFFERS;
		for (auto iter = meshBuffers.Begin(); iter != meshBuffers.end(); ++iter)
		{
			if (iter->first >= BS_MAX_BOUND_VERTEX_BUFFERS)
				BS_EXCEPT(InvalidParametersException, "Buffer index out of range");

			startSlot = std::min(iter->first, startSlot);
			endSlot = std::max(iter->first, endSlot);
		}

		startSlot = std::min(1U, startSlot);
		endSlot = std::max(1U, endSlot);

		for (auto iter = meshBuffers.Begin(); iter != meshBuffers.end(); ++iter)
			allBuffers[iter->first - startSlot] = iter->second;

		allBuffers[1] = morphVertices;
		rapi.SetVertexBuffers(startSlot, allBuffers, endSlot - startSlot + 1);

		SPtr<IndexBuffer> indexBuffer = mesh->GetIndexBuffer();
		rapi.SetIndexBuffer(indexBuffer);

		rapi.SetDrawOperation(subMesh.drawOp);

		UINT32 indexCount = subMesh.indexCount;
		rapi.DrawIndexed(subMesh.indexOffset + mesh->GetIndexOffset(), indexCount, mesh->getVertexOffset(),
			vertexData->vertexCount, 1);

		mesh->_notifyUsedOnGPU();
	}

	void RendererUtility::Blit(const SPtr<Texture>& texture, const Rect2I& area, bool flipUV, bool isDepth, bool isFiltered)
	{
		auto& texProps = texture->GetProperties();

		Rect2 FArea((float)area.x, (float)area.y, (float)area.width, (float)area.height);
		if (area.width == 0 || area.height == 0)
		{
			fArea.x = 0.0f;
			fArea.y = 0.0f;
			fArea.width = (float)texProps.GetWidth();
			fArea.height = (float)texProps.GetHeight();
		}

		BlitMat* blitMat = BlitMat::getVariation(texProps.GetNumSamples(), !isDepth, isFiltered);
		blitMat->Execute(texture, fArea, flipUV);
	}

	void RendererUtility::DrawScreenQuad(const Rect2& uv, const Vector2I& textureSize, UINT32 numInstances, bool flipUV)
	{
		// Note: Consider drawing the quad using a single large triangle for possibly better performance
		// Note2: Consider setting quad size in shader instead of rebuilding the mesh every time

		const Conventions& rapiConventions = gCaps().conventions;
		Vector3 vertices[4];

		if (rapiConventions.ndcYAxis == Conventions::Axis::Down)
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
		if ((rapiConventions.uvYAxis == Conventions::Axis::Up) ^ flipUV)
		{
			uvs[0] = Vector2(uv.x, uv.y + uv.height);
			uvs[1] = Vector2(uv.x + uv.width, uv.y + uv.height);
			uvs[2] = Vector2(uv.x, uv.y);
			uvs[3] = Vector2(uv.x + uv.width, uv.y);
		}
		else
		{
			uvs[0] = Vector2(uv.x, uv.y);
			uvs[1] = Vector2(uv.x + uv.width, uv.y);
			uvs[2] = Vector2(uv.x, uv.y + uv.height);
			uvs[3] = Vector2(uv.x + uv.width, uv.y + uv.height);
		}

		for (int i = 0; i < 4; i++)
		{
			uvs[i].x /= (float)textureSize.x;
			uvs[i].y /= (float)textureSize.y;
		}

		SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(4, 6, mFullscreenQuadVDesc);

		auto vecIter = meshData->GetVec3DataIter(VES_POSITION);
		for (UINT32 i = 0; i < 4; i++)
			vecIter.AddValue(vertices[i]);

		auto uvIter = meshData->GetVec2DataIter(VES_TEXCOORD);
		for (UINT32 i = 0; i < 4; i++)
			uvIter.AddValue(uvs[i]);

		UINT32 bufferSize = meshData->GetStreamSize(0);
		UINT8* srcVertBufferData = meshData->GetStreamData(0);

		void* dstData = mFullScreenQuadVB->Lock(mNextQuadVBSlot * bufferSize, bufferSize, GBL_WRITE_ONLY_NO_OVERWRITE);
		memcpy(dstData, srcVertBufferData, bufferSize);
		mFullScreenQuadVB->Unlock();

		RenderAPI& rapi = RenderAPI::instance();

		rapi.SetVertexDeclaration(mFullscreenQuadVDecl);
		rapi.SetVertexBuffers(0, &mFullScreenQuadVB, 1);
		rapi.SetIndexBuffer(mFullScreenQuadIB);
		rapi.SetDrawOperation(DOT_TRIANGLE_LIST);
		rapi.DrawIndexed(0, 6, mNextQuadVBSlot * 4, 4, numInstances);

		mNextQuadVBSlot = (mNextQuadVBSlot + 1) % NUM_QUAD_VB_SLOTS;
	}

	void RendererUtility::Clear(UINT32 value)
	{
		ClearMat* clearMat = ClearMat::get();
		clearMat->Execute(value);
	}

	RendererUtility& GRendererUtility()
	{
		return RendererUtility::Instance();
	}

	BlitMat::BlitMat()
	{
		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSource", mSource);
		mIsFiltered = mVariation.GetInt("MODE") == 1;
	}

	void BlitMat::Execute(const SPtr<Texture>& source, const Rect2& area, bool flipUV)
	{
		BS_RENMAT_PROFILE_BLOCK

		mSource.Set(source);
		bind();

		if(!mIsFiltered)
			gRendererUtility().DrawScreenQuad(area, Vector2I(1, 1), 1, flipUV);
		else
			gRendererUtility().DrawScreenQuad(Rect2(0, 0, 1, 1), Vector2I(1, 1), 1, flipUV);
	}

	BlitMat* BlitMat::getVariation(UINT32 msaaCount, bool isColor, bool isFiltered)
	{
		if (msaaCount > 1)
		{
			if(isColor)
			{
				switch (msaaCount)
				{
				case 2:
					return Get(getVariation<2, 0>());
				case 4:
					return Get(getVariation<4, 0>());
				default:
				case 8:
					return Get(getVariation<8, 0>());
				}
			}
			else
			{
				switch(msaaCount)
				{
				case 2:
					return Get(getVariation<2, 2>());
				case 4:
					return Get(getVariation<4, 2>());
				default:
				case 8:
					return Get(getVariation<8, 2>());
				}
			}
		}
		else
		{
			if(isFiltered)
				return Get(getVariation<1, 1>());
			else
				return Get(getVariation<1, 0>());
		}
	}

	ClearParamDef gClearParamDef;

	ClearMat::ClearMat()
	{
		mParamBuffer = gClearParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer("Params", mParamBuffer);
	}

	void ClearMat::Execute(UINT32 value)
	{
		BS_RENMAT_PROFILE_BLOCK

		gClearParamDef.gClearValue.Set(mParamBuffer, value);

		bind();
		gRendererUtility().DrawScreenQuad();
	}

	CompositeParamDef gCompositeParamDef;

	CompositeMat::CompositeMat()
	{
		mParamBuffer = gCompositeParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer("Input", mParamBuffer);

		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSource", mSourceTex);
	}

	void CompositeMat::Execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& target, const Color& tint)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mSourceTex.Set(source);

		gCompositeParamDef.gTint.Set(mParamBuffer, tint);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(target);

		bind();
		gRendererUtility().DrawScreenQuad();
	}

	BicubicUpsampleParamDef gBicubicUpsampleParamDef;

	BicubicUpsampleMat::BicubicUpsampleMat()
	{
		mParamBuffer = gBicubicUpsampleParamDef.CreateBuffer();
		mParams->SetParamBlockBuffer("Input", mParamBuffer);

		mParams->GetTextureParam(GPT_FRAGMENT_PROGRAM, "gSource", mSourceTex);
	}

	void BicubicUpsampleMat::Execute(const SPtr<Texture>& source, const SPtr<RenderTarget>& target, const Color& tint)
	{
		BS_RENMAT_PROFILE_BLOCK

		// Set parameters
		mSourceTex.Set(source);

		const TextureProperties& sourceProps = source->GetProperties();

		Vector2I TexSize(sourceProps.GetWidth(), sourceProps.getHeight());
		Vector2 InvPixelSize(1.0f / texSize.x, 1.0f / texSize.y);
		Vector2 InvTwoPixelSize(2.0f / texSize.x, 2.0f / texSize.y);

		gBicubicUpsampleParamDef.gTint.Set(mParamBuffer, tint);
		gBicubicUpsampleParamDef.gTextureSize.Set(mParamBuffer, texSize);
		gBicubicUpsampleParamDef.gInvPixel.Set(mParamBuffer, invPixelSize);
		gBicubicUpsampleParamDef.gInvTwoPixels.Set(mParamBuffer, invTwoPixelSize);

		// Render
		RenderAPI& rapi = RenderAPI::instance();
		rapi.SetRenderTarget(target);

		bind();
		gRendererUtility().DrawScreenQuad();
	}

	BicubicUpsampleMat* BicubicUpsampleMat::getVariation(bool hermite)
	{
		if (hermite)
			return Get(getVariation<true>());

		return Get(getVariation<false>());
	}

}}
