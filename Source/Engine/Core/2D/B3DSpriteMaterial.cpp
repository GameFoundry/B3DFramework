//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/B3DSprite.h"
#include "CoreObject/B3DCoreObjectManager.h"
#include "Material/B3DMaterial.h"
#include "Image/B3DTexture.h"
#include "Mesh/B3DMesh.h"
#include "Material/B3DShader.h"
#include "Renderer/B3DRendererUtility.h"
#include "Material/B3DMaterialParameterAdapter.h"
#include "CoreObject/B3DRenderThread.h"
#include "Image/B3DSpriteTexture.h"

using namespace b3d;

SpriteMaterial::SpriteMaterial(u32 id, const HMaterial& material, ShaderVariationParameters variation, bool allowBatching)
	: mId(id), mAllowBatching(allowBatching), mMaterialStored(false)
{
	mMaterial = B3DGetRenderProxy(material);

	FindVariationInformation findVariationInformation;
	findVariationInformation.VariationParameters = &variation;

	variation.SetBool("ENABLE_CLIPPING", true);
	mWithClippingVariationIndex = mMaterial->FindVariation(findVariationInformation);

	variation.SetBool("ENABLE_CLIPPING", false);
	mWithoutClippingVariationIndex = mMaterial->FindVariation(findVariationInformation);

	mMaterialStored.store(true, std::memory_order_release);

	GetRenderThread().PostCommand([this] { Initialize(); }, "SpriteMaterial::Initialize");
}

SpriteMaterial::~SpriteMaterial()
{
	GetRenderThread().PostCommand([material = mMaterial] { Destroy(material); }, "SpriteMaterial::Destroy");
}

void SpriteMaterial::Initialize()
{
	// Make sure that mMaterial assignment completes on the previous thread before continuing
	const bool materialStored = mMaterialStored.load(std::memory_order_acquire);
	B3D_ASSERT(materialStored == true);

	auto fnPrepareVariation = [this](u32 variationIndex)
	{
		const SPtr<render::Variation> variation = mMaterial->GetVariation(variationIndex);
		B3D_ASSERT(variation != nullptr);

		if(!variation->IsCompiled())
		{
			const TAsyncOp<bool> operation = variation->Compile();
			operation.BlockUntilComplete();
		}

		const SPtr<render::Pass>& pass = mMaterial->GetPass(0, variationIndex);

		if(pass)
			pass->Compile();
	};

	fnPrepareVariation(mWithClippingVariationIndex);
	fnPrepareVariation(mWithoutClippingVariationIndex);

	SPtr<render::Shader> shader = mMaterial->GetShader();
	if(shader->HasTextureParam("gMainTexture"))
	{
		mTextureParameter = mMaterial->GetParamTexture("gMainTexture");
		mSamplerParameter = mMaterial->GetParamSamplerState("gMainTexSamp");
	}
}

void SpriteMaterial::Destroy(const SPtr<render::Material>& material)
{
	// Do nothing, we just need to make sure the material pointer's last reference is lost while on the render thread
}

u64 SpriteMaterial::GetMergeHash(const SpriteMaterialInfo& info) const
{
	u64 textureId = 0;
	if(info.Texture.IsLoaded())
		textureId = info.Texture->GetInternalId();

	size_t hash = 0;
	B3DCombineHash(hash, info.GroupId);
	B3DCombineHash(hash, GetId());
	B3DCombineHash(hash, textureId);
	B3DCombineHash(hash, info.Tint);

	return (u64)hash;
}

SPtr<render::MaterialParameterAdapter> SpriteMaterial::CreateParameterAdapter(bool supportClipping)
{
	return mMaterial->CreateParameterAdapter(supportClipping ? mWithClippingVariationIndex : mWithoutClippingVariationIndex);
}

void SpriteMaterial::Prepare(const SPtr<render::MaterialParameterAdapter>& parameterAdapter, const SPtr<render::MeshBase>& mesh, const SPtr<render::Texture>& texture, const SPtr<SamplerState>& sampler, const render::GpuBufferSuballocation& uniformBuffer, const SPtr<render::GpuBuffer>& clipRegionBuffer) const
{
	SPtr<render::Texture> spriteTexture;
	if(texture != nullptr)
		spriteTexture = texture;
	else
		spriteTexture = render::Texture::kWhite;

	mTextureParameter.Set(spriteTexture);
	mSamplerParameter.Set(sampler);

	const SPtr<render::VertexData>& vertexData = mesh->GetVertexData();
	const SPtr<render::GpuBuffer>& vertexBuffer = vertexData->GetBuffer(0);

	parameterAdapter->Update(mMaterial);

	parameterAdapter->SetUniformBuffer("GUIParams", uniformBuffer);

	const SPtr<render::GpuParameterSet>& gpuParameters = parameterAdapter->GetGpuParameterSet();
	gpuParameters->SetStorageBuffer("gVertices", vertexBuffer);

	if(clipRegionBuffer != nullptr)
		gpuParameters->SetStorageBuffer("gClipRegions", clipRegionBuffer);
}


void SpriteMaterial::Render(render::GpuCommandBuffer& commandBuffer, const SPtr<render::GpuParameterSet>& parameters, const SPtr<render::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<render::GpuBuffer>& clipRegionBuffer, u32 clipRegionCount, const SPtr<SpriteMaterialExtraInfo>& additionalData) const
{
	if(clipRegionBuffer != nullptr)
	{
		render::GetRendererUtility().SetPass(commandBuffer, mMaterial, 0, mWithClippingVariationIndex);
		commandBuffer.SetGpuParameterSet(parameters);

		render::GetRendererUtility().Draw(commandBuffer, mesh, subMesh, clipRegionCount);
	}
	else
	{
		render::GetRendererUtility().SetPass(commandBuffer, mMaterial, 0, mWithoutClippingVariationIndex);
		commandBuffer.SetGpuParameterSet(parameters);
		
		render::GetRendererUtility().Draw(commandBuffer, mesh, subMesh);
	}
}

void SpriteMaterial::PopulateUniformBuffer(const render::GpuBufferMappedScope& uniforms, const Vector2I& viewportOffset, float inverseViewportWidth, float inverseViewportHeight, bool flipY, float animationTime, u32 clipRegionCount, const Matrix4& transform, const render::SpriteMaterialInfo& materialInformation)
{
	render::gGUISpriteUniformBufferDefinition.gTint.Set(uniforms, materialInformation.Tint);
	render::gGUISpriteUniformBufferDefinition.gWorldTransform.Set(uniforms, transform);
	render::gGUISpriteUniformBufferDefinition.gInvViewportWidth.Set(uniforms, inverseViewportWidth);
	render::gGUISpriteUniformBufferDefinition.gInvViewportHeight.Set(uniforms, inverseViewportHeight);
	render::gGUISpriteUniformBufferDefinition.gViewportOffset.Set(uniforms, viewportOffset);
	render::gGUISpriteUniformBufferDefinition.gViewportYFlip.Set(uniforms, flipY ? -1.0f : 1.0f);
	render::gGUISpriteUniformBufferDefinition.gClipRegionCount.Set(uniforms, clipRegionCount);

	const float t = std::max(0.0f, animationTime - materialInformation.AnimationStartTime);
	if(materialInformation.SpriteImage)
	{
		u32 row;
		u32 column;
		materialInformation.SpriteImage->GetAnimationFrame(t, row, column);

		const float inverseWidth = 1.0f / materialInformation.SpriteImage->GetAnimation().ColumnCount;
		const float inverseHeight = 1.0f / materialInformation.SpriteImage->GetAnimation().RowCount;

		Vector4 sizeOffset(inverseWidth, inverseHeight, column * inverseWidth, row * inverseHeight);
		render::gGUISpriteUniformBufferDefinition.gUVSizeOffset.Set(uniforms, sizeOffset);
	}
	else
		render::gGUISpriteUniformBufferDefinition.gUVSizeOffset.Set(uniforms, Vector4(1.0f, 1.0f, 0.0f, 0.0f));
}

namespace b3d
{
	namespace render
	{
		GUISpriteUniformBufferDefinition gGUISpriteUniformBufferDefinition;

		SpriteMaterialInfo::SpriteMaterialInfo(const TSpriteMaterialInfo<false>& other)
		{
			GroupId = other.GroupId;
			Texture = B3DGetRenderProxy(other.Texture);
			SpriteImage = B3DGetRenderProxy(other.SpriteImage);
			Tint = other.Tint;
			AnimationStartTime = other.AnimationStartTime;
			AdditionalData = other.AdditionalData;
		}
	} // namespace render
}


