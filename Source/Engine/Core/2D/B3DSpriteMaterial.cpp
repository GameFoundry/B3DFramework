//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/B3DSprite.h"
#include "2D/B3DSpriteMaterial.h"
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

namespace b3d
{
	TConfigVariable<bool> gGuiUseLinearColorSpace("gui.UseLinearColorSpace",
		"If true (default), GUI/sprite/vector content is composited in linear color space: input colors and "
		"sRGB-imported textures are decoded to linear, blending happens in linear, and the result is re-encoded to "
		"sRGB on output. If false, compositing happens in gamma (sRGB-encoded) space (matching web browsers). "
		"UI source textures should be imported as sRGB when true and as linear when false.",
		true,
		ConfigVariableFlag::ReadOnly);
}

SpriteMaterial::SpriteMaterial(u32 id, const HMaterial& material, ShaderVariationParameters variation, bool allowBatching)
	: mId(id), mAllowBatching(allowBatching), mMaterialStored(false)
{
	mMaterial = B3DGetRenderProxy(material);

	FindVariationInformation findVariationInformation;
	findVariationInformation.VariationParameters = &variation;

	variation.SetBool("ENABLE_CLIPPING", true);
	mWithClippingVariationIndex = material->FindVariation(findVariationInformation);

	variation.SetBool("ENABLE_CLIPPING", false);
	mWithoutClippingVariationIndex = material->FindVariation(findVariationInformation);

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
		const TShared<render::Variation> variation = mMaterial->GetVariation(variationIndex);
		B3D_ASSERT(variation != nullptr);

		if(!variation->IsCompiled())
		{
			const TAsyncOp<bool> operation = variation->Compile();
			operation.BlockUntilComplete();
		}

		const TShared<render::Pass>& pass = mMaterial->GetPass(0, variationIndex);

		if(pass)
			pass->Compile();
	};

	fnPrepareVariation(mWithClippingVariationIndex);
	fnPrepareVariation(mWithoutClippingVariationIndex);

	TShared<render::Shader> shader = mMaterial->GetShader();
	if(shader->HasTextureParameter("gMainTexture"))
	{
		mTextureParameter = mMaterial->GetParamTexture("gMainTexture");
		mSamplerParameter = mMaterial->GetParamSamplerState("gMainTexSamp");
	}
}

void SpriteMaterial::Destroy(const TShared<render::Material>& material)
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

TShared<render::MaterialParameterAdapter> SpriteMaterial::CreateParameterAdapter(bool supportClipping)
{
	return mMaterial->CreateParameterAdapter(supportClipping ? mWithClippingVariationIndex : mWithoutClippingVariationIndex);
}

void SpriteMaterial::Prepare(const TShared<render::MaterialParameterAdapter>& parameterAdapter, const TShared<render::MeshBase>& mesh, const TShared<render::Texture>& texture, const TShared<SamplerState>& sampler, const render::GpuBufferSuballocation& uniformBuffer, const TShared<render::GpuBuffer>& clipRegionBuffer) const
{
	TShared<render::Texture> spriteTexture;
	if(texture != nullptr)
		spriteTexture = texture;
	else
		spriteTexture = render::Texture::kWhite;

	mTextureParameter.Set(spriteTexture);
	mSamplerParameter.Set(sampler);

	const TShared<render::VertexData>& vertexData = mesh->GetVertexData();
	const TShared<render::GpuBuffer>& vertexBuffer = vertexData->GetBuffer(0);

	parameterAdapter->Update(mMaterial);

	parameterAdapter->SetUniformBuffer("GUIParams", uniformBuffer);

	const TShared<render::GpuParameterSet>& gpuParameters = parameterAdapter->GetGpuParameterSet();
	gpuParameters->SetStorageBuffer("gVertices", vertexBuffer);

	if(clipRegionBuffer != nullptr)
		gpuParameters->SetStorageBuffer("gClipRegions", clipRegionBuffer);
}


void SpriteMaterial::Render(render::GpuCommandBuffer& commandBuffer, const TShared<render::GpuParameterSet>& parameters, const TShared<render::MeshBase>& mesh, const SubMesh& subMesh, const TShared<render::GpuBuffer>& clipRegionBuffer, u32 clipRegionCount, const TShared<SpriteMaterialExtraInfo>& additionalData) const
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
	const Color tint = gGuiUseLinearColorSpace ? materialInformation.Tint.GetLinear() : materialInformation.Tint;
	render::gGUISpriteUniformBufferDefinition.gTint.Set(uniforms, tint);
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


