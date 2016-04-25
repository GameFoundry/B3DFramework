//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRendererUtility.h"
#include "BsRenderAPI.h"
#include "BsMesh.h"
#include "BsVertexDataDesc.h"
#include "BsMaterial.h"
#include "BsPass.h"
#include "BsBlendState.h"
#include "BsDepthStencilState.h"
#include "BsRasterizerState.h"
#include "BsGpuParams.h"
#include "BsGpuParamDesc.h"
#include "BsShapeMeshes3D.h"
#include "BsLight.h"

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

	void RendererUtility::setPass(const SPtr<MaterialCore>& material, UINT32 passIdx)
	{
		RenderAPICore& rs = RenderAPICore::instance();

		SPtr<PassCore> pass = material->getPass(passIdx);
		SPtr<PassParametersCore> passParams = material->getPassParameters(passIdx);

		struct StageData
		{
			GpuProgramType type;
			bool enable;
			SPtr<GpuParamsCore> params;
			SPtr<GpuProgramCore> program;
		};

		const UINT32 numStages = 5;
		StageData stages[numStages] =
		{
			{
				GPT_VERTEX_PROGRAM, pass->hasVertexProgram(),
				passParams->mVertParams, pass->getVertexProgram()
			},
			{
				GPT_FRAGMENT_PROGRAM, pass->hasFragmentProgram(),
				passParams->mFragParams, pass->getFragmentProgram()
			},
			{
				GPT_GEOMETRY_PROGRAM, pass->hasGeometryProgram(),
				passParams->mGeomParams, pass->getGeometryProgram()
			},
			{
				GPT_HULL_PROGRAM, pass->hasHullProgram(),
				passParams->mHullParams, pass->getHullProgram()
			},
			{
				GPT_DOMAIN_PROGRAM, pass->hasDomainProgram(),
				passParams->mDomainParams, pass->getDomainProgram()
			}
		};

		for (UINT32 i = 0; i < numStages; i++)
		{
			const StageData& stage = stages[i];

			if (stage.enable)
			{
				rs.bindGpuProgram(stage.program);
				rs.setGpuParams(stage.type, stage.params);

			}
			else
				rs.unbindGpuProgram(stage.type);
		}

		// Set up non-texture related pass settings
		if (pass->getBlendState() != nullptr)
			rs.setBlendState(pass->getBlendState());
		else
			rs.setBlendState(BlendStateCore::getDefault());

		if (pass->getDepthStencilState() != nullptr)
			rs.setDepthStencilState(pass->getDepthStencilState(), pass->getStencilRefValue());
		else
			rs.setDepthStencilState(DepthStencilStateCore::getDefault(), pass->getStencilRefValue());

		if (pass->getRasterizerState() != nullptr)
			rs.setRasterizerState(pass->getRasterizerState());
		else
			rs.setRasterizerState(RasterizerStateCore::getDefault());
	}

	void RendererUtility::setComputePass(const SPtr<MaterialCore>& material, UINT32 passIdx)
	{
		RenderAPICore& rs = RenderAPICore::instance();

		SPtr<PassCore> pass = material->getPass(passIdx);
		SPtr<PassParametersCore> passParams = material->getPassParameters(passIdx);

		if(pass->hasComputeProgram())
		{
			rs.bindGpuProgram(pass->getComputeProgram());
			rs.setGpuParams(GPT_COMPUTE_PROGRAM, passParams->mComputeParams);
		}
		else
			rs.unbindGpuProgram(GPT_COMPUTE_PROGRAM);
	}

	void RendererUtility::setPassParams(const SPtr<MaterialCore>& material, UINT32 passIdx)
	{
		const SPtr<PassParametersCore>& passParams = material->getPassParameters(passIdx);

		RenderAPICore& rs = RenderAPICore::instance();

		struct StageData
		{
			GpuProgramType type;
			SPtr<GpuParamsCore> params;
		};

		const UINT32 numStages = 6;
		StageData stages[numStages] =
		{
			{ GPT_VERTEX_PROGRAM, passParams->mVertParams },
			{ GPT_FRAGMENT_PROGRAM, passParams->mFragParams },
			{ GPT_GEOMETRY_PROGRAM, passParams->mGeomParams },
			{ GPT_HULL_PROGRAM, passParams->mHullParams },
			{ GPT_DOMAIN_PROGRAM, passParams->mDomainParams },
			{ GPT_COMPUTE_PROGRAM, passParams->mComputeParams }
		};

		for (UINT32 i = 0; i < numStages; i++)
		{
			const StageData& stage = stages[i];

			SPtr<GpuParamsCore> params = stage.params;
			if (params == nullptr)
				continue;

			const GpuParamDesc& paramDesc = params->getParamDesc();

			for (auto iter = paramDesc.samplers.begin(); iter != paramDesc.samplers.end(); ++iter)
			{
				SPtr<SamplerStateCore> samplerState = params->getSamplerState(iter->second.slot);

				if (samplerState == nullptr)
					rs.setSamplerState(stage.type, iter->second.slot, SamplerStateCore::getDefault());
				else
					rs.setSamplerState(stage.type, iter->second.slot, samplerState);
			}

			for (auto iter = paramDesc.textures.begin(); iter != paramDesc.textures.end(); ++iter)
			{
				SPtr<TextureCore> texture = params->getTexture(iter->second.slot);

				if (!params->isLoadStoreTexture(iter->second.slot))
				{
					if (texture == nullptr)
						rs.setTexture(stage.type, iter->second.slot, false, nullptr);
					else
						rs.setTexture(stage.type, iter->second.slot, true, texture);
				}
				else
				{
					const TextureSurface& surface = params->getLoadStoreSurface(iter->second.slot);

					if (texture == nullptr)
						rs.setLoadStoreTexture(stage.type, iter->second.slot, false, nullptr, surface);
					else
						rs.setLoadStoreTexture(stage.type, iter->second.slot, true, texture, surface);
				}
			}

			rs.setConstantBuffers(stage.type, params);
		}
	}

	void RendererUtility::draw(const SPtr<MeshCoreBase>& mesh, const SubMesh& subMesh)
	{
		RenderAPICore& rs = RenderAPICore::instance();
		SPtr<VertexData> vertexData = mesh->getVertexData();

		rs.setVertexDeclaration(vertexData->vertexDeclaration);
		auto& vertexBuffers = vertexData->getBuffers();

		if (vertexBuffers.size() > 0)
		{
			SPtr<VertexBufferCore> buffers[MAX_BOUND_VERTEX_BUFFERS];

			UINT32 endSlot = 0;
			UINT32 startSlot = MAX_BOUND_VERTEX_BUFFERS;
			for (auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
			{
				if (iter->first >= MAX_BOUND_VERTEX_BUFFERS)
					BS_EXCEPT(InvalidParametersException, "Buffer index out of range");

				startSlot = std::min(iter->first, startSlot);
				endSlot = std::max(iter->first, endSlot);
			}

			for (auto iter = vertexBuffers.begin(); iter != vertexBuffers.end(); ++iter)
			{
				buffers[iter->first - startSlot] = iter->second;
			}

			rs.setVertexBuffers(startSlot, buffers, endSlot - startSlot + 1);
		}

		rs.setDrawOperation(subMesh.drawOp);

		SPtr<IndexBufferCore> indexBuffer = mesh->getIndexBuffer();

		UINT32 indexCount = subMesh.indexCount;

		rs.setIndexBuffer(indexBuffer);
		rs.drawIndexed(subMesh.indexOffset + mesh->getIndexOffset(), indexCount, mesh->getVertexOffset(), vertexData->vertexCount);

		mesh->_notifyUsedOnGPU();
	}

	void RendererUtility::blit(const SPtr<TextureCore>& texture, const Rect2I& area)
	{
		auto& texProps = texture->getProperties();
		SPtr<MaterialCore> mat;
		if (texProps.getMultisampleCount() > 1)
		{
			mat = mResolveMat->getMaterial();
			mResolveMat->setParameters(texture);
		}
		else
		{
			mat = mBlitMat->getMaterial();
			mBlitMat->setParameters(texture);
		}

		setPass(mat, 0);
		setPassParams(mat);

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

	void RendererUtility::drawScreenQuad(const Rect2& uv, const Vector2I& textureSize)
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
		draw(mFullScreenQuadMesh, mFullScreenQuadMesh->getProperties().getSubMesh());
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

		UINT32 sampleCount = source->getProperties().getMultisampleCount();
		mNumSamples.set(sampleCount);
	}
}