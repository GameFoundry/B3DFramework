//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsShadowRendering.h"
#include "BsRendererView.h"
#include "BsRendererScene.h"
#include "Renderer/BsLight.h"
#include "Renderer/BsRendererUtility.h"
#include "Material/BsGpuParamsSet.h"
#include "Mesh/BsMesh.h"
#include "Renderer/BsCamera.h"
#include "Utility/BsBitwise.h"
#include "RenderAPI/BsVertexDescription.h"
#include "Renderer/BsRenderer.h"
#include "BsRendererRenderable.h"
#include "RenderAPI/BsCommandBuffer.h"

namespace bs { namespace ct {

ShadowParamsDef gShadowParamsDef;

void ShadowDepthNormalMat::Bind(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& shadowParams)
{
	mGPUParameters->SetUniformBuffer("ShadowParams", shadowParams);

	commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
	commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
}

void ShadowDepthNormalMat::SetPerObjectBuffer(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perObjectParams)
{
	mGPUParameters->SetUniformBuffer("PerObject", perObjectParams);

	commandBuffer.SetGpuParameters(mGPUParameters);
}

ShadowDepthNormalMat* ShadowDepthNormalMat::GetVariation(bool skinned, bool morph)
{
	if(skinned)
	{
		if(morph)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}
	else
	{
		if(morph)
			return Get(GetVariation<false, true>());

		return Get(GetVariation<false, false>());
	}
}

void ShadowDepthNormalNoPSMat::Bind(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& shadowParams)
{
	mGPUParameters->SetUniformBuffer("ShadowParams", shadowParams);

	commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
	commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
}

void ShadowDepthNormalNoPSMat::SetPerObjectBuffer(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perObjectParams)
{
	mGPUParameters->SetUniformBuffer("PerObject", perObjectParams);

	commandBuffer.SetGpuParameters(mGPUParameters);
}

ShadowDepthNormalNoPSMat* ShadowDepthNormalNoPSMat::GetVariation(bool skinned, bool morph)
{
	if(skinned)
	{
		if(morph)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}
	else
	{
		if(morph)
			return Get(GetVariation<false, true>());

		return Get(GetVariation<false, false>());
	}
}

void ShadowDepthDirectionalMat::Bind(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& shadowParams)
{
	mGPUParameters->SetUniformBuffer("ShadowParams", shadowParams);

	commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
	commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
}

void ShadowDepthDirectionalMat::SetPerObjectBuffer(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perObjectParams)
{
	mGPUParameters->SetUniformBuffer("PerObject", perObjectParams);
	commandBuffer.SetGpuParameters(mGPUParameters);
}

ShadowDepthDirectionalMat* ShadowDepthDirectionalMat::GetVariation(bool skinned, bool morph)
{
	if(skinned)
	{
		if(morph)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}
	else
	{
		if(morph)
			return Get(GetVariation<false, true>());

		return Get(GetVariation<false, false>());
	}
}

ShadowCubeMatricesDef gShadowCubeMatricesDef;
ShadowCubeMasksDef gShadowCubeMasksDef;

void ShadowDepthCubeMat::Bind(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& shadowParams, const SPtr<GpuBuffer>& shadowCubeMatrices)
{
	mGPUParameters->SetUniformBuffer("ShadowParams", shadowParams);
	mGPUParameters->SetUniformBuffer("ShadowCubeMatrices", shadowCubeMatrices);

	commandBuffer.SetGpuGraphicsPipelineState(mGraphicsPipeline);
	commandBuffer.SetStencilReferenceValue(mStencilReferenceValue);
}

void ShadowDepthCubeMat::SetPerObjectBuffer(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perObjectParams, const SPtr<GpuBuffer>& shadowCubeMasks)
{
	mGPUParameters->SetUniformBuffer("PerObject", perObjectParams);
	mGPUParameters->SetUniformBuffer("ShadowCubeMasks", shadowCubeMasks);

	commandBuffer.SetGpuParameters(mGPUParameters);
}

ShadowDepthCubeMat* ShadowDepthCubeMat::GetVariation(bool skinned, bool morph)
{
	if(skinned)
	{
		if(morph)
			return Get(GetVariation<true, true>());

		return Get(GetVariation<true, false>());
	}
	else
	{
		if(morph)
			return Get(GetVariation<false, true>());

		return Get(GetVariation<false, false>());
	}
}

ShadowProjectParamsDef gShadowProjectParamsDef;
ShadowProjectVertParamsDef gShadowProjectVertParamsDef;

void ShadowProjectStencilMat::Initialize()
{
	mVertParams = gShadowProjectVertParamsDef.CreateBuffer();
	if(mGPUParameters->HasUniformBuffer(GPT_VERTEX_PROGRAM, "VertParams"))
		mGPUParameters->SetUniformBuffer(GPT_VERTEX_PROGRAM, "VertParams", mVertParams);
}

void ShadowProjectStencilMat::Bind(CommandBuffer& commandBuffer, const SPtr<GpuBuffer>& perCamera)
{
	Vector4 lightPosAndScale(0, 0, 0, 1);
	gShadowProjectVertParamsDef.gPositionAndScale.Set(mVertParams, lightPosAndScale);

	mGPUParameters->SetUniformBuffer("PerCamera", perCamera);

	RendererMaterial::Bind(commandBuffer);
}

ShadowProjectStencilMat* ShadowProjectStencilMat::GetVariation(bool directional, bool useZFailStencil)
{
	if(directional)
		return Get(GetVariation<true, true>());
	else
	{
		if(useZFailStencil)
			return Get(GetVariation<false, true>());
		else
			return Get(GetVariation<false, false>());
	}
}

void ShadowProjectMat::Initialize()
{
	mGBufferParams.Initialize(GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mGPUParameters->GetSampledTextureParameter(GPT_FRAGMENT_PROGRAM, "gShadowTex", mShadowMapParam);
	if(mGPUParameters->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gShadowSampler"))
		mGPUParameters->GetSamplerStateParameter(GPT_FRAGMENT_PROGRAM, "gShadowSampler", mShadowSamplerParam);
	else
		mGPUParameters->GetSamplerStateParameter(GPT_FRAGMENT_PROGRAM, "gShadowTex", mShadowSamplerParam);

	SamplerStateInformation desc;
	desc.MinFilter = FO_POINT;
	desc.MagFilter = FO_POINT;
	desc.MipFilter = FO_POINT;
	desc.AddressMode.U = TAM_CLAMP;
	desc.AddressMode.V = TAM_CLAMP;
	desc.AddressMode.W = TAM_CLAMP;

	mSamplerState = SamplerState::Create(desc);

	mVertParams = gShadowProjectVertParamsDef.CreateBuffer();
	if(mGPUParameters->HasUniformBuffer(GPT_VERTEX_PROGRAM, "VertParams"))
		mGPUParameters->SetUniformBuffer(GPT_VERTEX_PROGRAM, "VertParams", mVertParams);
}

void ShadowProjectMat::Bind(CommandBuffer& commandBuffer, const ShadowProjectParams& params)
{
	Vector4 lightPosAndScale(Vector3(0.0f, 0.0f, 0.0f), 1.0f);
	gShadowProjectVertParamsDef.gPositionAndScale.Set(mVertParams, lightPosAndScale);

	mGBufferParams.Bind(params.Gbuffer);

	mShadowMapParam.Set(params.ShadowMap);
	mShadowSamplerParam.Set(mSamplerState);

	mGPUParameters->SetUniformBuffer("Params", params.ShadowParams);
	mGPUParameters->SetUniformBuffer("PerCamera", params.PerCamera);

	RendererMaterial::Bind(commandBuffer);
}

ShadowProjectMat* ShadowProjectMat::GetVariation(u32 quality, bool directional, bool MSAA)
{
#define BIND_MAT(QUALITY)                                         \
	{                                                             \
		if(directional)                                           \
			if(MSAA)                                              \
				return Get(GetVariation<QUALITY, true, true>());  \
			else                                                  \
				return Get(GetVariation<QUALITY, true, false>()); \
		else if(MSAA)                                             \
			return Get(GetVariation<QUALITY, false, true>());     \
		else                                                      \
			return Get(GetVariation<QUALITY, false, false>());    \
	}

	if(quality <= 1)
		BIND_MAT(1)
	else if(quality == 2)
		BIND_MAT(2)
	else if(quality == 3)
		BIND_MAT(3)
	else // 4 or higher
		BIND_MAT(4)

#undef BIND_MAT
}

ShadowProjectOmniParamsDef gShadowProjectOmniParamsDef;

void ShadowProjectOmniMat::Initialize()
{
	mGBufferParams.Initialize(GPT_FRAGMENT_PROGRAM, mGPUParameters);
	mGPUParameters->GetSampledTextureParameter(GPT_FRAGMENT_PROGRAM, "gShadowCubeTex", mShadowMapParam);

	if(mGPUParameters->HasSamplerState(GPT_FRAGMENT_PROGRAM, "gShadowCubeSampler"))
		mGPUParameters->GetSamplerStateParameter(GPT_FRAGMENT_PROGRAM, "gShadowCubeSampler", mShadowSamplerParam);
	else
		mGPUParameters->GetSamplerStateParameter(GPT_FRAGMENT_PROGRAM, "gShadowCubeTex", mShadowSamplerParam);

	SamplerStateInformation desc;
	desc.MinFilter = FO_LINEAR;
	desc.MagFilter = FO_LINEAR;
	desc.MipFilter = FO_POINT;
	desc.AddressMode.U = TAM_CLAMP;
	desc.AddressMode.V = TAM_CLAMP;
	desc.AddressMode.W = TAM_CLAMP;
	desc.ComparisonFunc = CMPF_GREATER_EQUAL;

	mSamplerState = SamplerState::Create(desc);

	mVertParams = gShadowProjectVertParamsDef.CreateBuffer();
	if(mGPUParameters->HasUniformBuffer(GPT_VERTEX_PROGRAM, "VertParams"))
		mGPUParameters->SetUniformBuffer(GPT_VERTEX_PROGRAM, "VertParams", mVertParams);
}

void ShadowProjectOmniMat::Bind(CommandBuffer& commandBuffer, const ShadowProjectParams& params)
{
	Vector4 lightPosAndScale(params.Light.GetTransform().GetPosition(), params.Light.GetAttenuationRadius());
	gShadowProjectVertParamsDef.gPositionAndScale.Set(mVertParams, lightPosAndScale);

	mGBufferParams.Bind(params.Gbuffer);

	mShadowMapParam.Set(params.ShadowMap);
	mShadowSamplerParam.Set(mSamplerState);

	mGPUParameters->SetUniformBuffer("Params", params.ShadowParams);
	mGPUParameters->SetUniformBuffer("PerCamera", params.PerCamera);

	RendererMaterial::Bind(commandBuffer);
}

ShadowProjectOmniMat* ShadowProjectOmniMat::GetVariation(u32 quality, bool inside, bool MSAA)
{
#define BIND_MAT(QUALITY)                                         \
	{                                                             \
		if(inside)                                                \
			if(MSAA)                                              \
				return Get(GetVariation<QUALITY, true, true>());  \
			else                                                  \
				return Get(GetVariation<QUALITY, true, false>()); \
		else if(MSAA)                                             \
			return Get(GetVariation<QUALITY, false, true>());     \
		else                                                      \
			return Get(GetVariation<QUALITY, false, false>());    \
	}

	if(quality <= 1)
		BIND_MAT(1)
	else if(quality == 2)
		BIND_MAT(2)
	else if(quality == 3)
		BIND_MAT(3)
	else // 4 or higher
		BIND_MAT(4)

#undef BIND_MAT
}

void ShadowInfo::UpdateNormArea(u32 atlasSize)
{
	NormArea.X = Area.X / (float)atlasSize;
	NormArea.Y = Area.Y / (float)atlasSize;
	NormArea.Width = Area.Width / (float)atlasSize;
	NormArea.Height = Area.Height / (float)atlasSize;
}

ShadowMapAtlas::ShadowMapAtlas(u32 size)
	: mLayout(0, 0, size, size, true), mLastUsedCounter(0)
{
	mAtlas = GpuResourcePool::Instance().Get(
		POOLED_RENDER_TEXTURE_DESC::Create2D(kShadowMapFormat, size, size, TU_DEPTHSTENCIL));
}

bool ShadowMapAtlas::AddMap(u32 size, Rect2I& area, u32 border)
{
	u32 sizeWithBorder = size + border * 2;

	u32 x, y;
	if(!mLayout.AddElement(sizeWithBorder, sizeWithBorder, x, y))
		return false;

	area.Width = area.Height = size;
	area.X = x + border;
	area.Y = y + border;

	mLastUsedCounter = 0;
	return true;
}

void ShadowMapAtlas::Clear()
{
	mLayout.Clear();
	mLastUsedCounter++;
}

bool ShadowMapAtlas::IsEmpty() const
{
	return mLayout.IsEmpty();
}

SPtr<Texture> ShadowMapAtlas::GetTexture() const
{
	return mAtlas->Texture;
}

SPtr<RenderTexture> ShadowMapAtlas::GetTarget() const
{
	return mAtlas->RenderTexture;
}

ShadowMapBase::ShadowMapBase(u32 size)
	: mSize(size), mIsUsed(false), mLastUsedCounter(0)
{}

SPtr<Texture> ShadowMapBase::GetTexture() const
{
	return mShadowMap->Texture;
}

ShadowCubemap::ShadowCubemap(u32 size)
	: ShadowMapBase(size)
{
	mShadowMap = GpuResourcePool::Instance().Get(
		POOLED_RENDER_TEXTURE_DESC::CreateCube(kShadowMapFormat, size, size, TU_DEPTHSTENCIL));
}

SPtr<RenderTexture> ShadowCubemap::GetTarget() const
{
	return mShadowMap->RenderTexture;
}

ShadowCascadedMap::ShadowCascadedMap(u32 size, u32 numCascades)
	: ShadowMapBase(size), mNumCascades(numCascades), mTargets(numCascades), mShadowInfos(numCascades)
{
	mShadowMap = GpuResourcePool::Instance().Get(POOLED_RENDER_TEXTURE_DESC::Create2D(kShadowMapFormat, size, size, TU_DEPTHSTENCIL, 0, false, numCascades));

	RENDER_TEXTURE_DESC rtDesc;
	rtDesc.DepthStencilSurface.Texture = mShadowMap->Texture;
	rtDesc.DepthStencilSurface.NumFaces = 1;

	for(u32 i = 0; i < mNumCascades; ++i)
	{
		rtDesc.DepthStencilSurface.Face = i;
		mTargets[i] = RenderTexture::Create(rtDesc);
	}
}

SPtr<RenderTexture> ShadowCascadedMap::GetTarget(u32 cascadeIdx) const
{
	return mTargets[cascadeIdx];
}

/**
 * Provides a common way for all types of shadow depth rendering to render the relevant objects into the depth map.
 * Iterates over all relevant objects in the scene, binds the relevant materials and renders the objects into the depth
 * map.
 */
class ShadowRenderQueue
{
public:
	struct Command
	{
		Command()
		{}

		Command(RenderableElement* element)
			: Element(element), IsElement(true)
		{}

		union
		{
			RenderableElement* Element;
			RendererRenderable* Renderable;
		};

		bool IsElement : 1;
		u32 Mask : 6;
	};

	template <class Options>
	static void Execute(CommandBuffer& commandBuffer, RendererScene& scene, const FrameInfo& frameInfo, const Options& opt)
	{
		static_assert((u32)RenderableAnimType::Count == 4, "RenderableAnimType is expected to have four sequential entries.");

		const SceneInfo& sceneInfo = scene.GetSceneInfo();

		B3DMarkAllocatorFrame();
		{
			FrameVector<Command> commands[4];

			// Make a list of relevant renderables and prepare them for rendering
			for(u32 i = 0; i < sceneInfo.Renderables.size(); i++)
			{
				const Sphere& bounds = sceneInfo.RenderableCullInfos[i].Bounds.GetSphere();
				if(!opt.Intersects(bounds))
					continue;

				scene.PrepareVisibleRenderable(i, frameInfo);

				Command renderableCommand;
				renderableCommand.Mask = 0;

				RendererRenderable* renderable = sceneInfo.Renderables[i];
				renderableCommand.IsElement = false;
				renderableCommand.Renderable = renderable;

				opt.Prepare(renderableCommand, bounds);

				bool renderableBound[4];
				B3DZeroOut(renderableBound);

				for(auto& element : renderable->Elements)
				{
					u32 arrayIdx = (int)element.AnimType;

					if(!renderableBound[arrayIdx])
					{
						commands[arrayIdx].push_back(renderableCommand);
						renderableBound[arrayIdx] = true;
					}

					commands[arrayIdx].push_back(Command(&element));
				}
			}

			static const ShaderVariationParameters* VAR_LOOKUP[4];
			VAR_LOOKUP[0] = &GetVertexInputVariation<false, false, false>(false);
			VAR_LOOKUP[1] = &GetVertexInputVariation<true, false, false>(false);
			VAR_LOOKUP[2] = &GetVertexInputVariation<false, true, false>(false);
			VAR_LOOKUP[3] = &GetVertexInputVariation<true, true, false>(false);

			for(u32 i = 0; i < (u32)RenderableAnimType::Count; i++)
			{
				opt.BindMaterial(commandBuffer, *VAR_LOOKUP[i]);

				for(auto& command : commands[i])
				{
					if(command.IsElement)
					{
						const RenderableElement& element = *command.Element;

						if(element.MorphVertexDefinition == nullptr)
							GetRendererUtility().Draw(commandBuffer, element.Mesh, element.SubMesh);
						else
							GetRendererUtility().DrawMorph(commandBuffer, element.Mesh, element.SubMesh, element.MorphShapeBuffer, element.MorphVertexDefinition);
					}
					else
						opt.BindRenderable(commandBuffer, command);
				}
			}
		}
		B3DClearAllocatorFrame();
	}
};

/** Specialization used for ShadowRenderQueue when rendering cube (omnidirectional) shadow maps (all faces at once). */
struct ShadowRenderQueueCubeOptions
{
	ShadowRenderQueueCubeOptions(
		const ConvexVolume (&frustums)[6],
		const ConvexVolume& boundingVolume,
		const SPtr<GpuBuffer>& shadowParamsBuffer,
		const SPtr<GpuBuffer>& shadowCubeMatricesBuffer,
		const SPtr<GpuBuffer>& shadowCubeMasksBuffer)
		: Frustums(frustums), BoundingVolume(boundingVolume), ShadowParamsBuffer(shadowParamsBuffer), ShadowCubeMatricesBuffer(shadowCubeMatricesBuffer), ShadowCubeMasksBuffer(shadowCubeMasksBuffer)
	{}

	bool Intersects(const Sphere& bounds) const
	{
		return BoundingVolume.Intersects(bounds);
	}

	void Prepare(ShadowRenderQueue::Command& command, const Sphere& bounds) const
	{
		for(u32 j = 0; j < 6; j++)
			command.Mask |= (Frustums[j].Intersects(bounds) ? 1 : 0) << j;
	}

	void BindMaterial(CommandBuffer& commandBuffer, const ShaderVariationParameters& variation) const
	{
		Material = ShadowDepthCubeMat::Get(variation);
		Material->Bind(commandBuffer, ShadowParamsBuffer, ShadowCubeMatricesBuffer);
	}

	void BindRenderable(CommandBuffer& commandBuffer, ShadowRenderQueue::Command& command) const
	{
		RendererRenderable* renderable = command.Renderable;

		for(u32 j = 0; j < 6; j++)
			gShadowCubeMasksDef.gFaceMasks.Set(ShadowCubeMasksBuffer, (command.Mask & (1 << j)), j);

		Material->SetPerObjectBuffer(commandBuffer, renderable->PerObjectParamBuffer, ShadowCubeMasksBuffer);
	}

	const ConvexVolume (&Frustums)[6];
	const ConvexVolume& BoundingVolume;
	const SPtr<GpuBuffer>& ShadowParamsBuffer;
	const SPtr<GpuBuffer>& ShadowCubeMatricesBuffer;
	const SPtr<GpuBuffer>& ShadowCubeMasksBuffer;

	mutable ShadowDepthCubeMat* Material = nullptr;
};

/** Specialization used for ShadowRenderQueue when rendering cube (omnidirectional) shadow maps (one face at a time). */
struct ShadowRenderQueueCubeSingleOptions
{
	ShadowRenderQueueCubeSingleOptions(
		const ConvexVolume& boundingVolume,
		const SPtr<GpuBuffer>& shadowParamsBuffer)
		: BoundingVolume(boundingVolume), ShadowParamsBuffer(shadowParamsBuffer)
	{}

	bool Intersects(const Sphere& bounds) const
	{
		return BoundingVolume.Intersects(bounds);
	}

	void Prepare(ShadowRenderQueue::Command& command, const Sphere& bounds) const
	{
	}

	void BindMaterial(CommandBuffer& commandBuffer, const ShaderVariationParameters& variation) const
	{
		Material = ShadowDepthNormalNoPSMat::Get(variation);
		Material->Bind(commandBuffer, ShadowParamsBuffer);
	}

	void BindRenderable(CommandBuffer& commandBuffer, ShadowRenderQueue::Command& command) const
	{
		RendererRenderable* renderable = command.Renderable;

		Material->SetPerObjectBuffer(commandBuffer, renderable->PerObjectParamBuffer);
	}

	const ConvexVolume& BoundingVolume;
	const SPtr<GpuBuffer>& ShadowParamsBuffer;

	mutable ShadowDepthNormalNoPSMat* Material = nullptr;
};

/** Specialization used for ShadowRenderQueue when rendering spot light shadow maps. */
struct ShadowRenderQueueSpotOptions
{
	ShadowRenderQueueSpotOptions(
		const ConvexVolume& boundingVolume,
		const SPtr<GpuBuffer>& shadowParamsBuffer)
		: BoundingVolume(boundingVolume), ShadowParamsBuffer(shadowParamsBuffer)
	{}

	bool Intersects(const Sphere& bounds) const
	{
		return BoundingVolume.Intersects(bounds);
	}

	void Prepare(ShadowRenderQueue::Command& command, const Sphere& bounds) const
	{
	}

	void BindMaterial(CommandBuffer& commandBuffer, const ShaderVariationParameters& variation) const
	{
		Material = ShadowDepthNormalMat::Get(variation);
		Material->Bind(commandBuffer, ShadowParamsBuffer);
	}

	void BindRenderable(CommandBuffer& commandBuffer, ShadowRenderQueue::Command& command) const
	{
		RendererRenderable* renderable = command.Renderable;

		Material->SetPerObjectBuffer(commandBuffer, renderable->PerObjectParamBuffer);
	}

	const ConvexVolume& BoundingVolume;
	const SPtr<GpuBuffer>& ShadowParamsBuffer;

	mutable ShadowDepthNormalMat* Material = nullptr;
};

/** Specialization used for ShadowRenderQueue when rendering directional light shadow maps. */
struct ShadowRenderQueueDirOptions
{
	ShadowRenderQueueDirOptions(
		const ConvexVolume& boundingVolume,
		const SPtr<GpuBuffer>& shadowParamsBuffer)
		: BoundingVolume(boundingVolume), ShadowParamsBuffer(shadowParamsBuffer)
	{}

	bool Intersects(const Sphere& bounds) const
	{
		return BoundingVolume.Intersects(bounds);
	}

	void Prepare(ShadowRenderQueue::Command& command, const Sphere& bounds) const
	{
	}

	void BindMaterial(CommandBuffer& commandBuffer, const ShaderVariationParameters& variation) const
	{
		Material = ShadowDepthDirectionalMat::Get(variation);
		Material->Bind(commandBuffer, ShadowParamsBuffer);
	}

	void BindRenderable(CommandBuffer& commandBuffer, ShadowRenderQueue::Command& command) const
	{
		RendererRenderable* renderable = command.Renderable;

		Material->SetPerObjectBuffer(commandBuffer, renderable->PerObjectParamBuffer);
	}

	const ConvexVolume& BoundingVolume;
	const SPtr<GpuBuffer>& ShadowParamsBuffer;

	mutable ShadowDepthDirectionalMat* Material = nullptr;
};

const u32 ShadowRendering::kMaxAtlasSize = 4096;
const u32 ShadowRendering::kMaxUnusedFrames = 60;
const u32 ShadowRendering::kMinShadowMapSize = 32;
const u32 ShadowRendering::kShadowMapFadeSize = 64;
const u32 ShadowRendering::kShadowMapBorder = 4;
const float ShadowRendering::kCascadeFractionFade = 0.1f;

ShadowRendering::ShadowRendering(u32 shadowMapSize)
	: mShadowMapSize(shadowMapSize)
{
	const SPtr<GpuDevice>& gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();

	SmallVector<VertexElement, 8> vertexElements;
	vertexElements.Add(VertexElement(VET_FLOAT3, VES_POSITION));

	mPositionOnlyVertexDescription = B3DMakeShared<VertexDescription>(vertexElements);

	// Create plane index and vertex buffers
	{
		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		vertexBufferCreateInformation.Vertex.Count = 8;
		vertexBufferCreateInformation.Vertex.ElementSize = mPositionOnlyVertexDescription->GetVertexStride(0);

		mPlaneVB = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

		GpuBufferCreateInformation indexBufferCreateInformation;
		indexBufferCreateInformation.Type = GpuBufferType::Index;
		indexBufferCreateInformation.Index.Type = IT_32BIT;
		indexBufferCreateInformation.Index.Count = 12;

		mPlaneIB = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);

		u32 indices[] = {
			// Far plane, back facing
			4, 7, 6,
			4, 6, 5,

			// Near plane, front facing
			0, 1, 2,
			0, 2, 3
		};

		mPlaneIB->WriteData(0, sizeof(indices), indices);
	}

	// Create frustum index and vertex buffers
	{
		GpuBufferCreateInformation vertexBufferCreateInformation;
		vertexBufferCreateInformation.Type = GpuBufferType::Vertex;
		vertexBufferCreateInformation.Flags = GpuBufferFlag::StoreOnCPUWithGPUAccess;
		vertexBufferCreateInformation.Vertex.Count = 8;
		vertexBufferCreateInformation.Vertex.ElementSize = mPositionOnlyVertexDescription->GetVertexStride(0);

		mFrustumVB = gpuDevice->CreateGpuBuffer(vertexBufferCreateInformation);

		GpuBufferCreateInformation indexBufferCreateInformation;
		indexBufferCreateInformation.Type = GpuBufferType::Index;
		indexBufferCreateInformation.Index.Type = IT_32BIT;
		indexBufferCreateInformation.Index.Count = 36;

		mFrustumIB = gpuDevice->CreateGpuBuffer(indexBufferCreateInformation);
		mFrustumIB->WriteData(0, sizeof(AABox::kCubeIndices), AABox::kCubeIndices);
	}
}

void ShadowRendering::SetShadowMapSize(u32 size)
{
	if(mShadowMapSize == size)
		return;

	mCascadedShadowMaps.clear();
	mDynamicShadowMaps.clear();
	mShadowCubemaps.clear();

	mShadowMapSize = size;
}

void ShadowRendering::RenderShadowMaps(CommandBuffer& commandBuffer, RendererScene& scene, const RendererViewGroup& viewGroup, const FrameInfo& frameInfo)
{
	// Note: Currently all shadows are dynamic and are rebuilt every frame. I should later added support for static
	// shadow maps which can be used for immovable lights. Such a light can then maintain a set of shadow maps,
	// one of which is static and only effects the static geometry, while the rest are per-object shadow maps used
	// for dynamic objects. Then only a small subset of geometry needs to be redrawn, instead of everything.

	// Note: Add support for per-object shadows and a way to force a renderable to use per-object shadows. This can be
	// used for adding high quality shadows on specific objects (e.g. important characters during cinematics).

	const SceneInfo& sceneInfo = scene.GetSceneInfo();
	const VisibilityInfo& visibility = viewGroup.GetVisibilityInfo();

	// Clear all transient data from last frame
	mShadowInfos.clear();

	mSpotLightShadows.resize(sceneInfo.SpotLights.size());
	mRadialLightShadows.resize(sceneInfo.RadialLights.size());
	mDirectionalLightShadows.resize(sceneInfo.DirectionalLights.size());

	mSpotLightShadowOptions.clear();
	mRadialLightShadowOptions.clear();

	// Clear all dynamic light atlases
	for(auto& entry : mCascadedShadowMaps)
		entry.Clear();

	for(auto& entry : mDynamicShadowMaps)
		entry.Clear();

	for(auto& entry : mShadowCubemaps)
		entry.Clear();

	// Determine shadow map sizes and sort them
	u32 shadowInfoCount = 0;
	for(u32 i = 0; i < (u32)sceneInfo.SpotLights.size(); ++i)
	{
		const RendererLight& light = sceneInfo.SpotLights[i];
		mSpotLightShadows[i].StartIdx = shadowInfoCount;
		mSpotLightShadows[i].NumShadows = 0;

		// Note: I'm using visibility across all views, while I could be using visibility for every view individually,
		// if I kept that information somewhere
		if(!light.Internal->GetCastsShadow() || !visibility.SpotLights[i])
			continue;

		ShadowMapOptions options;
		options.LightIdx = i;

		float maxFadePercent;
		CalcShadowMapProperties(light, viewGroup, kShadowMapBorder, options.MapSize, options.FadePercents, maxFadePercent);

		// Don't render shadow maps that will end up nearly completely faded out
		if(maxFadePercent < 0.005f)
			continue;

		mSpotLightShadowOptions.push_back(options);
		shadowInfoCount++; // For now, always a single fully dynamic shadow for a single light, but that may change
	}

	for(u32 i = 0; i < (u32)sceneInfo.RadialLights.size(); ++i)
	{
		const RendererLight& light = sceneInfo.RadialLights[i];
		mRadialLightShadows[i].StartIdx = shadowInfoCount;
		mRadialLightShadows[i].NumShadows = 0;

		// Note: I'm using visibility across all views, while I could be using visibility for every view individually,
		// if I kept that information somewhere
		if(!light.Internal->GetCastsShadow() || !visibility.RadialLights[i])
			continue;

		ShadowMapOptions options;
		options.LightIdx = i;

		float maxFadePercent;
		CalcShadowMapProperties(light, viewGroup, 0, options.MapSize, options.FadePercents, maxFadePercent);

		// Don't render shadow maps that will end up nearly completely faded out
		if(maxFadePercent < 0.005f)
			continue;

		mRadialLightShadowOptions.push_back(options);

		shadowInfoCount++; // For now, always a single fully dynamic shadow for a single light, but that may change
	}

	// Sort spot lights by size so they fit neatly in the texture atlas
	std::sort(mSpotLightShadowOptions.begin(), mSpotLightShadowOptions.end(), [](const ShadowMapOptions& a, const ShadowMapOptions& b)
			  { return a.MapSize > b.MapSize; });

	// Reserve space for shadow infos
	mShadowInfos.resize(shadowInfoCount);

	// Deallocate unused textures (must be done before rendering shadows, in order to ensure indices don't change)
	for(auto iter = mDynamicShadowMaps.begin(); iter != mDynamicShadowMaps.end(); ++iter)
	{
		if(iter->GetLastUsedCounter() >= kMaxUnusedFrames)
		{
			// These are always populated in order, so we can assume all following atlases are also empty
			mDynamicShadowMaps.erase(iter, mDynamicShadowMaps.end());
			break;
		}
	}

	for(auto iter = mCascadedShadowMaps.begin(); iter != mCascadedShadowMaps.end();)
	{
		if(iter->GetLastUsedCounter() >= kMaxUnusedFrames)
			iter = mCascadedShadowMaps.erase(iter);
		else
			++iter;
	}

	for(auto iter = mShadowCubemaps.begin(); iter != mShadowCubemaps.end();)
	{
		if(iter->GetLastUsedCounter() >= kMaxUnusedFrames)
			iter = mShadowCubemaps.erase(iter);
		else
			++iter;
	}

	// Render shadow maps
	for(u32 i = 0; i < (u32)sceneInfo.DirectionalLights.size(); ++i)
	{
		const RendererLight& light = sceneInfo.DirectionalLights[i];

		if(!light.Internal->GetCastsShadow())
			return;

		u32 numViews = viewGroup.GetNumViews();
		mDirectionalLightShadows[i].ViewShadows.Resize(numViews);

		for(u32 j = 0; j < numViews; ++j)
			RenderCascadedShadowMaps(commandBuffer, *viewGroup.GetView(j), i, scene, frameInfo);
	}

	for(auto& entry : mSpotLightShadowOptions)
	{
		u32 lightIdx = entry.LightIdx;
		RenderSpotShadowMap(commandBuffer, sceneInfo.SpotLights[lightIdx], entry, scene, frameInfo);
	}

	for(auto& entry : mRadialLightShadowOptions)
	{
		u32 lightIdx = entry.LightIdx;
		RenderRadialShadowMap(commandBuffer, sceneInfo.RadialLights[lightIdx], entry, scene, frameInfo);
	}
}

/**
 * Generates a frustum from the provided view-projection matrix.
 *
 * @param[in]	invVP			Inverse of the view-projection matrix to use for generating the frustum.
 * @param[out]	worldFrustum	Generated frustum planes, in world space.
 * @return						Individual vertices of the frustum corners, in world space. Ordered using the
 *								AABox::CornerEnum.
 */
std::array<Vector3, 8> GetFrustum(const Matrix4& invVP, ConvexVolume& worldFrustum)
{
	std::array<Vector3, 8> output;

	const GpuDeviceCapabilities& caps = GetGpuDeviceCapabilities();

	float flipY = 1.0f;
	if(caps.Conventions.NdcYAxis == GpuBackendConventions::Axis::Down)
		flipY = -1.0f;

	AABox frustumCube(
		Vector3(-1, -1 * flipY, caps.MinDepth),
		Vector3(1, 1 * flipY, caps.MaxDepth));

	for(size_t i = 0; i < output.size(); i++)
	{
		Vector3 corner = frustumCube.GetCorner((AABox::Corner)i);
		output[i] = invVP.Multiply(corner);
	}

	Vector<Plane> planes(6);
	planes[FRUSTUM_PLANE_NEAR] = Plane(output[AABox::NEAR_LEFT_BOTTOM], output[AABox::NEAR_RIGHT_BOTTOM], output[AABox::NEAR_RIGHT_TOP]);
	planes[FRUSTUM_PLANE_FAR] = Plane(output[AABox::FAR_LEFT_BOTTOM], output[AABox::FAR_LEFT_TOP], output[AABox::FAR_RIGHT_TOP]);
	planes[FRUSTUM_PLANE_LEFT] = Plane(output[AABox::NEAR_LEFT_BOTTOM], output[AABox::NEAR_LEFT_TOP], output[AABox::FAR_LEFT_TOP]);
	planes[FRUSTUM_PLANE_RIGHT] = Plane(output[AABox::FAR_RIGHT_TOP], output[AABox::NEAR_RIGHT_TOP], output[AABox::NEAR_RIGHT_BOTTOM]);
	planes[FRUSTUM_PLANE_TOP] = Plane(output[AABox::NEAR_LEFT_TOP], output[AABox::NEAR_RIGHT_TOP], output[AABox::FAR_RIGHT_TOP]);
	planes[FRUSTUM_PLANE_BOTTOM] = Plane(output[AABox::NEAR_LEFT_BOTTOM], output[AABox::FAR_LEFT_BOTTOM], output[AABox::FAR_RIGHT_BOTTOM]);

	worldFrustum = ConvexVolume(planes);
	return output;
}

/**
 * Converts a point in mixed space (clip_x, clip_y, view_z, view_w) to UV coordinates on a shadow map (x, y),
 * and normalized linear depth from the shadow caster's perspective (z).
 */
Matrix4 CreateMixedToShadowUvMatrix(const Matrix4& viewP, const Matrix4& viewInvVP, const Rect2& shadowMapArea, float depthScale, float depthOffset, const Matrix4& shadowViewProj)
{
	// Projects a point from (clip_x, clip_y, view_z, view_w) into clip space
	Matrix4 mixedToShadow = Matrix4::kIdentity;
	mixedToShadow[2][2] = viewP[2][2];
	mixedToShadow[2][3] = viewP[2][3];
	mixedToShadow[3][2] = viewP[3][2];
	mixedToShadow[3][3] = 0.0f;

	// Projects a point in clip space back to homogeneus world space
	mixedToShadow = viewInvVP * mixedToShadow;

	// Projects a point in world space to shadow clip space
	mixedToShadow = shadowViewProj * mixedToShadow;

	// Convert shadow clip space coordinates to UV coordinates relative to the shadow map rectangle, and normalize
	// depth
	const GpuBackendConventions& rapiConventions = GetGpuDeviceCapabilities().Conventions;

	float flipY = -1.0f;
	// Either of these flips the Y axis, but if they're both true they cancel out
	if((rapiConventions.UvYAxis == GpuBackendConventions::Axis::Up) ^ (rapiConventions.NdcYAxis == GpuBackendConventions::Axis::Down))
		flipY = -flipY;

	Matrix4 shadowMapTfrm(
		shadowMapArea.Width * 0.5f, 0, 0, shadowMapArea.X + 0.5f * shadowMapArea.Width,
		0, flipY * shadowMapArea.Height * 0.5f, 0, shadowMapArea.Y + 0.5f * shadowMapArea.Height,
		0, 0, depthScale, depthOffset,
		0, 0, 0, 1);

	return shadowMapTfrm * mixedToShadow;
}

void ShadowRendering::RenderShadowOcclusion(CommandBuffer& commandBuffer, const RendererView& view, const RendererLight& rendererLight, GBufferTextures gbuffer) const
{
	u32 shadowQuality = view.GetRenderSettings().ShadowSettings.ShadowFilteringQuality;

	const Light* light = rendererLight.Internal;
	u32 lightIdx = light->GetRendererId();

	auto viewProps = view.GetProperties();

	const Matrix4& viewP = viewProps.ProjTransform;
	Matrix4 viewInvVP = viewProps.ViewProjTransform.Inverse();

	SPtr<GpuBuffer> perViewBuffer = view.GetPerViewBuffer();

	ProfileGPUBlock sampleBlock("Render shadow occlusion");

	const GpuDeviceCapabilities& caps = GetGpuDeviceCapabilities();
	// TODO - Calculate and set a scissor rectangle for the light

	SPtr<GpuBuffer> shadowParamBuffer = gShadowProjectParamsDef.CreateBuffer();
	SPtr<GpuBuffer> shadowOmniParamBuffer = gShadowProjectOmniParamsDef.CreateBuffer();

	u32 viewIdx = view.GetViewIdx();
	Vector<const ShadowInfo*> shadowInfos;

	if(light->GetType() == LightType::Radial)
	{
		const LightShadows& shadows = mRadialLightShadows[lightIdx];

		for(u32 i = 0; i < shadows.NumShadows; ++i)
		{
			u32 shadowIdx = shadows.StartIdx + i;
			const ShadowInfo& shadowInfo = mShadowInfos[shadowIdx];

			if(shadowInfo.FadePerView[viewIdx] < 0.005f)
				continue;

			for(u32 j = 0; j < 6; j++)
				gShadowProjectOmniParamsDef.gFaceVPMatrices.Set(shadowOmniParamBuffer, shadowInfo.ShadowVpTransforms[j], j);

			gShadowProjectOmniParamsDef.gDepthBias.Set(shadowOmniParamBuffer, shadowInfo.DepthBias);
			gShadowProjectOmniParamsDef.gFadePercent.Set(shadowOmniParamBuffer, shadowInfo.FadePerView[viewIdx]);
			gShadowProjectOmniParamsDef.gInvResolution.Set(shadowOmniParamBuffer, 1.0f / shadowInfo.Area.Width);

			const Transform& tfrm = light->GetTransform();
			Vector4 lightPosAndRadius(tfrm.GetPosition(), light->GetAttenuationRadius());
			gShadowProjectOmniParamsDef.gLightPosAndRadius.Set(shadowOmniParamBuffer, lightPosAndRadius);

			// Reduce shadow quality based on shadow map resolution for spot lights
			u32 effectiveShadowQuality = GetShadowQuality(shadowQuality, shadowInfo.Area.Width, 2);

			// Check if viewer is inside the light bounds
			//// Expand the light bounds slightly to handle the case when the near plane is intersecting the light volume
			float lightRadius = light->GetAttenuationRadius() + viewProps.NearPlane * 3.0f;
			bool viewerInsideVolume = (tfrm.GetPosition() - viewProps.ViewOrigin).Length() < lightRadius;

			SPtr<Texture> shadowMap = mShadowCubemaps[shadowInfo.TextureIdx].GetTexture();
			ShadowProjectParams shadowParams(*light, shadowMap, shadowOmniParamBuffer, perViewBuffer, gbuffer);

			ShadowProjectOmniMat* mat = ShadowProjectOmniMat::GetVariation(effectiveShadowQuality, viewerInsideVolume, viewProps.Target.NumSamples > 1);
			mat->Bind(commandBuffer, shadowParams);

			GetRendererUtility().Draw(commandBuffer, GetRendererUtility().GetSphereStencil());
		}
	}
	else // Directional & spot
	{
		shadowInfos.clear();

		bool isCSM = light->GetType() == LightType::Directional;
		if(!isCSM)
		{
			const LightShadows& shadows = mSpotLightShadows[lightIdx];
			for(u32 i = 0; i < shadows.NumShadows; ++i)
			{
				u32 shadowIdx = shadows.StartIdx + i;
				const ShadowInfo& shadowInfo = mShadowInfos[shadowIdx];

				if(shadowInfo.FadePerView[viewIdx] < 0.005f)
					continue;

				shadowInfos.push_back(&shadowInfo);
			}
		}
		else // Directional
		{
			const LightShadows& shadows = mDirectionalLightShadows[lightIdx].ViewShadows[viewIdx];
			if(shadows.NumShadows > 0)
			{
				u32 mapIdx = shadows.StartIdx;
				const ShadowCascadedMap& cascadedMap = mCascadedShadowMaps[mapIdx];

				// Render cascades in far to near order.
				// Note: If rendering other non-cascade maps they should be rendered after cascades.
				for(i32 i = cascadedMap.GetNumCascades() - 1; i >= 0; i--)
					shadowInfos.push_back(&cascadedMap.GetShadowInfo(i));
			}
		}

		for(auto& shadowInfo : shadowInfos)
		{
			float depthScale, depthOffset;

			// Depth range scale is already baked into the ortho projection matrix, so avoid doing it here
			if(isCSM)
			{
				// Need to map from API-specific clip space depth to [0, 1] range
				depthScale = 1.0f / (caps.MaxDepth - caps.MinDepth);
				depthOffset = -caps.MinDepth * depthScale;
			}
			else
			{
				depthScale = 1.0f / shadowInfo->DepthRange;
				depthOffset = 0.0f;
			}

			SPtr<Texture> shadowMap;
			u32 shadowMapFace = 0;
			if(!isCSM)
				shadowMap = mDynamicShadowMaps[shadowInfo->TextureIdx].GetTexture();
			else
			{
				shadowMap = mCascadedShadowMaps[shadowInfo->TextureIdx].GetTexture();
				shadowMapFace = shadowInfo->CascadeIdx;
			}

			Matrix4 mixedToShadowUV = CreateMixedToShadowUvMatrix(viewP, viewInvVP, shadowInfo->NormArea, depthScale, depthOffset, shadowInfo->ShadowVpTransform);

			auto shadowMapProps = shadowMap->GetProperties();

			Vector2 shadowMapSize((float)shadowMapProps.Width, (float)shadowMapProps.Height);
			float transitionScale = GetFadeTransition(*light, shadowInfo->SubjectBounds.GetRadius(), shadowInfo->DepthRange, shadowInfo->Area.Width);

			gShadowProjectParamsDef.gFadePlaneDepth.Set(shadowParamBuffer, shadowInfo->DepthFade);
			gShadowProjectParamsDef.gMixedToShadowSpace.Set(shadowParamBuffer, mixedToShadowUV);
			gShadowProjectParamsDef.gShadowMapSize.Set(shadowParamBuffer, shadowMapSize);
			gShadowProjectParamsDef.gShadowMapSizeInv.Set(shadowParamBuffer, 1.0f / shadowMapSize);
			gShadowProjectParamsDef.gSoftTransitionScale.Set(shadowParamBuffer, transitionScale);

			if(isCSM)
				gShadowProjectParamsDef.gFadePercent.Set(shadowParamBuffer, 1.0f);
			else
				gShadowProjectParamsDef.gFadePercent.Set(shadowParamBuffer, shadowInfo->FadePerView[viewIdx]);

			if(shadowInfo->FadeRange == 0.0f)
				gShadowProjectParamsDef.gInvFadePlaneRange.Set(shadowParamBuffer, 0.0f);
			else
				gShadowProjectParamsDef.gInvFadePlaneRange.Set(shadowParamBuffer, 1.0f / shadowInfo->FadeRange);

			// Generate a stencil buffer to avoid evaluating pixels without any receiver geometry in the shadow area
			std::array<Vector3, 8> frustumVertices;
			u32 effectiveShadowQuality = shadowQuality;
			if(!isCSM)
			{
				ConvexVolume shadowFrustum;
				frustumVertices = GetFrustum(shadowInfo->ShadowVpTransform.Inverse(), shadowFrustum);

				// Check if viewer is inside the frustum. Frustum is slightly expanded so that if the near plane is
				// intersecting the shadow frustum, it is counted as inside. This needs to be conservative as the code
				// for handling viewer outside the frustum will not properly render intersections with the near plane.
				bool viewerInsideFrustum = shadowFrustum.Contains(viewProps.ViewOrigin, viewProps.NearPlane * 3.0f);

				ShadowProjectStencilMat* mat = ShadowProjectStencilMat::GetVariation(false, viewerInsideFrustum);
				mat->Bind(commandBuffer, perViewBuffer);
				DrawFrustum(commandBuffer, frustumVertices);

				// Reduce shadow quality based on shadow map resolution for spot lights
				effectiveShadowQuality = GetShadowQuality(shadowQuality, shadowInfo->Area.Width, 2);
			}
			else
			{
				// Need to generate near and far planes to clip the geometry within the current CSM slice.
				// Note: If the render API supports built-in depth bound tests that could be used instead.

				Vector3 near = viewProps.ProjTransform.Multiply(Vector3(0, 0, -shadowInfo->DepthNear));
				Vector3 far = viewProps.ProjTransform.Multiply(Vector3(0, 0, -shadowInfo->DepthFar));

				ShadowProjectStencilMat* mat = ShadowProjectStencilMat::GetVariation(true, true);
				mat->Bind(commandBuffer, perViewBuffer);

				DrawNearFarPlanes(commandBuffer, near.Z, far.Z, shadowInfo->CascadeIdx != 0);
			}

			gShadowProjectParamsDef.gFace.Set(shadowParamBuffer, (float)shadowMapFace);
			ShadowProjectParams shadowParams(*light, shadowMap, shadowParamBuffer, perViewBuffer, gbuffer);

			ShadowProjectMat* mat = ShadowProjectMat::GetVariation(effectiveShadowQuality, isCSM, viewProps.Target.NumSamples > 1);
			mat->Bind(commandBuffer, shadowParams);

			if(!isCSM)
				DrawFrustum(commandBuffer, frustumVertices);
			else
				GetRendererUtility().DrawScreenQuad(commandBuffer);
		}
	}
}

void ShadowRendering::RenderCascadedShadowMaps(CommandBuffer& commandBuffer, const RendererView& view, u32 lightIdx, RendererScene& scene, const FrameInfo& frameInfo)
{
	u32 viewIdx = view.GetViewIdx();
	LightShadows& lightShadows = mDirectionalLightShadows[lightIdx].ViewShadows[viewIdx];

	if(!view.GetRenderSettings().EnableShadows)
	{
		lightShadows.StartIdx = -1;
		lightShadows.NumShadows = 0;
		return;
	}

	// Note: Currently I'm using spherical bounds for the cascaded frustum which might result in non-optimal usage
	// of the shadow map. A different approach would be to generate a bounding box and then both adjust the aspect
	// ratio (and therefore dimensions) of the shadow map, as well as rotate the camera so the visible area best fits
	// in the map. It remains to be seen if this is viable.
	//  - Note2: Actually both of these will likely have serious negative impact on shadow stability.
	const SceneInfo& sceneInfo = scene.GetSceneInfo();

	const RendererLight& rendererLight = sceneInfo.DirectionalLights[lightIdx];
	Light* light = rendererLight.Internal;

	const Transform& tfrm = light->GetTransform();
	Vector3 lightDir = -tfrm.GetRotation().ZAxis();
	SPtr<GpuBuffer> shadowParamsBuffer = gShadowParamsDef.CreateBuffer();

	ShadowInfo shadowInfo;
	shadowInfo.LightIdx = lightIdx;
	shadowInfo.TextureIdx = -1;

	u32 mapSize = std::min(mShadowMapSize, kMaxAtlasSize);
	shadowInfo.Area = Rect2I(0, 0, mapSize, mapSize);
	shadowInfo.UpdateNormArea(mapSize);

	u32 numCascades = view.GetRenderSettings().ShadowSettings.NumCascades;
	for(u32 i = 0; i < (u32)mCascadedShadowMaps.size(); i++)
	{
		ShadowCascadedMap& shadowMap = mCascadedShadowMaps[i];

		if(!shadowMap.IsUsed() && shadowMap.GetSize() == mapSize && shadowMap.GetNumCascades() == numCascades)
		{
			shadowInfo.TextureIdx = i;
			shadowMap.MarkAsUsed();

			break;
		}
	}

	if(shadowInfo.TextureIdx == (u32)-1)
	{
		shadowInfo.TextureIdx = (u32)mCascadedShadowMaps.size();
		mCascadedShadowMaps.push_back(ShadowCascadedMap(mapSize, numCascades));

		ShadowCascadedMap& shadowMap = mCascadedShadowMaps.back();
		shadowMap.MarkAsUsed();
	}

	ShadowCascadedMap& shadowMap = mCascadedShadowMaps[shadowInfo.TextureIdx];

	Quaternion lightRotation(BsIdentity);
	lightRotation.LookRotation(lightDir, Vector3::kUnitY);

	ProfileGPUBlock profileSample("Project directional light shadow");

	for(u32 i = 0; i < numCascades; ++i)
	{
		Sphere frustumBounds;
		ConvexVolume cascadeCullVolume = GetCsmSplitFrustum(view, lightDir, i, numCascades, frustumBounds);

		// Make sure the size of the projected area is in multiples of shadow map pixel size (for stability)
		float worldUnitsPerTexel = frustumBounds.GetRadius() * 2.0f / shadowMap.GetSize();

		float orthoSize = floor(frustumBounds.GetRadius() * 2.0f / worldUnitsPerTexel) * worldUnitsPerTexel * 0.5f;
		worldUnitsPerTexel = orthoSize * 2.0f / shadowMap.GetSize();

		// Snap caster origin to the shadow map pixel grid, to ensure shadow map stability
		Vector3 casterOrigin = frustumBounds.GetCenter();
		Matrix4 shadowView = Matrix4::View(Vector3::kZero, lightRotation);
		Vector3 shadowSpaceOrigin = shadowView.MultiplyAffine(casterOrigin);

		Vector2 snapOffset(fmod(shadowSpaceOrigin.X, worldUnitsPerTexel), fmod(shadowSpaceOrigin.Y, worldUnitsPerTexel));
		shadowSpaceOrigin.X -= snapOffset.X;
		shadowSpaceOrigin.Y -= snapOffset.Y;

		Matrix4 shadowViewInv = shadowView.InverseAffine();
		casterOrigin = shadowViewInv.MultiplyAffine(shadowSpaceOrigin);

		// Move the light so it is centered at the subject frustum, with depth range covering the frustum bounds
		shadowInfo.DepthRange = frustumBounds.GetRadius() * 2.0f;

		Vector3 offsetLightPos = casterOrigin - lightDir * frustumBounds.GetRadius();
		Matrix4 offsetViewMat = Matrix4::View(offsetLightPos, lightRotation);

		Matrix4 proj = Matrix4::ProjectionOrthographic(-orthoSize, orthoSize, orthoSize, -orthoSize, 0.0f, shadowInfo.DepthRange);

		RenderAPI::Instance().ConvertProjectionMatrix(proj, proj);

		shadowInfo.CascadeIdx = i;
		shadowInfo.ShadowVpTransform = proj * offsetViewMat;

		// Determine split range
		float splitNear = GetCsmSplitDistance(view, i, numCascades);
		float splitFar = GetCsmSplitDistance(view, i + 1, numCascades);

		shadowInfo.DepthNear = splitNear;
		shadowInfo.DepthFade = splitFar;
		shadowInfo.SubjectBounds = frustumBounds;

		if((u32)(i + 1) < numCascades)
			shadowInfo.FadeRange = kCascadeFractionFade * (shadowInfo.DepthFade - shadowInfo.DepthNear);
		else
			shadowInfo.FadeRange = 0.0f;

		shadowInfo.DepthFar = shadowInfo.DepthFade + shadowInfo.FadeRange;
		shadowInfo.DepthBias = GetDepthBias(*light, frustumBounds.GetRadius(), shadowInfo.DepthRange, mapSize);

		gShadowParamsDef.gDepthBias.Set(shadowParamsBuffer, shadowInfo.DepthBias);
		gShadowParamsDef.gInvDepthRange.Set(shadowParamsBuffer, 1.0f / shadowInfo.DepthRange);
		gShadowParamsDef.gMatViewProj.Set(shadowParamsBuffer, shadowInfo.ShadowVpTransform);
		gShadowParamsDef.gNDCZToDeviceZ.Set(shadowParamsBuffer, RendererView::GetNdczToDeviceZ());

		commandBuffer.SetRenderTarget(shadowMap.GetTarget(i));
		commandBuffer.ClearRenderTarget(FBT_DEPTH);

		ShadowDepthDirectionalMat* depthDirMat = ShadowDepthDirectionalMat::Get();
		depthDirMat->Bind(commandBuffer, shadowParamsBuffer);

		// Render all renderables into the shadow map
		ShadowRenderQueueDirOptions dirOptions(
			cascadeCullVolume,
			shadowParamsBuffer);

		ShadowRenderQueue::Execute(commandBuffer, scene, frameInfo, dirOptions);

		shadowMap.SetShadowInfo(i, shadowInfo);
	}

	lightShadows.StartIdx = shadowInfo.TextureIdx;
	lightShadows.NumShadows = 1;
}

void ShadowRendering::RenderSpotShadowMap(CommandBuffer& commandBuffer, const RendererLight& rendererLight, const ShadowMapOptions& options, RendererScene& scene, const FrameInfo& frameInfo)
{
	Light* light = rendererLight.Internal;

	SPtr<GpuBuffer> shadowParamsBuffer = gShadowParamsDef.CreateBuffer();

	ShadowInfo mapInfo;
	mapInfo.FadePerView = options.FadePercents;
	mapInfo.LightIdx = options.LightIdx;
	mapInfo.CascadeIdx = -1;

	bool foundSpace = false;
	for(u32 i = 0; i < (u32)mDynamicShadowMaps.size(); i++)
	{
		ShadowMapAtlas& atlas = mDynamicShadowMaps[i];

		if(atlas.AddMap(options.MapSize, mapInfo.Area, kShadowMapBorder))
		{
			mapInfo.TextureIdx = i;

			foundSpace = true;
			break;
		}
	}

	if(!foundSpace)
	{
		mapInfo.TextureIdx = (u32)mDynamicShadowMaps.size();
		mDynamicShadowMaps.push_back(ShadowMapAtlas(kMaxAtlasSize));

		ShadowMapAtlas& atlas = mDynamicShadowMaps.back();
		atlas.AddMap(options.MapSize, mapInfo.Area, kShadowMapBorder);
	}

	mapInfo.UpdateNormArea(kMaxAtlasSize);
	ShadowMapAtlas& atlas = mDynamicShadowMaps[mapInfo.TextureIdx];

	ProfileGPUBlock profileSample("Project spot light shadows");

	commandBuffer.SetRenderTarget(atlas.GetTarget());
	commandBuffer.SetViewport(mapInfo.NormArea);
	commandBuffer.ClearViewport(FBT_DEPTH);

	mapInfo.DepthNear = 0.05f;
	mapInfo.DepthFar = light->GetAttenuationRadius();
	mapInfo.DepthFade = mapInfo.DepthFar;
	mapInfo.FadeRange = 0.0f;
	mapInfo.DepthRange = mapInfo.DepthFar - mapInfo.DepthNear;
	mapInfo.DepthBias = GetDepthBias(*light, light->GetBounds().GetRadius(), mapInfo.DepthRange, options.MapSize);
	mapInfo.SubjectBounds = light->GetBounds();

	Quaternion lightRotation = light->GetTransform().GetRotation();

	Matrix4 view = Matrix4::View(rendererLight.GetShiftedLightPosition(), lightRotation);
	Matrix4 proj = Matrix4::ProjectionPerspective(light->GetSpotAngle(), 1.0f, 0.05f, light->GetAttenuationRadius());

	ConvexVolume localFrustum = ConvexVolume(proj);
	RenderAPI::Instance().ConvertProjectionMatrix(proj, proj);

	mapInfo.ShadowVpTransform = proj * view;

	gShadowParamsDef.gDepthBias.Set(shadowParamsBuffer, mapInfo.DepthBias);
	gShadowParamsDef.gInvDepthRange.Set(shadowParamsBuffer, 1.0f / mapInfo.DepthRange);
	gShadowParamsDef.gMatViewProj.Set(shadowParamsBuffer, mapInfo.ShadowVpTransform);
	gShadowParamsDef.gNDCZToDeviceZ.Set(shadowParamsBuffer, RendererView::GetNdczToDeviceZ());

	const Vector<Plane>& frustumPlanes = localFrustum.GetPlanes();
	Matrix4 worldMatrix = view.InverseAffine();

	Vector<Plane> worldPlanes(frustumPlanes.size());
	u32 j = 0;
	for(auto& plane : frustumPlanes)
	{
		worldPlanes[j] = worldMatrix.MultiplyAffine(plane);
		j++;
	}

	ConvexVolume worldFrustum(worldPlanes);

	// Render all renderables into the shadow map
	ShadowRenderQueueSpotOptions spotOptions(
		worldFrustum,
		shadowParamsBuffer);

	ShadowRenderQueue::Execute(commandBuffer, scene, frameInfo, spotOptions);

	// Restore viewport
	commandBuffer.SetViewport(Rect2(0.0f, 0.0f, 1.0f, 1.0f));

	LightShadows& lightShadows = mSpotLightShadows[options.LightIdx];

	mShadowInfos[lightShadows.StartIdx + lightShadows.NumShadows] = mapInfo;
	lightShadows.NumShadows++;
}

void ShadowRendering::RenderRadialShadowMap(CommandBuffer& commandBuffer, const RendererLight& rendererLight, const ShadowMapOptions& options, RendererScene& scene, const FrameInfo& frameInfo)
{
	Light* light = rendererLight.Internal;

	SPtr<GpuBuffer> shadowParamsBuffer = gShadowParamsDef.CreateBuffer();

	ShadowInfo mapInfo;
	mapInfo.LightIdx = options.LightIdx;
	mapInfo.TextureIdx = -1;
	mapInfo.FadePerView = options.FadePercents;
	mapInfo.CascadeIdx = -1;
	mapInfo.Area = Rect2I(0, 0, options.MapSize, options.MapSize);
	mapInfo.UpdateNormArea(options.MapSize);

	for(u32 i = 0; i < (u32)mShadowCubemaps.size(); i++)
	{
		ShadowCubemap& cubemap = mShadowCubemaps[i];

		if(!cubemap.IsUsed() && cubemap.GetSize() == options.MapSize)
		{
			mapInfo.TextureIdx = i;
			cubemap.MarkAsUsed();

			break;
		}
	}

	if(mapInfo.TextureIdx == (u32)-1)
	{
		mapInfo.TextureIdx = (u32)mShadowCubemaps.size();
		mShadowCubemaps.push_back(ShadowCubemap(options.MapSize));

		ShadowCubemap& cubemap = mShadowCubemaps.back();
		cubemap.MarkAsUsed();
	}

	ShadowCubemap& cubemap = mShadowCubemaps[mapInfo.TextureIdx];

	mapInfo.DepthNear = 0.05f;
	mapInfo.DepthFar = light->GetAttenuationRadius();
	mapInfo.DepthFade = mapInfo.DepthFar;
	mapInfo.FadeRange = 0.0f;
	mapInfo.DepthRange = mapInfo.DepthFar - mapInfo.DepthNear;
	mapInfo.DepthBias = GetDepthBias(*light, light->GetBounds().GetRadius(), mapInfo.DepthRange, options.MapSize);
	mapInfo.SubjectBounds = light->GetBounds();

	// Note: Projecting on positive Z axis, because cubemaps use a left-handed coordinate system
	Matrix4 proj = Matrix4::ProjectionPerspective(Degree(90.0f), 1.0f, 0.05f, light->GetAttenuationRadius(), true);
	ConvexVolume localFrustum(proj);

	ProfileGPUBlock profileSample("Project radial light shadows");

	const GpuDeviceCapabilities& caps = GetGpuDeviceCapabilities();
	const GpuBackendConventions& rapiConventions = GetGpuDeviceCapabilities().Conventions;

	RenderAPI& rapi = RenderAPI::Instance();
	rapi.ConvertProjectionMatrix(proj, proj);

	// Render cubemaps upside down if necessary
	Matrix4 adjustedProj = proj;
	if(caps.Conventions.UvYAxis == GpuBackendConventions::Axis::Up)
	{
		// All big APIs use the same cubemap sampling coordinates, as well as the same face order. But APIs that
		// use bottom-up UV coordinates require the cubemap faces to be stored upside down in order to get the same
		// behaviour. APIs that use an upside-down NDC Y axis have the same problem as the rendered image will be
		// upside down, but this is handled by the projection matrix. If both of those are enabled, then the effect
		// cancels out.

		adjustedProj[1][1] = -proj[1][1];
	}

	bool renderAllFacesAtOnce = caps.HasCapability(RSC_RENDER_TARGET_LAYERS);

	SPtr<GpuBuffer> shadowCubeMatricesBuffer;
	SPtr<GpuBuffer> shadowCubeMasksBuffer;
	if(renderAllFacesAtOnce)
	{
		shadowCubeMatricesBuffer = gShadowCubeMatricesDef.CreateBuffer();
		shadowCubeMasksBuffer = gShadowCubeMasksDef.CreateBuffer();
	}

	gShadowParamsDef.gDepthBias.Set(shadowParamsBuffer, mapInfo.DepthBias);
	gShadowParamsDef.gInvDepthRange.Set(shadowParamsBuffer, 1.0f / mapInfo.DepthRange);
	gShadowParamsDef.gMatViewProj.Set(shadowParamsBuffer, Matrix4::kIdentity);
	gShadowParamsDef.gNDCZToDeviceZ.Set(shadowParamsBuffer, RendererView::GetNdczToDeviceZ());

	ConvexVolume frustums[6];
	Vector<Plane> boundingPlanes;
	for(u32 i = 0; i < 6; i++)
	{
		// Calculate view matrix
		Vector3 forward;
		Vector3 up = Vector3::kUnitY;

		switch(i)
		{
		case CF_PositiveX:
			forward = Vector3::kUnitX;
			break;
		case CF_NegativeX:
			forward = -Vector3::kUnitX;
			break;
		case CF_PositiveY:
			forward = Vector3::kUnitY;
			up = -Vector3::kUnitZ;
			break;
		case CF_NegativeY:
			forward = -Vector3::kUnitY;
			up = Vector3::kUnitZ;
			break;
		case CF_PositiveZ:
			forward = Vector3::kUnitZ;
			break;
		case CF_NegativeZ:
			forward = -Vector3::kUnitZ;
			break;
		}

		Vector3 right = Vector3::Cross(up, forward);
		Matrix3 viewRotationMat = Matrix3(right, up, forward);

		Vector3 lightPos = light->GetTransform().GetPosition();
		Matrix4 viewOffsetMat = Matrix4::Translation(-lightPos);

		Matrix4 view = Matrix4(viewRotationMat.Transpose()) * viewOffsetMat;
		mapInfo.ShadowVpTransforms[i] = proj * view;

		Matrix4 shadowViewProj = adjustedProj * view;

		// Calculate world frustum for culling
		const Vector<Plane>& frustumPlanes = localFrustum.GetPlanes();

		Matrix4 worldMatrix = Matrix4::Translation(lightPos) * Matrix4(viewRotationMat);

		Vector<Plane> worldPlanes(frustumPlanes.size());
		u32 j = 0;
		for(auto& plane : frustumPlanes)
		{
			worldPlanes[j] = worldMatrix.MultiplyAffine(plane);
			j++;
		}

		ConvexVolume frustum(worldPlanes);

		if(renderAllFacesAtOnce)
		{
			frustums[i] = frustum;

			// Register far plane of all frustums
			boundingPlanes.push_back(worldPlanes[FRUSTUM_PLANE_FAR]);
			gShadowCubeMatricesDef.gFaceVPMatrices.Set(shadowCubeMatricesBuffer, shadowViewProj, i);
		}
		else
		{
			gShadowParamsDef.gMatViewProj.Set(shadowParamsBuffer, shadowViewProj);

			RENDER_TEXTURE_DESC rtDesc;
			rtDesc.DepthStencilSurface.Texture = cubemap.GetTexture();
			rtDesc.DepthStencilSurface.Face = i;
			rtDesc.DepthStencilSurface.NumFaces = 1;

			SPtr<RenderTarget> faceRt = RenderTexture::Create(rtDesc);

			commandBuffer.SetRenderTarget(faceRt);
			commandBuffer.ClearRenderTarget(FBT_DEPTH);

			// Render all renderables into the shadow map
			ConvexVolume boundingVolume(boundingPlanes);
			ShadowRenderQueueCubeSingleOptions cubeOptions(
				frustum,
				shadowParamsBuffer);

			ShadowRenderQueue::Execute(commandBuffer, scene, frameInfo, cubeOptions);
		}
	}

	if(renderAllFacesAtOnce)
	{
		commandBuffer.SetRenderTarget(cubemap.GetTarget());
		commandBuffer.ClearRenderTarget(FBT_DEPTH);

		// Render all renderables into the shadow map
		ConvexVolume boundingVolume(boundingPlanes);
		ShadowRenderQueueCubeOptions cubeOptions(
			frustums,
			boundingVolume,
			shadowParamsBuffer,
			shadowCubeMatricesBuffer,
			shadowCubeMasksBuffer);

		ShadowRenderQueue::Execute(commandBuffer, scene, frameInfo, cubeOptions);
	}

	LightShadows& lightShadows = mRadialLightShadows[options.LightIdx];

	mShadowInfos[lightShadows.StartIdx + lightShadows.NumShadows] = mapInfo;
	lightShadows.NumShadows++;
}

void ShadowRendering::CalcShadowMapProperties(const RendererLight& light, const RendererViewGroup& viewGroup, u32 border, u32& size, SmallVector<float, 6>& fadePercents, float& maxFadePercent) const
{
	const static float kShadowTexelsPerPixel = 1.0f;

	// Find a view in which the light has the largest radius
	float maxMapSize = 0.0f;
	maxFadePercent = 0.0f;
	for(int i = 0; i < (int)viewGroup.GetNumViews(); ++i)
	{
		const RendererView& view = *viewGroup.GetView(i);
		const RendererViewProperties& viewProps = view.GetProperties();
		const RenderSettings& viewSettings = view.GetRenderSettings();

		if(!viewSettings.EnableShadows)
			fadePercents.Add(0.0f);
		else
		{
			// Approximation for screen space sphere radius: screenSize * 0.5 * cot(fov) * radius / Z, where FOV is the
			// largest one
			//// First get sphere depth
			const Matrix4& viewVP = viewProps.ViewProjTransform;
			float depth = viewVP.Multiply(Vector4(light.Internal->GetTransform().GetPosition(), 1.0f)).W;

			// This is just 1/tan(fov), for both horz. and vert. FOV
			float viewScaleX = viewProps.ProjTransform[0][0];
			float viewScaleY = viewProps.ProjTransform[1][1];

			float screenScaleX = viewScaleX * viewProps.Target.ViewRect.Width * 0.5f;
			float screenScaleY = viewScaleY * viewProps.Target.ViewRect.Height * 0.5f;

			float screenScale = std::max(screenScaleX, screenScaleY);

			//// Calc radius (clamp if too close to avoid massive numbers)
			float radiusNDC = light.Internal->GetBounds().GetRadius() / std::max(depth, 1.0f);

			//// Radius of light bounds in percent of the view surface, multiplied by screen size in pixels
			float radiusScreen = radiusNDC * screenScale;

			float optimalMapSize = kShadowTexelsPerPixel * radiusScreen;
			maxMapSize = std::max(maxMapSize, optimalMapSize);

			// Determine if the shadow should fade out
			float fadePercent = Math::InvLerp(optimalMapSize, (float)kMinShadowMapSize, (float)kShadowMapFadeSize);
			fadePercents.Add(fadePercent);
			maxFadePercent = std::max(maxFadePercent, fadePercent);
		}
	}

	// If light fully (or nearly fully) covers the screen, use full shadow map resolution, otherwise
	// scale it down to smaller power of two, while clamping to minimal allowed resolution
	u32 effectiveMapSize = Bitwise::NextPow2((u32)maxMapSize);
	effectiveMapSize = Math::Clamp(effectiveMapSize, kMinShadowMapSize, mShadowMapSize);

	// Leave room for border
	size = std::max(effectiveMapSize - 2 * border, 1u);
}

void ShadowRendering::DrawNearFarPlanes(CommandBuffer& commandBuffer, float near, float far, bool drawNear) const
{
	const GpuBackendConventions& rapiConventions = GetGpuDeviceCapabilities().Conventions;
	float flipY = (rapiConventions.NdcYAxis == GpuBackendConventions::Axis::Down) ? -1.0f : 1.0f;

	// Update VB with new vertices
	Vector3 vertices[8] = {
		// Near plane
		{ -1.0f, -1.0f * flipY, near },
		{ 1.0f, -1.0f * flipY, near },
		{ 1.0f, 1.0f * flipY, near },
		{ -1.0f, 1.0f * flipY, near },

		// Far plane
		{ -1.0f, -1.0f * flipY, far },
		{ 1.0f, -1.0f * flipY, far },
		{ 1.0f, 1.0f * flipY, far },
		{ -1.0f, 1.0f * flipY, far },
	};

	mPlaneVB->WriteData(0, sizeof(vertices), vertices, BWT_DISCARD);

	// Draw the mesh
	commandBuffer.SetVertexDescription(mPositionOnlyVertexDescription);
	commandBuffer.SetVertexBuffers(0, &mPlaneVB, 1);
	commandBuffer.SetIndexBuffer(mPlaneIB);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	commandBuffer.DrawIndexed(0, drawNear ? 12 : 6, 0, drawNear ? 8 : 4);
}

void ShadowRendering::DrawFrustum(CommandBuffer& commandBuffer, const std::array<Vector3, 8>& corners) const
{
	// Update VB with new vertices
	mFrustumVB->WriteData(0, sizeof(Vector3) * 8, corners.data(), BWT_DISCARD);

	// Draw the mesh
	commandBuffer.SetVertexDescription(mPositionOnlyVertexDescription);
	commandBuffer.SetVertexBuffers(0, &mFrustumVB, 1);
	commandBuffer.SetIndexBuffer(mFrustumIB);
	commandBuffer.SetDrawOperation(DOT_TRIANGLE_LIST);

	commandBuffer.DrawIndexed(0, 36, 0, 8);
}

u32 ShadowRendering::GetShadowQuality(u32 requestedQuality, u32 shadowMapResolution, u32 minAllowedQuality)
{
	static const u32 kTargetResolution = 512;

	// If shadow map resolution is smaller than some target resolution drop the number of PCF samples (shadow quality)
	// so that the penumbra better matches with larger sized shadow maps.
	while(requestedQuality > minAllowedQuality && shadowMapResolution < kTargetResolution)
	{
		shadowMapResolution *= 2;
		requestedQuality = std::max(requestedQuality - 1, 1U);
	}

	return requestedQuality;
}

ConvexVolume ShadowRendering::GetCsmSplitFrustum(const RendererView& view, const Vector3& lightDir, u32 cascade, u32 numCascades, Sphere& outBounds)
{
	// Determine split range
	float splitNear = GetCsmSplitDistance(view, cascade, numCascades);
	float splitFar = GetCsmSplitDistance(view, cascade + 1, numCascades);

	// Increase by fade range, unless last cascade
	if((u32)(cascade + 1) < numCascades)
		splitFar += kCascadeFractionFade * (splitFar - splitNear);

	// Calculate the eight vertices of the split frustum
	auto& viewProps = view.GetProperties();

	const Matrix4& projMat = viewProps.ProjTransform;

	float aspect;
	float nearHalfWidth, nearHalfHeight;
	float farHalfWidth, farHalfHeight;
	if(viewProps.ProjType == PT_PERSPECTIVE)
	{
		aspect = fabs(projMat[0][0] / projMat[1][1]);
		float tanHalfFOV = 1.0f / projMat[0][0];

		nearHalfWidth = splitNear * tanHalfFOV;
		nearHalfHeight = nearHalfWidth * aspect;

		farHalfWidth = splitFar * tanHalfFOV;
		farHalfHeight = farHalfWidth * aspect;
	}
	else
	{
		aspect = projMat[0][0] / projMat[1][1];

		nearHalfWidth = farHalfWidth = projMat[0][0] / 4.0f;
		nearHalfHeight = farHalfHeight = projMat[1][1] / 4.0f;
	}

	const Matrix4& viewMat = viewProps.ViewTransform;
	Vector3 cameraRight = Vector3(viewMat[0]);
	Vector3 cameraUp = Vector3(viewMat[1]);

	const Vector3& viewOrigin = viewProps.ViewOrigin;
	const Vector3& viewDir = viewProps.ViewDirection;

	Vector3 frustumVerts[] = {
		viewOrigin + viewDir * splitNear - cameraRight * nearHalfWidth + cameraUp * nearHalfHeight, // Near, left, top
		viewOrigin + viewDir * splitNear + cameraRight * nearHalfWidth + cameraUp * nearHalfHeight, // Near, right, top
		viewOrigin + viewDir * splitNear + cameraRight * nearHalfWidth - cameraUp * nearHalfHeight, // Near, right, bottom
		viewOrigin + viewDir * splitNear - cameraRight * nearHalfWidth - cameraUp * nearHalfHeight, // Near, left, bottom
		viewOrigin + viewDir * splitFar - cameraRight * farHalfWidth + cameraUp * farHalfHeight, // Far, left, top
		viewOrigin + viewDir * splitFar + cameraRight * farHalfWidth + cameraUp * farHalfHeight, // Far, right, top
		viewOrigin + viewDir * splitFar + cameraRight * farHalfWidth - cameraUp * farHalfHeight, // Far, right, bottom
		viewOrigin + viewDir * splitFar - cameraRight * farHalfWidth - cameraUp * farHalfHeight, // Far, left, bottom
	};

	// Calculate the bounding sphere of the frustum
	float diagonalNearSq = nearHalfWidth * nearHalfWidth + nearHalfHeight * nearHalfHeight;
	float diagonalFarSq = farHalfWidth * farHalfWidth + farHalfHeight * farHalfHeight;

	float length = splitFar - splitNear;
	float offset = (diagonalNearSq - diagonalFarSq) / (2 * length) + length * 0.5f;
	float distToCenter = Math::Clamp(splitFar - offset, splitNear, splitFar);

	Vector3 center = viewOrigin + viewDir * distToCenter;

	float radius = 0.0f;
	for(auto& entry : frustumVerts)
		radius = std::max(radius, center.SquaredDistance(entry));

	radius = std::max((float)sqrt(radius), 1.0f);
	outBounds = Sphere(center, radius);

	// Generate light frustum planes
	Plane viewPlanes[6];
	viewPlanes[FRUSTUM_PLANE_NEAR] = Plane(frustumVerts[0], frustumVerts[1], frustumVerts[2]);
	viewPlanes[FRUSTUM_PLANE_FAR] = Plane(frustumVerts[5], frustumVerts[4], frustumVerts[7]);
	viewPlanes[FRUSTUM_PLANE_LEFT] = Plane(frustumVerts[4], frustumVerts[0], frustumVerts[3]);
	viewPlanes[FRUSTUM_PLANE_RIGHT] = Plane(frustumVerts[1], frustumVerts[5], frustumVerts[6]);
	viewPlanes[FRUSTUM_PLANE_TOP] = Plane(frustumVerts[4], frustumVerts[5], frustumVerts[1]);
	viewPlanes[FRUSTUM_PLANE_BOTTOM] = Plane(frustumVerts[3], frustumVerts[2], frustumVerts[6]);

	//// Add camera's planes facing towards the lights (forming the back of the volume)
	Vector<Plane> lightVolume;
	for(auto& entry : viewPlanes)
	{
		if(entry.Normal.Dot(lightDir) < 0.0f)
			lightVolume.push_back(entry);
	}

	//// Determine edge planes by testing adjacent planes with different facing
	////// Pairs of frustum planes that share an edge
	u32 adjacentPlanes[][2] = {
		{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_LEFT },
		{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_RIGHT },
		{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_TOP },
		{ FRUSTUM_PLANE_NEAR, FRUSTUM_PLANE_BOTTOM },

		{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_LEFT },
		{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_RIGHT },
		{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_TOP },
		{ FRUSTUM_PLANE_FAR, FRUSTUM_PLANE_BOTTOM },

		{ FRUSTUM_PLANE_LEFT, FRUSTUM_PLANE_TOP },
		{ FRUSTUM_PLANE_TOP, FRUSTUM_PLANE_RIGHT },
		{ FRUSTUM_PLANE_RIGHT, FRUSTUM_PLANE_BOTTOM },
		{ FRUSTUM_PLANE_BOTTOM, FRUSTUM_PLANE_LEFT },
	};

	////// Vertex indices of edges on the boundary between two planes
	u32 sharedEdges[][2] = {
		{ 3, 0 }, { 1, 2 }, { 0, 1 }, { 2, 3 }, { 4, 7 }, { 6, 5 }, { 5, 4 }, { 7, 6 }, { 4, 0 }, { 5, 1 }, { 6, 2 }, { 7, 3 }
	};

	for(u32 i = 0; i < 12; i++)
	{
		const Plane& planeA = viewPlanes[adjacentPlanes[i][0]];
		const Plane& planeB = viewPlanes[adjacentPlanes[i][1]];

		float dotA = planeA.Normal.Dot(lightDir);
		float dotB = planeB.Normal.Dot(lightDir);

		if((dotA * dotB) < 0.0f)
		{
			const Vector3& vertA = frustumVerts[sharedEdges[i][0]];
			const Vector3& vertB = frustumVerts[sharedEdges[i][1]];
			Vector3 vertC = vertA + lightDir;

			if(dotA < 0.0f)
				lightVolume.push_back(Plane(vertA, vertB, vertC));
			else
				lightVolume.push_back(Plane(vertB, vertA, vertC));
		}
	}

	return ConvexVolume(lightVolume);
}

float ShadowRendering::GetCsmSplitDistance(const RendererView& view, u32 index, u32 numCascades)
{
	auto& shadowSettings = view.GetRenderSettings().ShadowSettings;
	float distributionExponent = shadowSettings.CascadeDistributionExponent;

	// First determine the scale of the split, relative to the entire range
	float scaleModifier = 1.0f;
	float scale = 0.0f;
	float totalScale = 0.0f;

	//// Split 0 corresponds to near plane
	if(index > 0)
	{
		for(u32 i = 0; i < numCascades; i++)
		{
			if(i < index)
				scale += scaleModifier;

			totalScale += scaleModifier;
			scaleModifier *= distributionExponent;
		}

		scale = scale / totalScale;
	}

	// Calculate split distance in Z
	auto& viewProps = view.GetProperties();
	float near = viewProps.NearPlane;
	float far = Math::Clamp(shadowSettings.DirectionalShadowDistance, viewProps.NearPlane, viewProps.FarPlane);

	return near + (far - near) * scale;
}

float ShadowRendering::GetDepthBias(const Light& light, float radius, float depthRange, u32 mapSize)
{
	const static float kRadialLightBias = 0.005f;
	const static float kSpotDepthBias = 0.01f;
	const static float kDirDepthBias = 0.001f; // In clip space units
	const static float kDefaultResolution = 512.0f;

	// Increase bias if map size smaller than some resolution
	float resolutionScale = 1.0f;

	if(light.GetType() != LightType::Directional)
		resolutionScale = kDefaultResolution / (float)mapSize;

	// Adjust range because in shader we compare vs. clip space depth
	float rangeScale = 1.0f;
	if(light.GetType() == LightType::Spot)
		rangeScale = 1.0f / depthRange;

	const GpuDeviceCapabilities& caps = GetGpuDeviceCapabilities();
	float deviceDepthRange = caps.MaxDepth - caps.MinDepth;

	float defaultBias = 1.0f;
	switch(light.GetType())
	{
	case LightType::Directional:
		defaultBias = kDirDepthBias * deviceDepthRange;

		// Use larger bias for further away cascades
		defaultBias *= depthRange * 0.01f;
		break;
	case LightType::Radial:
		defaultBias = kRadialLightBias;
		break;
	case LightType::Spot:
		defaultBias = kSpotDepthBias;
		break;
	default:
		break;
	}

	return defaultBias * light.GetShadowBias() * resolutionScale * rangeScale;
}

float ShadowRendering::GetFadeTransition(const Light& light, float radius, float depthRange, u32 mapSize)
{
	const static float kSpotLightScale = 1000.0f;
	const static float kDirLightScale = 50000000.0f;

	// Note: Currently fade transitions are only used in spot & directional (non omni-directional) lights, so no need
	// to account for radial light type.
	if(light.GetType() == LightType::Directional)
	{
		// Just use a large value, as we want a minimal transition region
		return kDirLightScale;
	}
	else
		return fabs(light.GetShadowBias()) * kSpotLightScale;
}
}}
