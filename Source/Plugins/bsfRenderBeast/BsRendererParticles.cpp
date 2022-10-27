//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsRendererParticles.h"
#include "Particles/BsParticleManager.h"
#include "Renderer/BsRendererUtility.h"
#include "RenderAPI/BsGpuBuffer.h"
#include "RenderAPI/BsVertexBuffer.h"
#include "Mesh/BsMeshData.h"
#include "Mesh/BsMesh.h"
#include "RenderAPI/BsVertexDataDesc.h"
#include "Shading/BsGpuParticleSimulation.h"
#include "Material/BsGpuParamsSet.h"
#include "BsRendererView.h"
#include "Mesh/BsMeshUtility.h"

namespace bs {
namespace ct {

template <bool LOCK_Y, bool GPU, bool IS_3D, ParticleForwardLightingType FWD>
const ShaderVariation& GetParticleShaderVariationInternal(ParticleOrientation orient)
{
	switch(orient)
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

template <bool GPU, bool IS_3D, ParticleForwardLightingType FWD>
const ShaderVariation& GetParticleShaderVariationInternal(ParticleOrientation orient, bool lockY)
{
	if(lockY)
		return GetParticleShaderVariationInternal<true, GPU, IS_3D, FWD>(orient);

	return GetParticleShaderVariationInternal<false, GPU, IS_3D, FWD>(orient);
}

template <bool IS_3D, ParticleForwardLightingType FWD>
const ShaderVariation& GetParticleShaderVariationInternal(ParticleOrientation orient, bool lockY, bool gpu)
{
	if(gpu)
		return GetParticleShaderVariationInternal<true, IS_3D, FWD>(orient, lockY);

	return GetParticleShaderVariationInternal<false, IS_3D, FWD>(orient, lockY);
}

template <ParticleForwardLightingType FWD>
const ShaderVariation& GetParticleShaderVariationInternal(ParticleOrientation orient, bool lockY, bool gpu, bool is3D)
{
	if(is3D)
		return GetParticleShaderVariationInternal<true, FWD>(orient, lockY, gpu);

	return GetParticleShaderVariationInternal<false, FWD>(orient, lockY, gpu);
}

const ShaderVariation& getParticleShaderVariation(ParticleOrientation orient, bool lockY, bool gpu, bool is3D, ParticleForwardLightingType forwardLighting)
{
	switch(forwardLighting)
	{
	default:
	case ParticleForwardLightingType::None:
		return GetParticleShaderVariationInternal<ParticleForwardLightingType::None>(orient, lockY, gpu, is3D);
	case ParticleForwardLightingType::Clustered:
		return GetParticleShaderVariationInternal<ParticleForwardLightingType::Clustered>(orient, lockY, gpu, is3D);
	case ParticleForwardLightingType::Standard:
		return GetParticleShaderVariationInternal<ParticleForwardLightingType::Standard>(orient, lockY, gpu, is3D);
	}
}

ParticlesParamDef gParticlesParamDef;
GpuParticlesParamDef gGpuParticlesParamDef;

void writeIndices(GpuBuffer* buffer, const Vector<u32>& input, u32 texSize)
{
	const auto numParticles = (u32)input.size();
	if(numParticles == 0)
		return;

	auto* const indices = (u32*)buffer->Lock(GBL_WRITE_ONLY_DISCARD);

	u32 idx = 0;
	for(auto& entry : input)
	{
		const u32 x = entry % texSize;
		const u32 y = entry / texSize;

		indices[idx++] = (x & 0xFFFF) | (y << 16);
	}

	buffer->Unlock();
}

void ParticlesRenderElement::Draw() const
{
	if(NumParticles > 0)
	{
		if(Is3D)
			gRendererUtility().Draw(Mesh, NumParticles);
		else
			ParticleRenderer::Instance().DrawBillboards(NumParticles);
	}
}

void RendererParticles::UpdatePerObjectBuffer()
{
	const ParticleSystemSettings& settings = ParticleSystem->GetSettings();
	const u32 layer = Bitwise::MostSignificantBit(ParticleSystem->GetLayer());
	Matrix4 localToWorldNoScale;
	if(settings.SimulationSpace == ParticleSimulationSpace::Local)
	{
		const Transform& tfrm = ParticleSystem->GetTransform();
		localToWorldNoScale = Matrix4::TRS(tfrm.GetPosition(), tfrm.GetRotation(), Vector3::ONE);
	}
	else
		localToWorldNoScale = Matrix4::IDENTITY;

	PerObjectBuffer::Update(PerObjectParamBuffer, LocalToWorld, localToWorldNoScale, PrevLocalToWorld, layer);
}

void RendererParticles::BindCpuSimulatedInputs(const ParticleRenderData* renderData, const RendererView& view) const
{
	ParticleTexturePool& particlesTexPool = ParticleRenderer::Instance().GetTexturePool();

	const ParticleSystemSettings& settings = ParticleSystem->GetSettings();
	u32 texSize;
	switch(settings.RenderMode)
	{
	default:
	case ParticleRenderMode::Billboard:
		{
			const auto billboardRenderData = static_cast<const ParticleBillboardRenderData*>(renderData);
			const ParticleBillboardTextures* textures = particlesTexPool.Alloc(*billboardRenderData);

			RenderElement.ParamsCpuBillboard.PositionAndRotTexture.Set(textures->PositionAndRotation);
			RenderElement.ParamsCpuBillboard.ColorTexture.Set(textures->Color);
			RenderElement.ParamsCpuBillboard.SizeAndFrameIdxTexture.Set(textures->SizeAndFrameIdx);

			RenderElement.IndicesBuffer.Set(textures->Indices);
			texSize = textures->PositionAndRotation->GetProperties().GetWidth();
		}
		break;
	case ParticleRenderMode::Mesh:
		{
			const auto meshRenderData = static_cast<const ParticleMeshRenderData*>(renderData);
			const ParticleMeshTextures* textures = particlesTexPool.Alloc(*meshRenderData);

			RenderElement.ParamsCpuMesh.PositionTexture.Set(textures->Position);
			RenderElement.ParamsCpuMesh.ColorTexture.Set(textures->Color);
			RenderElement.ParamsCpuMesh.RotationTexture.Set(textures->Rotation);
			RenderElement.ParamsCpuMesh.SizeTexture.Set(textures->Size);

			RenderElement.IndicesBuffer.Set(textures->Indices);
			texSize = textures->Position->GetProperties().GetWidth();
		}
		break;
	}

	RenderElement.NumParticles = renderData->NumParticles;

	gParticlesParamDef.gTexSize.Set(ParticlesParamBuffer, texSize);
	gParticlesParamDef.gBufferOffset.Set(ParticlesParamBuffer, 0);

	SPtr<GpuParams> gpuParams = RenderElement.Params->GetGpuParams();
	for(u32 j = 0; j < GPT_COUNT; j++)
	{
		const GpuParamBinding& binding = RenderElement.PerCameraBindings[j];
		if(binding.Slot != (u32)-1)
			gpuParams->SetParamBlockBuffer(binding.Set, binding.Slot, view.GetPerViewBuffer());
	}
}

void RendererParticles::BindGpuSimulatedInputs(const GpuParticleResources& gpuSimResources, const RendererView& view) const
{
	const GpuParticleStateTextures& gpuSimStateTextures = gpuSimResources.GetCurrentState();
	const GpuParticleStaticTextures& gpuSimStaticTextures = gpuSimResources.GetStaticTextures();
	const GpuParticleCurves& gpuCurves = gpuSimResources.GetCurveTexture();
	const SPtr<GpuBuffer>& sortedIndices = gpuSimResources.GetSortedIndices();

	RenderElement.ParamsGpu.PositionTimeTexture.Set(gpuSimStateTextures.PositionAndTimeTex);
	RenderElement.ParamsGpu.SizeRotationTexture.Set(gpuSimStaticTextures.SizeAndRotationTex);
	RenderElement.ParamsGpu.CurvesTexture.Set(gpuCurves.GetTexture());
	RenderElement.NumParticles = GpuParticleSystem->GetNumTiles() * GpuParticleResources::PARTICLES_PER_TILE;

	if(GpuParticleSystem->HasSortInfo())
	{
		RenderElement.IndicesBuffer.Set(sortedIndices);
		gParticlesParamDef.gBufferOffset.Set(ParticlesParamBuffer, GpuParticleSystem->GetSortOffset());
	}
	else
	{
		RenderElement.IndicesBuffer.Set(GpuParticleSystem->GetParticleIndices());
		gParticlesParamDef.gBufferOffset.Set(ParticlesParamBuffer, 0);
	}

	const u32 texSize = GpuParticleResources::TEX_SIZE;
	gParticlesParamDef.gTexSize.Set(ParticlesParamBuffer, texSize);

	SPtr<GpuParams> gpuParams = RenderElement.Params->GetGpuParams();
	for(u32 j = 0; j < GPT_COUNT; j++)
	{
		const GpuParamBinding& binding = RenderElement.PerCameraBindings[j];
		if(binding.Slot != (u32)-1)
			gpuParams->SetParamBlockBuffer(binding.Set, binding.Slot, view.GetPerViewBuffer());
	}
}

ParticleTexturePool::~ParticleTexturePool()
{
	for(auto& sizeEntry : mBillboardBufferList)
	{
		for(auto& entry : sizeEntry.second.Buffers)
			mBillboardAlloc.Destruct(entry);
	}

	for(auto& sizeEntry : mMeshBufferList)
	{
		for(auto& entry : sizeEntry.second.Buffers)
			mMeshAlloc.Destruct(entry);
	}
}

const ParticleBillboardTextures* ParticleTexturePool::Alloc(const ParticleBillboardRenderData& simulationData)
{
	const u32 size = simulationData.Color.GetWidth();

	const ParticleBillboardTextures* output = nullptr;
	BillboardBuffersPerSize& buffers = mBillboardBufferList[size];
	if(buffers.NextFreeIdx < (u32)buffers.Buffers.size())
	{
		output = buffers.Buffers[buffers.NextFreeIdx];
		buffers.NextFreeIdx++;
	}

	if(!output)
	{
		output = CreateNewBillboardTextures(size);
		buffers.NextFreeIdx++;
	}

	// Populate texture contents
	// Note: Perhaps instead of using write-discard here, we should track which frame has finished rendering and then
	// just use no-overwrite? write-discard will very likely allocate memory under the hood.
	output->PositionAndRotation->WriteData(simulationData.PositionAndRotation, 0, 0, true);
	output->Color->WriteData(simulationData.Color, 0, 0, true);
	output->SizeAndFrameIdx->WriteData(simulationData.SizeAndFrameIdx, 0, 0, true);

	writeIndices(output->Indices.get(), simulationData.Indices, size);
	return output;
}

const ParticleMeshTextures* ParticleTexturePool::Alloc(const ParticleMeshRenderData& simulationData)
{
	const u32 size = simulationData.Color.GetWidth();

	const ParticleMeshTextures* output = nullptr;
	MeshBuffersPerSize& buffers = mMeshBufferList[size];
	if(buffers.NextFreeIdx < (u32)buffers.Buffers.size())
	{
		output = buffers.Buffers[buffers.NextFreeIdx];
		buffers.NextFreeIdx++;
	}

	if(!output)
	{
		output = CreateNewMeshTextures(size);
		buffers.NextFreeIdx++;
	}

	// Populate texture contents
	// Note: Perhaps instead of using write-discard here, we should track which frame has finished rendering and then
	// just use no-overwrite? write-discard will very likely allocate memory under the hood.
	output->Position->WriteData(simulationData.Position, 0, 0, true);
	output->Color->WriteData(simulationData.Color, 0, 0, true);
	output->Size->WriteData(simulationData.Size, 0, 0, true);
	output->Rotation->WriteData(simulationData.Rotation, 0, 0, true);

	writeIndices(output->Indices.get(), simulationData.Indices, size);
	return output;
}

void ParticleTexturePool::Clear()
{
	for(auto& buffers : mBillboardBufferList)
		buffers.second.NextFreeIdx = 0;

	for(auto& buffers : mMeshBufferList)
		buffers.second.NextFreeIdx = 0;
}

ParticleBillboardTextures* ParticleTexturePool::CreateNewBillboardTextures(u32 size)
{
	ParticleBillboardTextures* output = mBillboardAlloc.Construct<ParticleBillboardTextures>();

	TEXTURE_DESC texDesc;
	texDesc.Type = TEX_TYPE_2D;
	texDesc.Width = size;
	texDesc.Height = size;
	texDesc.Usage = TU_DYNAMIC;

	texDesc.Format = PF_RGBA32F;
	output->PositionAndRotation = Texture::Create(texDesc);

	texDesc.Format = PF_RGBA8;
	output->Color = Texture::Create(texDesc);

	texDesc.Format = PF_RGBA16F;
	output->SizeAndFrameIdx = Texture::Create(texDesc);

	GPU_BUFFER_DESC bufferDesc;
	bufferDesc.Type = GBT_STANDARD;
	bufferDesc.ElementCount = size * size;
	bufferDesc.Format = BF_16X2U;

	output->Indices = GpuBuffer::Create(bufferDesc);

	mBillboardBufferList[size].Buffers.push_back(output);
	return output;
}

ParticleMeshTextures* ParticleTexturePool::CreateNewMeshTextures(u32 size)
{
	ParticleMeshTextures* output = mMeshAlloc.Construct<ParticleMeshTextures>();

	TEXTURE_DESC texDesc;
	texDesc.Type = TEX_TYPE_2D;
	texDesc.Width = size;
	texDesc.Height = size;
	texDesc.Usage = TU_DYNAMIC;

	texDesc.Format = PF_RGBA32F;
	output->Position = Texture::Create(texDesc);

	texDesc.Format = PF_RGBA8;
	output->Color = Texture::Create(texDesc);

	texDesc.Format = PF_RGBA16F;
	output->Size = Texture::Create(texDesc);

	texDesc.Format = PF_RGBA16F;
	output->Rotation = Texture::Create(texDesc);

	GPU_BUFFER_DESC bufferDesc;
	bufferDesc.Type = GBT_STANDARD;
	bufferDesc.ElementCount = size * size;
	bufferDesc.Format = BF_16X2U;

	output->Indices = GpuBuffer::Create(bufferDesc);

	mMeshBufferList[size].Buffers.push_back(output);
	return output;
}

struct ParticleRenderer::Members
{
	SPtr<VertexBuffer> BillboardVb;
	SPtr<VertexDeclaration> BillboardVd;
};

ParticleRenderer::ParticleRenderer()
	: m(bs_new<Members>())
{
	SPtr<VertexDataDesc> vertexDesc = bs_shared_ptr_new<VertexDataDesc>();
	vertexDesc->AddVertElem(VET_FLOAT3, VES_POSITION);
	vertexDesc->AddVertElem(VET_FLOAT2, VES_TEXCOORD);
	vertexDesc->AddVertElem(VET_UBYTE4_NORM, VES_NORMAL);
	vertexDesc->AddVertElem(VET_UBYTE4_NORM, VES_TANGENT);

	m->BillboardVd = VertexDeclaration::Create(vertexDesc);

	VERTEX_BUFFER_DESC vbDesc;
	vbDesc.NumVerts = 4;
	vbDesc.VertexSize = m->BillboardVd->GetProperties().GetVertexSize(0);
	m->BillboardVb = VertexBuffer::Create(vbDesc);

	MeshData meshData(4, 0, vertexDesc);
	auto vecIter = meshData.GetVec3DataIter(VES_POSITION);
	vecIter.AddValue(Vector3(-0.5f, -0.5f, 0.0f));
	vecIter.AddValue(Vector3(-0.5f, 0.5f, 0.0f));
	vecIter.AddValue(Vector3(0.5f, -0.5f, 0.0f));
	vecIter.AddValue(Vector3(0.5f, 0.5f, 0.0f));

	auto uvIter = meshData.GetVec2DataIter(VES_TEXCOORD);
	uvIter.AddValue(Vector2(0.0f, 1.0f));
	uvIter.AddValue(Vector2(0.0f, 0.0f));
	uvIter.AddValue(Vector2(1.0f, 1.0f));
	uvIter.AddValue(Vector2(1.0f, 0.0f));

	u32 stride = meshData.GetVertexDesc()->GetVertexStride(0);

	Vector3 normal = Vector3::UNIT_Y;
	Vector4 tangent(1.0f, 0.0f, 0.0f, 1.0f);

	u8* normalDst = meshData.GetElementData(VES_NORMAL);
	for(u32 i = 0; i < 4; i++)
	{
		MeshUtility::PackNormals(&normal, normalDst, 1, sizeof(Vector3), stride);
		normalDst += stride;
	}

	u8* tangentDst = meshData.GetElementData(VES_TANGENT);
	for(u32 i = 0; i < 4; i++)
	{
		MeshUtility::PackNormals(&tangent, tangentDst, 1, sizeof(Vector4), stride);
		tangentDst += stride;
	}

	m->BillboardVb->WriteData(0, meshData.GetStreamSize(0), meshData.GetStreamData(0), BWT_DISCARD);
}

ParticleRenderer::~ParticleRenderer()
{
	bs_delete(m);
}

void ParticleRenderer::DrawBillboards(u32 count)
{
	SPtr<VertexBuffer> vertexBuffers[] = { m->BillboardVb };

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.SetVertexDeclaration(m->BillboardVd);
	rapi.SetVertexBuffers(0, vertexBuffers, 1);
	rapi.SetDrawOperation(DOT_TRIANGLE_STRIP);
	rapi.Draw(0, 4, count);
}

void ParticleRenderer::SortByDistance(const Vector3& refPoint, const PixelData& positions, u32 numParticles, u32 stride, Vector<u32>& indices)
{
	struct ParticleSortData
	{
		ParticleSortData(float key, u32 idx)
			: Key(key), Idx(idx)
		{}

		float Key;
		u32 Idx;
	};

	const u32 size = positions.GetWidth();
	u8* positionPtr = positions.GetData();

	bs_frame_mark();
	{
		FrameVector<ParticleSortData> sortData;
		sortData.reserve(numParticles);

		u32 x = 0;
		for(u32 i = 0; i < numParticles; i++)
		{
			const Vector3& position = *(Vector3*)positionPtr;

			float distance = refPoint.SquaredDistance(position);
			sortData.emplace_back(distance, i);

			positionPtr += sizeof(float) * stride;
			x++;

			if(x >= size)
			{
				x = 0;
				positionPtr += positions.GetRowSkip();
			}
		}

		std::sort(sortData.begin(), sortData.end(), [](const ParticleSortData& lhs, const ParticleSortData& rhs)
				  { return rhs.Key < lhs.Key; });

		for(u32 i = 0; i < numParticles; i++)
			indices[i] = sortData[i].Idx;
	}
	bs_frame_clear();
}
}} // namespace bs::ct
