//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsSprite.h"
#include "CoreObject/BsCoreObjectManager.h"
#include "Material/BsMaterial.h"
#include "Image/BsTexture.h"
#include "Mesh/BsMesh.h"
#include "Material/BsShader.h"
#include "Renderer/BsRendererUtility.h"
#include "Material/BsGpuParamsSet.h"
#include "CoreObject/BsRenderThread.h"
#include "Image/BsSpriteTexture.h"

using namespace bs;

SpriteMaterial::SpriteMaterial(u32 id, const HMaterial& material, ShaderVariationParameters variation, bool allowBatching)
	: mId(id), mAllowBatching(allowBatching), mMaterialStored(false), mParamBufferIdx(-1)
{
	mMaterial = B3DGetRenderProxy(material);

	FindVariationInformation findTechniqueDesc;
	findTechniqueDesc.VariationParameters = &variation;

	mTechnique = mMaterial->FindTechnique(findTechniqueDesc);

	mMaterialStored.store(true, std::memory_order_release);

	GetRenderThread().PostCommand(std::bind(&SpriteMaterial::Initialize, this));
}

SpriteMaterial::~SpriteMaterial()
{
	GetRenderThread().PostCommand(std::bind(&SpriteMaterial::Destroy, mMaterial, mParams));
}

void SpriteMaterial::Initialize()
{
	// Make sure that mMaterial assignment completes on the previous thread before continuing
	const bool materialStored = mMaterialStored.load(std::memory_order_acquire);
	B3D_ASSERT(materialStored == true);

	auto fnPrepareTechnique = [this](const u32 techniqueIndex)
	{
		const SPtr<ct::Technique> technique = mMaterial->GetTechnique(techniqueIndex);
		B3D_ASSERT(technique != nullptr);

		if(!technique->IsCompiled())
		{
			const TAsyncOp<bool> operation = technique->Compile();
			operation.BlockUntilComplete();
		}
	};

	fnPrepareTechnique(mTechnique);

	const SPtr<ct::Pass>& pass = mMaterial->GetPass(0, mTechnique);

	if(pass)
		pass->Compile();

	mParams = mMaterial->CreateParamsSet(mTechnique);

	SPtr<ct::Shader> shader = mMaterial->GetShader();
	if(shader->HasTextureParam("gMainTexture"))
	{
		mTextureParam = mMaterial->GetParamTexture("gMainTexture");
		mSamplerParam = mMaterial->GetParamSamplerState("gMainTexSamp");
	}

	mParamBufferIdx = mParams->GetParamBlockBufferIndex("GUIParams");

	if(mParamBufferIdx == ~0u)
		B3D_LOG(Error, GUI, "Sprite material shader missing \"GUIParams\" block.");
}

void SpriteMaterial::Destroy(const SPtr<ct::Material>& material, const SPtr<ct::GpuParamsSet>& params)
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

void SpriteMaterial::Render(ct::GpuCommandBuffer& commandBuffer, const SPtr<ct::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<ct::Texture>& texture, const SPtr<SamplerState>& sampler, const SPtr<ct::GpuBuffer>& paramBuffer, const SPtr<SpriteMaterialExtraInfo>& additionalData) const
{
	SPtr<ct::Texture> spriteTexture;
	if(texture != nullptr)
		spriteTexture = texture;
	else
		spriteTexture = ct::Texture::kWhite;

	mTextureParam.Set(spriteTexture);
	mSamplerParam.Set(sampler);

	if(mParamBufferIdx != ~0u)
		mParams->SetParamBlockBuffer(mParamBufferIdx, paramBuffer, true);

	mMaterial->UpdateParamsSet(mParams);
	ct::GetRendererUtility().SetPass(commandBuffer, mMaterial, 0, mTechnique);
	ct::GetRendererUtility().SetPassParams(commandBuffer, mParams);

	ct::GetRendererUtility().Draw(commandBuffer, mesh, subMesh);
}

namespace bs
{
	namespace ct
	{
		SpriteMaterialInfo::SpriteMaterialInfo(const TSpriteMaterialInfo<false>& other)
		{
			GroupId = other.GroupId;
			Texture = B3DGetRenderProxy(other.Texture);
			SpriteTexture = B3DGetRenderProxy(other.SpriteTexture);
			Tint = other.Tint;
			AnimationStartTime = other.AnimationStartTime;
			AdditionalData = other.AdditionalData;
		}
	} // namespace ct
}


