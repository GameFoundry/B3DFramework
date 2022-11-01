//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "2D/BsSprite.h"
#include "Material/BsMaterial.h"
#include "Image/BsTexture.h"
#include "Mesh/BsMesh.h"
#include "Material/BsShader.h"
#include "Renderer/BsRendererUtility.h"
#include "Material/BsGpuParamsSet.h"
#include "CoreThread/BsCoreThread.h"

using namespace bs;

SpriteMaterial::SpriteMaterial(u32 id, const HMaterial& material, ShaderVariation variation, bool allowBatching)
	: mId(id), mAllowBatching(allowBatching), mMaterialStored(false), mParamBufferIdx(-1)
{
	mMaterial = material->GetCore();

	FIND_TECHNIQUE_DESC findTechniqueDesc;
	findTechniqueDesc.Variation = &variation;

	variation.SetBool("ALPHA", false);
	mTechnique = mMaterial->FindTechnique(findTechniqueDesc);

	variation.SetBool("ALPHA", true);
	mAlphaTechnique = mMaterial->FindTechnique(findTechniqueDesc);

	mMaterialStored.store(true, std::memory_order_release);

	GetCoreThread().QueueCommand(std::bind(&SpriteMaterial::Initialize, this));
}

SpriteMaterial::~SpriteMaterial()
{
	GetCoreThread().QueueCommand(std::bind(&SpriteMaterial::Destroy, mMaterial, mParams, mAlphaParams));
}

void SpriteMaterial::Initialize()
{
	// Make sure that mMaterial assignment completes on the previous thread before continuing
	const bool materialStored = mMaterialStored.load(std::memory_order_acquire);
	assert(materialStored == true);

	const SPtr<ct::Pass>& pass = mMaterial->GetPass(0, mTechnique);

	if(pass)
		pass->Compile();

	const SPtr<ct::Pass>& alphaPass = mMaterial->GetPass(0, mAlphaTechnique);

	if(alphaPass)
		alphaPass->Compile();

	mParams = mMaterial->CreateParamsSet(mTechnique);
	mAlphaParams = mMaterial->CreateParamsSet(mAlphaTechnique);

	SPtr<ct::Shader> shader = mMaterial->GetShader();
	if(shader->HasTextureParam("gMainTexture"))
	{
		mTextureParam = mMaterial->GetParamTexture("gMainTexture");
		mSamplerParam = mMaterial->GetParamSamplerState("gMainTexSamp");
	}

	mParamBufferIdx = mParams->GetParamBlockBufferIndex("GUIParams");
	mAlphaParamBufferIdx = mAlphaParams->GetParamBlockBufferIndex("GUIParams");

	if(mParamBufferIdx == (u32)-1 || mAlphaParamBufferIdx == (u32)-1)
		BS_LOG(Error, GUI, "Sprite material shader missing \"GUIParams\" block.");
}

void SpriteMaterial::Destroy(const SPtr<ct::Material>& material, const SPtr<ct::GpuParamsSet>& params, const SPtr<ct::GpuParamsSet>& alphaParams)
{
	// Do nothing, we just need to make sure the material pointer's last reference is lost while on the core thread
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

void SpriteMaterial::Render(const SPtr<ct::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<ct::Texture>& texture, const SPtr<ct::SamplerState>& sampler, const SPtr<ct::GpuParamBlockBuffer>& paramBuffer, const SPtr<SpriteMaterialExtraInfo>& additionalData, bool alphaOnly) const
{
	SPtr<ct::Texture> spriteTexture;
	if(texture != nullptr)
		spriteTexture = texture;
	else
		spriteTexture = ct::Texture::WHITE;

	mTextureParam.Set(spriteTexture);
	mSamplerParam.Set(sampler);

	if(!alphaOnly)
	{
		if(mParamBufferIdx != (u32)-1)
			mParams->SetParamBlockBuffer(mParamBufferIdx, paramBuffer, true);

		mMaterial->UpdateParamsSet(mParams);
		ct::GetRendererUtility().SetPass(mMaterial, 0, mTechnique);
		ct::GetRendererUtility().SetPassParams(mParams);
	}
	else
	{
		if(mAlphaParamBufferIdx != (u32)-1)
			mAlphaParams->SetParamBlockBuffer(mAlphaParamBufferIdx, paramBuffer, true);

		mMaterial->UpdateParamsSet(mAlphaParams);
		ct::GetRendererUtility().SetPass(mMaterial, 0, mAlphaTechnique);
		ct::GetRendererUtility().SetPassParams(mAlphaParams);
	}

	ct::GetRendererUtility().Draw(mesh, subMesh);
}
