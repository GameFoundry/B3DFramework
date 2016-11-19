//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererUtility.h"
#include "BsRenderAPI.h"
#include "BsMesh.h"
#include "BsVertexDataDesc.h"
#include "BsMaterial.h"
#include "BsGpuParamsSet.h"
#include "BsPass.h"
#include "BsBlendState.h"
#include "BsDepthStencilState.h"
#include "BsRasterizerState.h"
#include "BsGpuParams.h"
#include "BsGpuParamDesc.h"
#include "BsGpuParamBlockBuffer.h"
#include "BsShapeMeshes3D.h"
#include "BsLight.h"
#include "BsShader.h"

namespace BansheeEngine
{
	RendererUtility::RendererUtility()
	{
		{
			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
			vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

			mFullScreenQuadMesh = MeshCore::create(4, 6, vertexDesc);
		}

		{
			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

			UINT32 numVertices = 0;
			UINT32 numIndices = 0;

			ShapeMeshes3D::getNumElementsSphere(3, numVertices, numIndices);
			SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc);

			UINT32* indexData = meshData->getIndices32();
			UINT8* positionData = meshData->getElementData(VES_POSITION);

			Sphere localSphere(Vector3::ZERO, 1.0f);
			ShapeMeshes3D::solidSphere(localSphere, positionData, nullptr, 0,
				vertexDesc->getVertexStride(), indexData, 0, 3);

			mPointLightStencilMesh = MeshCore::create(meshData);
		}

		{
			UINT32 numSides = LightCore::LIGHT_CONE_NUM_SIDES;
			UINT32 numSlices = LightCore::LIGHT_CONE_NUM_SLICES;

			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

			UINT32 numVertices = numSides * numSlices * 2;
			UINT32 numIndices = ((numSides * 2) * (numSlices - 1) * 2) * 3;

			SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc);

			UINT32* indexData = meshData->getIndices32();
			UINT8* positionData = meshData->getElementData(VES_POSITION);
			UINT32 stride = vertexDesc->getVertexStride();

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

			mSpotLightStencilMesh = MeshCore::create(meshData);
		}

		// TODO - When I add proper preprocessor support, merge these into a single material
		mResolveMat = bs_shared_ptr_new<ResolveMat>();
		mBlitMat = bs_shared_ptr_new<BlitMat>();
	}

	RendererUtility::~RendererUtility()
	{

	}

	void RendererUtility::setPass(const SPtr<MaterialCore>& material, UINT32 passIdx, UINT32 techniqueIdx)
	{
		RenderAPICore& rapi = RenderAPICore::instance();

		SPtr<PassCore> pass = material->getPass(passIdx, techniqueIdx);
		rapi.setGraphicsPipeline(pass->getGraphicsPipelineState());
		rapi.setStencilRef(pass->getStencilRefValue());
	}

	void RendererUtility::setComputePass(const SPtr<MaterialCore>& material, UINT32 passIdx)
	{
		RenderAPICore& rapi = RenderAPICore::instance();

		SPtr<PassCore> pass = material->getPass(passIdx);
		rapi.setComputePipeline(pass->getComputePipelineState());
	}

	void RendererUtility::setPassParams(const SPtr<GpuParamsSetCore>& params, UINT32 passIdx)
	{
		SPtr<GpuParamsCore> gpuParams = params->getGpuParams(passIdx);
		if (gpuParams == nullptr)
			return;

		RenderAPICore& rapi = RenderAPICore::instance();
		rapi.setGpuParams(gpuParams);
	}

	void RendererUtility::draw(const SPtr<MeshCoreBase>& mesh, const SubMesh& subMesh, UINT32 numInstances)
	{
		RenderAPICore& rapi = RenderAPICore::instance();
		SPtr<VertexData> vertexData = mesh->getVertexData();

		rapi.setVertexDeclaration(mesh->getVertexData()->vertexDeclaration);

		auto& vertexBuffers = vertexData->getBuffers();
		if (vertexBuffers.size() > 0)
		{
			SPtr<VertexBufferCore> buffers[BS_MAX_BOUND_VERTEX_BUFFERS];

			UINT32 endSlot = 0;
			UINT32 startSlot = BS_MAX_BOUND_VERTEX_BUFFERS;
			for (auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
			{
				if (iter->first >= BS_MAX_BOUND_VERTEX_BUFFERS)
					BS_EXCEPT(InvalidParametersException, "Buffer index out of range");

				startSlot = std::min(iter->first, startSlot);
				endSlot = std::max(iter->first, endSlot);
			}

			for (auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
			{
				buffers[iter->first - startSlot] = iter->second;
			}

			rapi.setVertexBuffers(startSlot, buffers, endSlot - startSlot + 1);
		}

		SPtr<IndexBufferCore> indexBuffer = mesh->getIndexBuffer();
		rapi.setIndexBuffer(indexBuffer);

		rapi.setDrawOperation(subMesh.drawOp);

		UINT32 indexCount = subMesh.indexCount;
		rapi.drawIndexed(subMesh.indexOffset + mesh->getIndexOffset(), indexCount, mesh->getVertexOffset(), 
			vertexData->vertexCount, numInstances);

		mesh->_notifyUsedOnGPU();
	}

	void RendererUtility::drawMorph(const SPtr<MeshCoreBase>& mesh, const SubMesh& subMesh, 
		const SPtr<VertexBufferCore>& morphVertices, const SPtr<VertexDeclarationCore>& morphVertexDeclaration)
	{
		// Bind buffers and draw
		RenderAPICore& rapi = RenderAPICore::instance();

		SPtr<VertexData> vertexData = mesh->getVertexData();
		rapi.setVertexDeclaration(morphVertexDeclaration);

		auto& meshBuffers = vertexData->getBuffers();
		SPtr<VertexBufferCore> allBuffers[BS_MAX_BOUND_VERTEX_BUFFERS];

		UINT32 endSlot = 0;
		UINT32 startSlot = BS_MAX_BOUND_VERTEX_BUFFERS;
		for (auto iter = meshBuffers.begin(); iter != meshBuffers.end(); ++iter)
		{
			if (iter->first >= BS_MAX_BOUND_VERTEX_BUFFERS)
				BS_EXCEPT(InvalidParametersException, "Buffer index out of range");

			startSlot = std::min(iter->first, startSlot);
			endSlot = std::max(iter->first, endSlot);
		}

		startSlot = std::min(1U, startSlot);
		endSlot = std::max(1U, endSlot);

		for (auto iter = meshBuffers.begin(); iter != meshBuffers.end(); ++iter)
			allBuffers[iter->first - startSlot] = iter->second;

		allBuffers[1] = morphVertices;
		rapi.setVertexBuffers(startSlot, allBuffers, endSlot - startSlot + 1);

		SPtr<IndexBufferCore> indexBuffer = mesh->getIndexBuffer();
		rapi.setIndexBuffer(indexBuffer);

		rapi.setDrawOperation(subMesh.drawOp);

		UINT32 indexCount = subMesh.indexCount;
		rapi.drawIndexed(subMesh.indexOffset + mesh->getIndexOffset(), indexCount, mesh->getVertexOffset(),
			vertexData->vertexCount, 1);

		mesh->_notifyUsedOnGPU();
	}

	void RendererUtility::blit(const SPtr<TextureCore>& texture, const Rect2I& area)
	{
		auto& texProps = texture->getProperties();
		SPtr<MaterialCore> mat;
		SPtr<GpuParamsSetCore> params;
		if (texProps.getNumSamples() > 1)
		{
			mat = mResolveMat->getMaterial();
			params = mResolveMat->getParamsSet();
			mResolveMat->setParameters(texture);
		}
		else
		{
			mat = mBlitMat->getMaterial();
			params = mBlitMat->getParamsSet();
			mBlitMat->setParameters(texture);
		}

		setPass(mat);
		setPassParams(params);

		Rect2 fArea((float)area.x, (float)area.y, (float)area.width, (float)area.height);
		if(area.width == 0 || area.height == 0)
		{
			fArea.x = 0.0f;
			fArea.y = 0.0f;
			fArea.width = (float)texProps.getWidth();
			fArea.height = (float)texProps.getHeight();
		}

		drawScreenQuad(fArea);
	}

	void RendererUtility::drawScreenQuad(const Rect2& uv, const Vector2I& textureSize, UINT32 numInstances)
	{
		// Note: Consider drawing the quad using a single large triangle for possibly better performance

		Vector3 vertices[4];
		vertices[0] = Vector3(-1.0f, 1.0f, 0.0f);
		vertices[1] = Vector3(1.0f, 1.0f, 0.0f);
		vertices[2] = Vector3(-1.0f, -1.0f, 0.0f);
		vertices[3] = Vector3(1.0f, -1.0f, 0.0f);

		Vector2 uvs[4];

		if (RenderAPICore::instance().getAPIInfo().getNDCYAxisDown())
		{
			uvs[0] = Vector2(uv.x, uv.y);
			uvs[1] = Vector2(uv.x + uv.width, uv.y);
			uvs[2] = Vector2(uv.x, uv.y + uv.height);
			uvs[3] = Vector2(uv.x + uv.width, uv.y + uv.height);
		}
		else
		{
			uvs[0] = Vector2(uv.x, uv.y + uv.height);
			uvs[1] = Vector2(uv.x + uv.width, uv.y + uv.height);
			uvs[2] = Vector2(uv.x, uv.y);
			uvs[3] = Vector2(uv.x + uv.width, uv.y);
		}

		for (int i = 0; i < 4; i++)
		{
			uvs[i].x /= (float)textureSize.x;
			uvs[i].y /= (float)textureSize.y;
		}

		SPtr<VertexDataDesc> vertexDesc = mFullScreenQuadMesh->getVertexDesc();
		SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(4, 6, vertexDesc);

		auto vecIter = meshData->getVec3DataIter(VES_POSITION);
		for (UINT32 i = 0; i < 4; i++)
			vecIter.addValue(vertices[i]);

		auto uvIter = meshData->getVec2DataIter(VES_TEXCOORD);
		for (UINT32 i = 0; i < 4; i++)
			uvIter.addValue(uvs[i]);

		auto indices = meshData->getIndices32();
		indices[0] = 0;
		indices[1] = 1;
		indices[2] = 2;
		indices[3] = 1;
		indices[4] = 3;
		indices[5] = 2;

		mFullScreenQuadMesh->writeSubresource(0, *meshData, true, false);
		draw(mFullScreenQuadMesh, mFullScreenQuadMesh->getProperties().getSubMesh(), numInstances);
	}

	RendererUtility& gRendererUtility()
	{
		return RendererUtility::instance();
	}

	BlitMat::BlitMat()
	{
		mSource = mMaterial->getParamTexture("gSource");
	}

	void BlitMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void BlitMat::setParameters(const SPtr<TextureCore>& source)
	{
		mSource.set(source);
		mMaterial->updateParamsSet(mParamsSet);
	}

	ResolveMat::ResolveMat()
	{
		mSource = mMaterial->getParamTexture("gSource");
		mMaterial->getParam("gNumSamples", mNumSamples);
	}

	void ResolveMat::_initDefines(ShaderDefines& defines)
	{
		// Do nothing
	}

	void ResolveMat::setParameters(const SPtr<TextureCore>& source)
	{
		mSource.set(source);

		UINT32 sampleCount = source->getProperties().getNumSamples();
		mNumSamples.set(sampleCount);

		mMaterial->updateParamsSet(mParamsSet);
	}
}