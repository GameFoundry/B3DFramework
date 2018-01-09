//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
			vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

			mFullScreenQuadMesh = Mesh::create(4, 6, vertexDesc);
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
			ShapeMeshes3D::solidSphere(localSphere, positionData, nullptr, nullptr, 0,
				vertexDesc->getVertexStride(), indexData, 0, 3);

			mUnitSphereStencilMesh = Mesh::create(meshData);
		}

		{
			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

			UINT32 numVertices = 0;
			UINT32 numIndices = 0;

			ShapeMeshes3D::getNumElementsAABox(numVertices, numIndices);
			SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc);

			UINT32* indexData = meshData->getIndices32();
			UINT8* positionData = meshData->getElementData(VES_POSITION);

			AABox localBox(-Vector3::ONE, Vector3::ONE);
			ShapeMeshes3D::solidAABox(localBox, positionData, nullptr, nullptr, 0,
				vertexDesc->getVertexStride(), indexData, 0);

			mUnitBoxStencilMesh = Mesh::create(meshData);
		}

		{
			UINT32 numSides = Light::LIGHT_CONE_NUM_SIDES;
			UINT32 numSlices = Light::LIGHT_CONE_NUM_SLICES;

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

			mSpotLightStencilMesh = Mesh::create(meshData);
		}

		{
			SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
			vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);

			UINT32 numVertices = 0;
			UINT32 numIndices = 0;

			ShapeMeshes3D::getNumElementsAABox(numVertices, numIndices);
			SPtr<MeshData> meshData = bs_shared_ptr_new<MeshData>(numVertices, numIndices, vertexDesc);

			UINT32* indexData = meshData->getIndices32();
			UINT8* positionData = meshData->getElementData(VES_POSITION);

			AABox localBox(-Vector3::ONE * 1500.0f, Vector3::ONE * 1500.0f);
			ShapeMeshes3D::solidAABox(localBox, positionData, nullptr, nullptr, 0,
									   vertexDesc->getVertexStride(), indexData, 0);

			mSkyBoxMesh = Mesh::create(meshData);
		}
	}

	RendererUtility::~RendererUtility()
	{ }

	void RendererUtility::setPass(const SPtr<Material>& material, UINT32 passIdx, UINT32 techniqueIdx)
	{
		RenderAPI& rapi = RenderAPI::instance();

		SPtr<Pass> pass = material->getPass(passIdx, techniqueIdx);
		rapi.setGraphicsPipeline(pass->getGraphicsPipelineState());
		rapi.setStencilRef(pass->getStencilRefValue());
	}

	void RendererUtility::setComputePass(const SPtr<Material>& material, UINT32 passIdx)
	{
		RenderAPI& rapi = RenderAPI::instance();

		SPtr<Pass> pass = material->getPass(passIdx);
		rapi.setComputePipeline(pass->getComputePipelineState());
	}

	void RendererUtility::setPassParams(const SPtr<GpuParamsSet>& params, UINT32 passIdx)
	{
		SPtr<GpuParams> gpuParams = params->getGpuParams(passIdx);
		if (gpuParams == nullptr)
			return;

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setGpuParams(gpuParams);
	}

	void RendererUtility::draw(const SPtr<MeshBase>& mesh, UINT32 numInstances)
	{
		draw(mesh, mesh->getProperties().getSubMesh(0), numInstances);
	}

	void RendererUtility::draw(const SPtr<MeshBase>& mesh, const SubMesh& subMesh, UINT32 numInstances)
	{
		RenderAPI& rapi = RenderAPI::instance();
		SPtr<VertexData> vertexData = mesh->getVertexData();

		rapi.setVertexDeclaration(mesh->getVertexData()->vertexDeclaration);

		auto& vertexBuffers = vertexData->getBuffers();
		if (vertexBuffers.size() > 0)
		{
			SPtr<VertexBuffer> buffers[BS_MAX_BOUND_VERTEX_BUFFERS];

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

		SPtr<IndexBuffer> indexBuffer = mesh->getIndexBuffer();
		rapi.setIndexBuffer(indexBuffer);

		rapi.setDrawOperation(subMesh.drawOp);

		UINT32 indexCount = subMesh.indexCount;
		rapi.drawIndexed(subMesh.indexOffset + mesh->getIndexOffset(), indexCount, mesh->getVertexOffset(), 
			vertexData->vertexCount, numInstances);

		mesh->_notifyUsedOnGPU();
	}

	void RendererUtility::drawMorph(const SPtr<MeshBase>& mesh, const SubMesh& subMesh, 
		const SPtr<VertexBuffer>& morphVertices, const SPtr<VertexDeclaration>& morphVertexDeclaration)
	{
		// Bind buffers and draw
		RenderAPI& rapi = RenderAPI::instance();

		SPtr<VertexData> vertexData = mesh->getVertexData();
		rapi.setVertexDeclaration(morphVertexDeclaration);

		auto& meshBuffers = vertexData->getBuffers();
		SPtr<VertexBuffer> allBuffers[BS_MAX_BOUND_VERTEX_BUFFERS];

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

		SPtr<IndexBuffer> indexBuffer = mesh->getIndexBuffer();
		rapi.setIndexBuffer(indexBuffer);

		rapi.setDrawOperation(subMesh.drawOp);

		UINT32 indexCount = subMesh.indexCount;
		rapi.drawIndexed(subMesh.indexOffset + mesh->getIndexOffset(), indexCount, mesh->getVertexOffset(),
			vertexData->vertexCount, 1);

		mesh->_notifyUsedOnGPU();
	}

	void RendererUtility::blit(const SPtr<Texture>& texture, const Rect2I& area, bool flipUV, bool isDepth)
	{
		auto& texProps = texture->getProperties();

		BlitMat* blitMat = BlitMat::getVariation(texProps.getNumSamples(), !isDepth);
		blitMat->setParameters(texture);
		blitMat->bind();

		Rect2 fArea((float)area.x, (float)area.y, (float)area.width, (float)area.height);
		if (area.width == 0 || area.height == 0)
		{
			fArea.x = 0.0f;
			fArea.y = 0.0f;
			fArea.width = (float)texProps.getWidth();
			fArea.height = (float)texProps.getHeight();
		}

		drawScreenQuad(fArea, Vector2I(1, 1), 1, flipUV);
	}

	void RendererUtility::drawScreenQuad(const Rect2& uv, const Vector2I& textureSize, UINT32 numInstances, bool flipUV)
	{
		// Note: Consider drawing the quad using a single large triangle for possibly better performance

		const RenderAPIInfo& rapiInfo = RenderAPI::instance().getAPIInfo();
		Vector3 vertices[4];

		if (rapiInfo.isFlagSet(RenderAPIFeatureFlag::NDCYAxisDown))
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
		if (rapiInfo.isFlagSet(RenderAPIFeatureFlag::UVYAxisUp) ^ flipUV)
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

		mFullScreenQuadMesh->writeData(*meshData, true, false);
		draw(mFullScreenQuadMesh, mFullScreenQuadMesh->getProperties().getSubMesh(), numInstances);
	}

	void RendererUtility::clear(UINT32 value)
	{
		ClearMat* clearMat = ClearMat::get();
		clearMat->execute(value);
	}

	RendererUtility& gRendererUtility()
	{
		return RendererUtility::instance();
	}

	BlitMat::BlitMat()
	{
		mParams->getTextureParam(GPT_FRAGMENT_PROGRAM, "gSource", mSource);
	}

	void BlitMat::setParameters(const SPtr<Texture>& source)
	{
		mSource.set(source);
	}

	BlitMat* BlitMat::getVariation(UINT32 msaaCount, bool isColor)
	{
		if (msaaCount > 1)
		{
			if(isColor)
			{
				switch(msaaCount)
				{
				case 2:
					return get(getVariation<2, true>());
				case 4:
					return get(getVariation<4, true>());
				default:
				case 8:
					return get(getVariation<8, true>());
				}
			}
			else
			{
				switch(msaaCount)
				{
				case 2:
					return get(getVariation<2, false>());
				case 4:
					return get(getVariation<4, false>());
				default:
				case 8:
					return get(getVariation<8, false>());
				}
			}
		}
		else
			return get(getVariation<1, true>());
	}

	ClearParamDef gClearParamDef;

	ClearMat::ClearMat()
	{
		mParamBuffer = gClearParamDef.createBuffer();
		mParams->setParamBlockBuffer("Params", mParamBuffer);
	}

	void ClearMat::execute(UINT32 value)
	{
		gClearParamDef.gClearValue.set(mParamBuffer, value);

		bind();
		gRendererUtility().drawScreenQuad();
	}
}}
