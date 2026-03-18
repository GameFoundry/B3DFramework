//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DParticleRenderState.h"
#include "B3DRenderBeast.h"
#include "Particles/B3DParticleScene.h"
#include "Renderer/B3DRendererUtility.h"
#include "Mesh/B3DMeshData.h"
#include "Mesh/B3DMesh.h"
#include "RenderAPI/B3DVertexDescription.h"
#include "Shading/B3DGpuParticleSimulation.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "B3DRendererView.h"
#include "Mesh/B3DMeshUtility.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"
#include "Utility/B3DBitwise.h"
#include "Image/B3DSpriteTexture.h"
#include "Image/B3DTexture.h"

namespace b3d {
namespace render {

template <bool LOCK_Y, bool GPU, bool IS_3D, ParticleForwardLightingType FWD>
const ShaderVariationParameters& GetParticleShaderVariationInternal(ParticleOrientation orient)
{
	switch(orient)
	{
	default:
	case ParticleOrientation::ViewPlane:
		return GetParticleShaderVariation<ParticleOrientation::ViewPlane, LOCK_Y, GPU, IS_3D, FWD>();
	case ParticleOrientation::ViewPosition:
		return GetParticleShaderVariation<ParticleOrientation::ViewPosition, LOCK_Y, GPU, IS_3D, FWD>();
	case ParticleOrientation::Plane:
		return GetParticleShaderVariation<ParticleOrientation::Plane, LOCK_Y, GPU, IS_3D, FWD>();
	}
}

template <bool GPU, bool IS_3D, ParticleForwardLightingType FWD>
const ShaderVariationParameters& GetParticleShaderVariationInternal(ParticleOrientation orient, bool lockY)
{
	if(lockY)
		return GetParticleShaderVariationInternal<true, GPU, IS_3D, FWD>(orient);

	return GetParticleShaderVariationInternal<false, GPU, IS_3D, FWD>(orient);
}

template <bool IS_3D, ParticleForwardLightingType FWD>
const ShaderVariationParameters& GetParticleShaderVariationInternal(ParticleOrientation orient, bool lockY, bool gpu)
{
	if(gpu)
		return GetParticleShaderVariationInternal<true, IS_3D, FWD>(orient, lockY);

	return GetParticleShaderVariationInternal<false, IS_3D, FWD>(orient, lockY);
}

template <ParticleForwardLightingType FWD>
const ShaderVariationParameters& GetParticleShaderVariationInternal(ParticleOrientation orient, bool lockY, bool gpu, bool is3D)
{
	if(is3D)
		return GetParticleShaderVariationInternal<true, FWD>(orient, lockY, gpu);

	return GetParticleShaderVariationInternal<false, FWD>(orient, lockY, gpu);
}

const ShaderVariationParameters& GetParticleShaderVariationParameters(ParticleOrientation orient, bool lockY, bool gpu, bool is3D, ParticleForwardLightingType forwardLighting)
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

ParticlesUniformDefinition gParticlesUniformDefinition;
GpuParticlesUniformDefinition gGpuParticlesUniformDefinition;

void WriteIndices(const SPtr<GpuBuffer>& buffer, const Vector<u32>& input, u32 texSize)
{
	const auto numParticles = (u32)input.size();
	if(numParticles == 0)
		return;

	auto* const indices = (u32*)B3DStackAllocate(buffer->GetTotalSize());

	u32 idx = 0;
	for(auto& entry : input)
	{
		const u32 x = entry % texSize;
		const u32 y = entry / texSize;

		indices[idx++] = (x & 0xFFFF) | (y << 16);
	}

	GpuBufferUtility::Write(buffer, 0, buffer->GetTotalSize(), indices, GpuBufferWriteFlag::Discard);
	B3DStackFree(indices);
}

void ParticlesDrawCommand::Draw(GpuCommandBuffer& commandBuffer) const
{
	if(NumParticles > 0)
	{
		if(IsGpuSimulated)
		{
			const u32 gpuParticlesDynamicOffsetIndex = GetRenderBeast()->GetGpuParticlesParameterSetInfo().GpuParticlesDynamicOffsetIndex;
			commandBuffer.SetDynamicBufferOffset(GpuPipelineSet::kPerObject, gpuParticlesDynamicOffsetIndex, GpuParticlesParamBufferOffset);
		}

		if(Is3D)
			GetRendererUtility().Draw(commandBuffer, Mesh, NumParticles);
		else
			ParticleRenderer::Instance().DrawBillboards(commandBuffer, NumParticles);
	}
}

void ParticleRenderState::BindCpuSimulatedInputs(const ParticleRenderData* renderData, const RendererView& view) const
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

			DrawCommand.ParamsCpuBillboard.PositionAndRotTexture.Set(textures->PositionAndRotation);
			DrawCommand.ParamsCpuBillboard.ColorTexture.Set(textures->Color);
			DrawCommand.ParamsCpuBillboard.SizeAndFrameIdxTexture.Set(textures->SizeAndFrameIdx);

			DrawCommand.IndicesBuffer.Set(textures->Indices);
			texSize = textures->PositionAndRotation->GetProperties().Width;
		}
		break;
	case ParticleRenderMode::Mesh:
		{
			const auto meshRenderData = static_cast<const ParticleMeshRenderData*>(renderData);
			const ParticleMeshTextures* textures = particlesTexPool.Alloc(*meshRenderData);

			DrawCommand.ParamsCpuMesh.PositionTexture.Set(textures->Position);
			DrawCommand.ParamsCpuMesh.ColorTexture.Set(textures->Color);
			DrawCommand.ParamsCpuMesh.RotationTexture.Set(textures->Rotation);
			DrawCommand.ParamsCpuMesh.SizeTexture.Set(textures->Size);

			DrawCommand.IndicesBuffer.Set(textures->Indices);
			texSize = textures->Position->GetProperties().Width;
		}
		break;
	}

	DrawCommand.NumParticles = renderData->NumParticles;

	PopulateAndBindParticlesUniformBuffer(texSize, 0);
	DrawCommand.PerCameraUniformBufferParameter.Set(view.GetPerViewBuffer());
}

void ParticleRenderState::BindGpuSimulatedInputs(const GpuParticleResources& gpuSimResources, const RendererView& view) const
{
	const GpuParticleStateTextures& gpuSimStateTextures = gpuSimResources.GetCurrentState();
	const GpuParticleStaticTextures& gpuSimStaticTextures = gpuSimResources.GetStaticTextures();
	const GpuParticleCurves& gpuCurves = gpuSimResources.GetCurveTexture();
	const SPtr<GpuBuffer>& sortedIndices = gpuSimResources.GetSortedIndices();

	DrawCommand.ParamsGpu.PositionTimeTexture.Set(gpuSimStateTextures.PositionAndTimeTex);
	DrawCommand.ParamsGpu.SizeRotationTexture.Set(gpuSimStaticTextures.SizeAndRotationTex);
	DrawCommand.ParamsGpu.CurvesTexture.Set(gpuCurves.GetTexture());
	DrawCommand.NumParticles = GpuParticleSystem->GetTileCount() * GpuParticleConstants::kParticlesPerTile;

	i32 bufferOffset;
	if(GpuParticleSystem->HasSortInfo())
	{
		DrawCommand.IndicesBuffer.Set(sortedIndices);
		bufferOffset = GpuParticleSystem->GetSortOffset();
	}
	else
	{
		DrawCommand.IndicesBuffer.Set(GpuParticleSystem->GetParticleIndices());
		bufferOffset = 0;
	}

	PopulateAndBindParticlesUniformBuffer(GpuParticleConstants::kTexSize, bufferOffset);
	DrawCommand.PerCameraUniformBufferParameter.Set(view.GetPerViewBuffer());
}

void ParticleRenderState::PopulateAndBindParticlesUniformBuffer(i32 texSize, i32 bufferOffset) const
{
	GpuBufferMappedScope uniforms = gParticlesUniformDefinition.AllocateTransient().Map();

	// Set axis vectors
	const ParticleSystemSettings& settings = ParticleSystem->GetSettings();
	Vector3 axisForward = settings.OrientationPlaneNormal;
	Vector3 axisUp = Vector3::kUnitY;
	if(axisForward.Dot(axisUp) > 0.9998f)
		axisUp = Vector3::kUnitZ;

	Vector3 axisRight = axisUp.Cross(axisForward);
	Vector3::Orthonormalize(axisRight, axisUp, axisForward);

	gParticlesUniformDefinition.gAxisUp.Set(uniforms, axisUp);
	gParticlesUniformDefinition.gAxisRight.Set(uniforms, axisRight);

	// Set UV parameters from sprite image if available
	const SPtr<Shader> shader = DrawCommand.Material->GetShader();
	SpriteImage* spriteImage = nullptr;
	if(shader->HasTextureParameter("gTexture"))
		spriteImage = DrawCommand.Material->GetSpriteImage("gTexture").get();

	if(!spriteImage && shader->HasTextureParameter("gAlbedoTex"))
		spriteImage = DrawCommand.Material->GetSpriteImage("gAlbedoTex").get();

	if(spriteImage)
	{
		const Area2 UVRange = spriteImage->GetDefaultAllocatedImage().GetUVRange();
		gParticlesUniformDefinition.gUVOffset.Set(uniforms, UVRange.GetPosition());
		gParticlesUniformDefinition.gUVScale.Set(uniforms, Vector2(UVRange.Width, UVRange.Height));

		const SpriteSheetGridAnimation& anim = spriteImage->GetAnimation();
		gParticlesUniformDefinition.gSubImageSize.Set(uniforms, Vector4((float)anim.ColumnCount, (float)anim.RowCount, 1.0f / anim.ColumnCount, 1.0f / anim.RowCount));
	}
	else
	{
		gParticlesUniformDefinition.gUVOffset.Set(uniforms, Vector2::kZero);
		gParticlesUniformDefinition.gUVScale.Set(uniforms, Vector2::kOne);
		gParticlesUniformDefinition.gSubImageSize.Set(uniforms, Vector4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	gParticlesUniformDefinition.gTexSize.Set(uniforms, texSize);
	gParticlesUniformDefinition.gBufferOffset.Set(uniforms, bufferOffset);

	DrawCommand.ParticlesUniformBufferParameter.Set(uniforms);
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
	TextureUtility::Write(output->PositionAndRotation, simulationData.PositionAndRotation, 0, 0, TextureWriteFlag::Discard);
	TextureUtility::Write(output->Color, simulationData.Color, 0, 0, TextureWriteFlag::Discard);
	TextureUtility::Write(output->SizeAndFrameIdx, simulationData.SizeAndFrameIdx, 0, 0, TextureWriteFlag::Discard);

	WriteIndices(output->Indices, simulationData.Indices, size);
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
	TextureUtility::Write(output->Position, simulationData.Position, 0, 0, TextureWriteFlag::Discard);
	TextureUtility::Write(output->Color, simulationData.Color, 0, 0, TextureWriteFlag::Discard);
	TextureUtility::Write(output->Size, simulationData.Size, 0, 0, TextureWriteFlag::Discard);
	TextureUtility::Write(output->Rotation, simulationData.Rotation, 0, 0, TextureWriteFlag::Discard);

	WriteIndices(output->Indices, simulationData.Indices, size);
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
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	ParticleBillboardTextures* output = mBillboardAlloc.Construct<ParticleBillboardTextures>();

	TextureCreateInformation textureCreateInformation;
	textureCreateInformation.Name = "Particle Billboard Position & Size";
	textureCreateInformation.Type = TEX_TYPE_2D;
	textureCreateInformation.Width = size;
	textureCreateInformation.Height = size;
	textureCreateInformation.Usage = TextureUsageFlag::StoreOnCPUWithGPUAccess;

	textureCreateInformation.Format = PF_RGBA32F;
	output->PositionAndRotation = gpuDevice->CreateTexture(textureCreateInformation);

	textureCreateInformation.Format = PF_RGBA8;
	textureCreateInformation.Name = "Particle Billboard Color";
	output->Color = gpuDevice->CreateTexture(textureCreateInformation);

	textureCreateInformation.Format = PF_RGBA16F;
	textureCreateInformation.Name = "Particle Billboard Size & Frame Index";
	output->SizeAndFrameIdx = gpuDevice->CreateTexture(textureCreateInformation);

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	bufferCreateInformation.SimpleStorage.Count = size * size;
	bufferCreateInformation.SimpleStorage.Format = BF_16X2U;

	output->Indices = gpuDevice->CreateGpuBuffer(bufferCreateInformation);

	mBillboardBufferList[size].Buffers.push_back(output);
	return output;
}

ParticleMeshTextures* ParticleTexturePool::CreateNewMeshTextures(u32 size)
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	ParticleMeshTextures* output = mMeshAlloc.Construct<ParticleMeshTextures>();

	TextureCreateInformation texDesc;
	texDesc.Type = TEX_TYPE_2D;
	texDesc.Width = size;
	texDesc.Height = size;
	texDesc.Usage = TextureUsageFlag::StoreOnCPUWithGPUAccess;

	texDesc.Format = PF_RGBA32F;
	texDesc.Name = "Particle Mesh Position";
	output->Position = gpuDevice->CreateTexture(texDesc);

	texDesc.Format = PF_RGBA8;
	texDesc.Name = "Particle Mesh Color";
	output->Color = gpuDevice->CreateTexture(texDesc);

	texDesc.Format = PF_RGBA16F;
	texDesc.Name = "Particle Mesh Size";
	output->Size = gpuDevice->CreateTexture(texDesc);

	texDesc.Format = PF_RGBA16F;
	texDesc.Name = "Particle Mesh Rotation";
	output->Rotation = gpuDevice->CreateTexture(texDesc);

	GpuBufferCreateInformation bufferCreateInformation;
	bufferCreateInformation.Type = GpuBufferType::SimpleStorage;
	bufferCreateInformation.SimpleStorage.Count = size * size;
	bufferCreateInformation.SimpleStorage.Format = BF_16X2U;

	output->Indices = gpuDevice->CreateGpuBuffer(bufferCreateInformation);

	mMeshBufferList[size].Buffers.push_back(output);
	return output;
}

struct ParticleRenderer::Members
{
	SPtr<GpuBuffer> BillboardVb;
	SPtr<VertexDescription> BillboardVertexDescription;
};

ParticleRenderer::ParticleRenderer()
	: m(B3DNew<Members>())
{
	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();

	TInlineArray<VertexElement, 8> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));
	vertexElements.Add(VertexElement(VET_FLOAT2, VES_TEXCOORD));
	vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_NORMAL));
	vertexElements.Add(VertexElement(VET_UBYTE4_NORM, VES_TANGENT));

	m->BillboardVertexDescription = B3DMakeShared<VertexDescription>(vertexElements);

	GpuBufferCreateInformation vbDesc;
	vbDesc.Type = GpuBufferType::Vertex;
	vbDesc.Vertex.Count = 4;
	vbDesc.Vertex.ElementSize = m->BillboardVertexDescription->GetVertexStride(0);
	m->BillboardVb = gpuDevice->CreateGpuBuffer(vbDesc);

	MeshData meshData(4, 0, m->BillboardVertexDescription);
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

	u32 stride = meshData.GetVertexDescription()->GetVertexStride(0);

	Vector3 normal = Vector3::kUnitY;
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

	GpuBufferUtility::Write(m->BillboardVb, 0, meshData.GetStreamSize(0), meshData.GetStreamData(0), GpuBufferWriteFlag::Discard);
}

ParticleRenderer::~ParticleRenderer()
{
	B3DDelete(m);
}

void ParticleRenderer::DrawBillboards(GpuCommandBuffer& commandBuffer, u32 count)
{
	SPtr<GpuBuffer> vertexBuffers[] = { m->BillboardVb };

	commandBuffer.SetVertexDescription(m->BillboardVertexDescription);
	commandBuffer.SetVertexBuffers(0, vertexBuffers, 1);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_STRIP);
	commandBuffer.Draw(0, 4, count);
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

	B3DMarkAllocatorFrame();
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
	B3DClearAllocatorFrame();
}

void ParticleRenderState::UpdatePerObjectData()
{
	const ParticleSystemSettings& settings = ParticleSystem->GetSettings();
	if(settings.SimulationSpace == ParticleSimulationSpace::Local)
	{
		const Transform& tfrm = ParticleSystem->GetWorldTransform();
		WorldTransform = tfrm.GetMatrix();
		WorldNoScale = Matrix4::TRS(tfrm.GetPosition(), tfrm.GetRotation(), Vector3::kOne);
	}
	else
	{
		WorldTransform = Matrix4::kIdentity;
		WorldNoScale = Matrix4::kIdentity;
	}

	Layer = Bitwise::MostSignificantBit(ParticleSystem->GetLayer());
}

}} // namespace b3d::render
