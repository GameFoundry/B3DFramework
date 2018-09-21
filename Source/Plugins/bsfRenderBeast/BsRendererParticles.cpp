//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererParticles.h"
#include "Particles/BsParticleManager.h"
#include "Renderer/BsRendererUtility.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "Mesh/BsMeshData.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Shading/BsGpuParticleSimulation.h"
#include "Material/BsGpuParamsSet.h"
#include "BsRendererView.h"

namespace bs { namespace ct
{
	template<bool LOCK_Y, bool GPU, bool IS_3D, ParticleForwardLightingType FWD>
	const ShaderVariation& _getParticleShaderVariation(ParticleOrientation orient)
	{
		switch (orient)
		{
		default:
		case ParticleOrientation::ViewPlane:
			return getParticleShaderVariation<ParticleOrientation::ViewPlane, LOCK_Y, GPU, IS_3D, FWD>();
		case ParticleOrientation::ViewPosition:
			return getParticleShaderVariation<ParticleOrientation::ViewPosition, LOCK_Y, GPU, IS_3D, FWD>();
		case ParticleOrientation::Plane:
			return getParticleShaderVariation<ParticleOrientation::Plane, LOCK_Y, GPU, IS_3D, FWD>();
		}
	}

	template<bool GPU, bool IS_3D, ParticleForwardLightingType FWD>
	const ShaderVariation& _getParticleShaderVariation(ParticleOrientation orient, bool lockY)
	{
		if (lockY)
			return _getParticleShaderVariation<true, GPU, IS_3D, FWD>(orient);

		return _getParticleShaderVariation<false, GPU, IS_3D, FWD>(orient);
	}

	template<bool IS_3D, ParticleForwardLightingType FWD>
	const ShaderVariation& _getParticleShaderVariation(ParticleOrientation orient, bool lockY, bool gpu)
	{
		if(gpu)
			return _getParticleShaderVariation<true, IS_3D, FWD>(orient, lockY);

		return _getParticleShaderVariation<false, IS_3D, FWD>(orient, lockY);
	}

	template<ParticleForwardLightingType FWD>
	const ShaderVariation& _getParticleShaderVariation(ParticleOrientation orient, bool lockY, bool gpu, bool is3D)
	{
		if(is3D)
			return _getParticleShaderVariation<true, FWD>(orient, lockY, gpu);

		return _getParticleShaderVariation<false, FWD>(orient, lockY, gpu);
	}
	
	const ShaderVariation& getParticleShaderVariation(ParticleOrientation orient, bool lockY, bool gpu, 
		bool is3D, ParticleForwardLightingType forwardLighting)
	{
		switch(forwardLighting)
		{
		default:
		case ParticleForwardLightingType::None: 
			return _getParticleShaderVariation<ParticleForwardLightingType::None>(orient, lockY, gpu, is3D);
		case ParticleForwardLightingType::Clustered:
			return _getParticleShaderVariation<ParticleForwardLightingType::Clustered>(orient, lockY, gpu, is3D);
		case ParticleForwardLightingType::Standard:
			return _getParticleShaderVariation<ParticleForwardLightingType::Standard>(orient, lockY, gpu, is3D);
		}
	}

	ParticlesParamDef gParticlesParamDef;
	GpuParticlesParamDef gGpuParticlesParamDef;

	void writeIndices(GpuBuffer* buffer, const Vector<UINT32>& input, UINT32 texSize)
	{
		const auto numParticles = (UINT32)input.size();
		if (numParticles == 0)
			return;

		auto* const indices = (UINT32*)buffer->lock(GBL_WRITE_ONLY_DISCARD);

		UINT32 idx = 0;
		for(auto& entry : input)
		{
			const UINT32 x = entry % texSize;
			const UINT32 y = entry / texSize;

			indices[idx++] = (x & 0xFFFF) | (y << 16);
		}

		buffer->unlock();
	}

	void RendererParticles::bindCPUSimulatedInputs(const ParticleRenderData* renderData, const RendererView& view) const
	{
		ParticleTexturePool& particlesTexPool = ParticleRenderer::instance().getTexturePool();

		const ParticleSystemSettings& settings = particleSystem->getSettings();
		UINT32 texSize;
		switch (settings.renderMode)
		{
		default:
		case ParticleRenderMode::Billboard:
		{
			const auto billboardRenderData = static_cast<const ParticleBillboardRenderData*>(renderData);
			const ParticleBillboardTextures* textures = particlesTexPool.alloc(*billboardRenderData);

			renderElement.paramsCPUBillboard.positionAndRotTexture.set(textures->positionAndRotation);
			renderElement.paramsCPUBillboard.colorTexture.set(textures->color);
			renderElement.paramsCPUBillboard.sizeAndFrameIdxTexture.set(textures->sizeAndFrameIdx);

			renderElement.indicesBuffer.set(textures->indices);
			texSize = textures->positionAndRotation->getProperties().getWidth();
		}
		break;
		case ParticleRenderMode::Mesh:
		{
			const auto meshRenderData = static_cast<const ParticleMeshRenderData*>(renderData);
			const ParticleMeshTextures* textures = particlesTexPool.alloc(*meshRenderData);

			renderElement.paramsCPUMesh.positionTexture.set(textures->position);
			renderElement.paramsCPUMesh.colorTexture.set(textures->color);
			renderElement.paramsCPUMesh.rotationTexture.set(textures->rotation);
			renderElement.paramsCPUMesh.sizeTexture.set(textures->size);

			renderElement.indicesBuffer.set(textures->indices);
			texSize = textures->position->getProperties().getWidth();
		}
		break;
		}

		renderElement.numParticles = renderData->numParticles;

		gParticlesParamDef.gTexSize.set(particlesParamBuffer, texSize);
		gParticlesParamDef.gBufferOffset.set(particlesParamBuffer, 0);

		SPtr<GpuParams> gpuParams = renderElement.params->getGpuParams();
		for (UINT32 j = 0; j < GPT_COUNT; j++)
		{
			const GpuParamBinding& binding = renderElement.perCameraBindings[j];
			if (binding.slot != (UINT32)-1)
				gpuParams->setParamBlockBuffer(binding.set, binding.slot, view.getPerViewBuffer());
		}
	}

	void RendererParticles::bindGPUSimulatedInputs(const GpuParticleResources& gpuSimResources, const RendererView& view) const
	{
		const GpuParticleStateTextures& gpuSimStateTextures = gpuSimResources.getCurrentState();
		const GpuParticleStaticTextures& gpuSimStaticTextures = gpuSimResources.getStaticTextures();
		const GpuParticleCurves& gpuCurves = gpuSimResources.getCurveTexture();
		const SPtr<GpuBuffer>& sortedIndices = gpuSimResources.getSortedIndices();

		renderElement.paramsGPU.positionTimeTexture.set(gpuSimStateTextures.positionAndTimeTex);
		renderElement.paramsGPU.sizeRotationTexture.set(gpuSimStaticTextures.sizeAndRotationTex);
		renderElement.paramsGPU.curvesTexture.set(gpuCurves.getTexture());
		renderElement.numParticles = gpuParticleSystem->getNumTiles() * GpuParticleResources::PARTICLES_PER_TILE;

		if (gpuParticleSystem->hasSortInfo())
		{
			renderElement.indicesBuffer.set(sortedIndices);
			gParticlesParamDef.gBufferOffset.set(particlesParamBuffer,
				gpuParticleSystem->getSortOffset());
		}
		else
		{
			renderElement.indicesBuffer.set(gpuParticleSystem->getParticleIndices());
			gParticlesParamDef.gBufferOffset.set(particlesParamBuffer, 0);
		}

		const UINT32 texSize = GpuParticleResources::TEX_SIZE;
		gParticlesParamDef.gTexSize.set(particlesParamBuffer, texSize);

		SPtr<GpuParams> gpuParams = renderElement.params->getGpuParams();
		for (UINT32 j = 0; j < GPT_COUNT; j++)
		{
			const GpuParamBinding& binding = renderElement.perCameraBindings[j];
			if (binding.slot != (UINT32)-1)
				gpuParams->setParamBlockBuffer(binding.set, binding.slot, view.getPerViewBuffer());
		}
	}

	ParticleTexturePool::~ParticleTexturePool()
	{
		for (auto& sizeEntry : mBillboardBufferList)
		{
			for (auto& entry : sizeEntry.second.buffers)
				mBillboardAlloc.destruct(entry);
		}

		for (auto& sizeEntry : mMeshBufferList)
		{
			for (auto& entry : sizeEntry.second.buffers)
				mMeshAlloc.destruct(entry);
		}
	}

	const ParticleBillboardTextures* ParticleTexturePool::alloc(const ParticleBillboardRenderData& simulationData)
	{
		const UINT32 size = simulationData.color.getWidth();

		const ParticleBillboardTextures* output = nullptr;
		BillboardBuffersPerSize& buffers = mBillboardBufferList[size];
		if (buffers.nextFreeIdx < (UINT32)buffers.buffers.size())
		{
			output = buffers.buffers[buffers.nextFreeIdx];
			buffers.nextFreeIdx++;
		}

		if (!output)
		{
			output = createNewBillboardTextures(size);
			buffers.nextFreeIdx++;
		}

		// Populate texture contents
		// Note: Perhaps instead of using write-discard here, we should track which frame has finished rendering and then
		// just use no-overwrite? write-discard will very likely allocate memory under the hood.
		output->positionAndRotation->writeData(simulationData.positionAndRotation, 0, 0, true);
		output->color->writeData(simulationData.color, 0, 0, true);
		output->sizeAndFrameIdx->writeData(simulationData.sizeAndFrameIdx, 0, 0, true);

		writeIndices(output->indices.get(), simulationData.indices, size);
		return output;
	}

	const ParticleMeshTextures* ParticleTexturePool::alloc(const ParticleMeshRenderData& simulationData)
	{
		const UINT32 size = simulationData.color.getWidth();

		const ParticleMeshTextures* output = nullptr;
		MeshBuffersPerSize& buffers = mMeshBufferList[size];
		if (buffers.nextFreeIdx < (UINT32)buffers.buffers.size())
		{
			output = buffers.buffers[buffers.nextFreeIdx];
			buffers.nextFreeIdx++;
		}

		if (!output)
		{
			output = createNewMeshTextures(size);
			buffers.nextFreeIdx++;
		}

		// Populate texture contents
		// Note: Perhaps instead of using write-discard here, we should track which frame has finished rendering and then
		// just use no-overwrite? write-discard will very likely allocate memory under the hood.
		output->position->writeData(simulationData.position, 0, 0, true);
		output->color->writeData(simulationData.color, 0, 0, true);
		output->size->writeData(simulationData.size, 0, 0, true);
		output->rotation->writeData(simulationData.rotation, 0, 0, true);

		writeIndices(output->indices.get(), simulationData.indices, size);
		return output;
	}

	void ParticleTexturePool::clear()
	{
		for(auto& buffers : mBillboardBufferList)
			buffers.second.nextFreeIdx = 0;

		for(auto& buffers : mMeshBufferList)
			buffers.second.nextFreeIdx = 0;
	}

	ParticleBillboardTextures* ParticleTexturePool::createNewBillboardTextures(UINT32 size)
	{
		ParticleBillboardTextures* output = mBillboardAlloc.construct<ParticleBillboardTextures>();

		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_2D;
		texDesc.width = size;
		texDesc.height = size;
		texDesc.usage = TU_DYNAMIC;

		texDesc.format = PF_RGBA32F;
		output->positionAndRotation = Texture::create(texDesc);

		texDesc.format = PF_RGBA8;
		output->color = Texture::create(texDesc);

		texDesc.format = PF_RGBA16F;
		output->sizeAndFrameIdx = Texture::create(texDesc);

		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.type = GBT_STANDARD;
		bufferDesc.elementCount = size * size;
		bufferDesc.format = BF_16X2U;

		output->indices = GpuBuffer::create(bufferDesc);

		mBillboardBufferList[size].buffers.push_back(output);
		return output;
	}

	ParticleMeshTextures* ParticleTexturePool::createNewMeshTextures(UINT32 size)
	{
		ParticleMeshTextures* output = mMeshAlloc.construct<ParticleMeshTextures>();

		TEXTURE_DESC texDesc;
		texDesc.type = TEX_TYPE_2D;
		texDesc.width = size;
		texDesc.height = size;
		texDesc.usage = TU_DYNAMIC;

		texDesc.format = PF_RGBA32F;
		output->position = Texture::create(texDesc);

		texDesc.format = PF_RGBA8;
		output->color = Texture::create(texDesc);

		texDesc.format = PF_RGBA16F;
		output->size = Texture::create(texDesc);

		texDesc.format = PF_RGBA16F;
		output->rotation = Texture::create(texDesc);

		GPU_BUFFER_DESC bufferDesc;
		bufferDesc.type = GBT_STANDARD;
		bufferDesc.elementCount = size * size;
		bufferDesc.format = BF_16X2U;

		output->indices = GpuBuffer::create(bufferDesc);

		mMeshBufferList[size].buffers.push_back(output);
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

	void ParticleRenderer::sortByDistance(const Vector3& refPoint, const PixelData& positions, UINT32 numParticles, 
		UINT32 stride, Vector<UINT32>& indices)
	{
		struct ParticleSortData
		{
			ParticleSortData(float key, UINT32 idx)
				:key(key), idx(idx)
			{ }

			float key;
			UINT32 idx;
		};

		const UINT32 size = positions.getWidth();
		UINT8* positionPtr = positions.getData();

		bs_frame_mark();
		{
			FrameVector<ParticleSortData> sortData;
			sortData.reserve(numParticles);

			UINT32 x = 0;
			for (UINT32 i = 0; i < numParticles; i++)
			{
				const Vector3& position = *(Vector3*)positionPtr;

				float distance = refPoint.squaredDistance(position);
				sortData.emplace_back(distance, i);

				positionPtr += sizeof(float) * stride;
				x++;

				if (x >= size)
				{
					x = 0;
					positionPtr += positions.getRowSkip();
				}
			}

			std::sort(sortData.begin(), sortData.end(),
				[](const ParticleSortData& lhs, const ParticleSortData& rhs)
			{
				return rhs.key < lhs.key;
			});

			for (UINT32 i = 0; i < numParticles; i++)
				indices[i] = sortData[i].idx;
		}
		bs_frame_clear();
	}

}}
