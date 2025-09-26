//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/B3DSprite.h"
#include "CoreObject/B3DCoreObjectManager.h"
#include "Material/B3DMaterial.h"
#include "Image/B3DTexture.h"
#include "Mesh/B3DMesh.h"
#include "Material/B3DShader.h"
#include "Renderer/B3DRendererUtility.h"
#include "Material/B3DGpuParamsSet.h"
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
	mWithClippingVariation.VariationIndex = mMaterial->FindTechnique(findVariationInformation);

	variation.SetBool("ENABLE_CLIPPING", false);
	mWithoutClippingVariation.VariationIndex = mMaterial->FindTechnique(findVariationInformation);

	mMaterialStored.store(true, std::memory_order_release);

	GetRenderThread().PostCommand([this] { Initialize(); }, "SpriteMaterial::Initialize");
}

SpriteMaterial::~SpriteMaterial()
{
	GetRenderThread().PostCommand([material = mMaterial, withClippingParameterSet = mWithClippingVariation.ParameterSet, withoutClippingParameterSet = mWithoutClippingVariation.ParameterSet] { Destroy(material, withClippingParameterSet, withoutClippingParameterSet); }, "SpriteMaterial::Destroy");
}

void SpriteMaterial::Initialize()
{
	// Make sure that mMaterial assignment completes on the previous thread before continuing
	const bool materialStored = mMaterialStored.load(std::memory_order_acquire);
	B3D_ASSERT(materialStored == true);

	auto fnPrepareVariation = [this](MaterialVariationInformation& variationInformation)
	{
		const SPtr<render::Technique> variation = mMaterial->GetTechnique(variationInformation.VariationIndex);
		B3D_ASSERT(variation != nullptr);

		if(!variation->IsCompiled())
		{
			const TAsyncOp<bool> operation = variation->Compile();
			operation.BlockUntilComplete();
		}

		const SPtr<render::Pass>& pass = mMaterial->GetPass(0, variationInformation.VariationIndex);

		if(pass)
			pass->Compile();

		variationInformation.ParameterSet = mMaterial->CreateParamsSet(variationInformation.VariationIndex);
		variationInformation.UniformBufferIndex = variationInformation.ParameterSet->GetParamBlockBufferIndex("GUIParams");

		const SPtr<render::GpuParameters>& withClippingVariationGpuParameters = variationInformation.ParameterSet->GetGpuParams(0);
		withClippingVariationGpuParameters->GetStorageBufferParameter("gVertices", variationInformation.VerticesBufferParameter);
		withClippingVariationGpuParameters->TryGetStorageBufferParameter("gClipRegions", variationInformation.ClipRegionsBufferParameter);
	};

	fnPrepareVariation(mWithClippingVariation);
	fnPrepareVariation(mWithoutClippingVariation);

	SPtr<render::Shader> shader = mMaterial->GetShader();
	if(shader->HasTextureParam("gMainTexture"))
	{
		mTextureParameter = mMaterial->GetParamTexture("gMainTexture");
		mSamplerParameter = mMaterial->GetParamSamplerState("gMainTexSamp");
	}

	if(mWithClippingVariation.UniformBufferIndex == ~0u || mWithoutClippingVariation.UniformBufferIndex == ~0u)
		B3D_LOG(Error, GUI, "Sprite material shader missing \"GUIParams\" block.");
}

void SpriteMaterial::Destroy(const SPtr<render::Material>& material, const SPtr<render::GpuParamsSet>& withClippingParams, const SPtr<render::GpuParamsSet>& withoutClippingParams)
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

void SpriteMaterial::Render(render::GpuCommandBuffer& commandBuffer, const SPtr<render::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<render::Texture>& texture, const SPtr<SamplerState>& sampler, const SPtr<render::GpuBuffer>& uniformBuffer, const SPtr<render::GpuBuffer>& clipRegionBuffer, u32 clipRegionCount, const SPtr<SpriteMaterialExtraInfo>& additionalData) const
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

	if(clipRegionBuffer != nullptr)
	{
		mMaterial->UpdateParamsSet(mWithClippingVariation.ParameterSet);

		mWithClippingVariation.ParameterSet->SetParamBlockBuffer(mWithClippingVariation.UniformBufferIndex, uniformBuffer);
		mWithClippingVariation.ClipRegionsBufferParameter.Set(clipRegionBuffer);
		mWithClippingVariation.VerticesBufferParameter.Set(vertexBuffer);

		render::GetRendererUtility().SetPass(commandBuffer, mMaterial, 0, mWithClippingVariation.VariationIndex);
		render::GetRendererUtility().SetPassParams(commandBuffer, mWithClippingVariation.ParameterSet);

		render::GetRendererUtility().Draw(commandBuffer, mesh, subMesh, clipRegionCount);
	}
	else
	{
		mMaterial->UpdateParamsSet(mWithoutClippingVariation.ParameterSet);

		mWithoutClippingVariation.ParameterSet->SetParamBlockBuffer(mWithoutClippingVariation.UniformBufferIndex, uniformBuffer);
		mWithoutClippingVariation.VerticesBufferParameter.Set(vertexBuffer);

		render::GetRendererUtility().SetPass(commandBuffer, mMaterial, 0, mWithoutClippingVariation.VariationIndex);
		render::GetRendererUtility().SetPassParams(commandBuffer, mWithoutClippingVariation.ParameterSet);
		
		render::GetRendererUtility().Draw(commandBuffer, mesh, subMesh);
	}
}

void SpriteMaterial::PopulateUniformBuffer(const SPtr<render::GpuBuffer>& buffer, const Vector2I& viewportOffset, float inverseViewportWidth, float inverseViewportHeight, bool flipY, float animationTime, u32 clipRegionCount, const Matrix4& transform, const render::SpriteMaterialInfo& materialInformation)
{
	render::gGUISpriteUniformBufferDefinition.gTint.Set(buffer, materialInformation.Tint);
	render::gGUISpriteUniformBufferDefinition.gWorldTransform.Set(buffer, transform);
	render::gGUISpriteUniformBufferDefinition.gInvViewportWidth.Set(buffer, inverseViewportWidth);
	render::gGUISpriteUniformBufferDefinition.gInvViewportHeight.Set(buffer, inverseViewportHeight);
	render::gGUISpriteUniformBufferDefinition.gViewportOffset.Set(buffer, viewportOffset);
	render::gGUISpriteUniformBufferDefinition.gViewportYFlip.Set(buffer, flipY ? -1.0f : 1.0f);
	render::gGUISpriteUniformBufferDefinition.gClipRegionCount.Set(buffer, clipRegionCount);

	const float t = std::max(0.0f, animationTime - materialInformation.AnimationStartTime);
	if(materialInformation.SpriteImage)
	{
		u32 row;
		u32 column;
		materialInformation.SpriteImage->GetAnimationFrame(t, row, column);

		const float inverseWidth = 1.0f / materialInformation.SpriteImage->GetAnimation().ColumnCount;
		const float inverseHeight = 1.0f / materialInformation.SpriteImage->GetAnimation().RowCount;

		Vector4 sizeOffset(inverseWidth, inverseHeight, column * inverseWidth, row * inverseHeight);
		render::gGUISpriteUniformBufferDefinition.gUVSizeOffset.Set(buffer, sizeOffset);
	}
	else
		render::gGUISpriteUniformBufferDefinition.gUVSizeOffset.Set(buffer, Vector4(1.0f, 1.0f, 0.0f, 0.0f));

	buffer->FlushCache();
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


