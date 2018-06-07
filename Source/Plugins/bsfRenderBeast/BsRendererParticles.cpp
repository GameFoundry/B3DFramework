//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererParticles.h"
#include "Particles/BsParticleManager.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Mesh/BsMeshData.h"
#include "Renderer/BsRendererUtility.h"

namespace bs { namespace ct
{
	const ShaderVariation& getParticleShaderVariation(ParticleOrientation orient, bool lockY)
	{
		if (lockY)
		{
			switch (orient)
			{
			default:
			case ParticleOrientation::ViewPlane:
				return getParticleShaderVariation<ParticleOrientation::ViewPlane, true>();
			case ParticleOrientation::ViewPosition:
				return getParticleShaderVariation<ParticleOrientation::ViewPosition, true>();
			case ParticleOrientation::Axis:
				return getParticleShaderVariation<ParticleOrientation::Axis, true>();
			}
		}
		else
		{
			switch (orient)
			{
			default:
			case ParticleOrientation::ViewPlane:
				return getParticleShaderVariation<ParticleOrientation::ViewPlane, false>();
			case ParticleOrientation::ViewPosition:
				return getParticleShaderVariation<ParticleOrientation::ViewPosition, false>();
			case ParticleOrientation::Axis:
				return getParticleShaderVariation<ParticleOrientation::Axis, false>();
			}
		}
	}

	ParticlesParamDef gParticlesParamDef;

	ParticleTexturePool::~ParticleTexturePool()
	{
		for (auto& sizeEntry : mBufferList)
		{
			for (auto& entry : sizeEntry.second.buffers)
				mAlloc.destruct(entry);
		}
	}

	const ParticleTextures* ParticleTexturePool::alloc(const ParticleRenderData& renderData)
	{
		const UINT32 size = renderData.color.getWidth();

		const ParticleTextures* output = nullptr;
		BuffersPerSize& buffers = mBufferList[size];
		if (buffers.nextFreeIdx < (UINT32)buffers.buffers.size())
		{
			output = buffers.buffers[buffers.nextFreeIdx];
			buffers.nextFreeIdx++;
		}

		if (!output)
		{
			output = createNewTextures(size);
			buffers.nextFreeIdx++;
		}

		// Populate texture contents
		// Note: Perhaps instead of using write-discard here, we should track which frame has finished rendering and then
		// just use no-overwrite? write-discard will very likely allocate memory under the hood.
		output->positionAndRotation->writeData(renderData.positionAndRotation, 0, 0, true);
		output->color->writeData(renderData.color, 0, 0, true);
		output->size->writeData(renderData.size, 0, 0, true);

		return output;
	}

	void ParticleTexturePool::clear()
	{
		for(auto& buffers : mBufferList)
			buffers.second.nextFreeIdx = 0;
	}

	ParticleTextures* ParticleTexturePool::createNewTextures(UINT32 size)
	{
		ParticleTextures* output = mAlloc.construct<ParticleTextures>();

		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_2D;
		texDesc.width = size;
		texDesc.height = size;
		texDesc.usage = TU_DYNAMIC;

		texDesc.format = PF_RGBA32F;
		output->positionAndRotation = Texture::create(texDesc);

		texDesc.format = PF_RGBA8;
		output->color = Texture::create(texDesc);

		texDesc.format = PF_RG16F;
		output->size = Texture::create(texDesc);

		mBufferList[size].buffers.push_back(output);

		return output;
	}

	struct ParticleRenderer::Members
	{
		SPtr<VertexBuffer> billboardVB;
		SPtr<VertexDeclaration> billboardVD;
	};

	ParticleRenderer::ParticleRenderer()
		:m(bs_new<Members>())
	{
		SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
		vertexDesc->addVertElem(VET_FLOAT3, VES_POSITION);
		vertexDesc->addVertElem(VET_FLOAT2, VES_TEXCOORD);

		m->billboardVD = VertexDeclaration::create(vertexDesc);

		VERTEX_BUFFER_DESC vbDesc;
		vbDesc.numVerts = 4;
		vbDesc.vertexSize = m->billboardVD->getProperties().getVertexSize(0);
		m->billboardVB = VertexBuffer::create(vbDesc);

		MeshData meshData(4, 0, vertexDesc);
		auto vecIter = meshData.getVec3DataIter(VES_POSITION);
		vecIter.addValue(Vector3(-0.5f, -0.5f, 0.0f));
		vecIter.addValue(Vector3(-0.5f, 0.5f, 0.0f));
		vecIter.addValue(Vector3(0.5f, -0.5f, 0.0f));
		vecIter.addValue(Vector3(0.5f, 0.5f, 0.0f));

		auto uvIter = meshData.getVec2DataIter(VES_TEXCOORD);
		uvIter.addValue(Vector2(0.0f, 1.0f));
		uvIter.addValue(Vector2(0.0f, 0.0f));
		uvIter.addValue(Vector2(1.0f, 1.0f));
		uvIter.addValue(Vector2(1.0f, 0.0f));

		m->billboardVB->writeData(0, meshData.getStreamSize(0), meshData.getStreamData(0), BWT_DISCARD);
	}

	ParticleRenderer::~ParticleRenderer()
	{
		bs_delete(m);
	}

	void ParticleRenderer::drawBillboards(UINT32 count)
	{
		SPtr<VertexBuffer> vertexBuffers[] = { m->billboardVB };

		RenderAPI& rapi = RenderAPI::instance();
		rapi.setVertexDeclaration(m->billboardVD);
		rapi.setVertexBuffers(0, vertexBuffers, 1);
		rapi.setDrawOperation(DOT_TRIANGLE_STRIP);
		rapi.draw(0, 4, count);
	}
}}
