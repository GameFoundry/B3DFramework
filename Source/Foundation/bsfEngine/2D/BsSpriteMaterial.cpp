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

namespace bs
{
	SpriteMaterial::SpriteMaterial(UINT32 id, const HMaterial& material, ShaderVariation variation, bool allowBatching)
		:mId(id), mAllowBatching(allowBatching), mMaterialStored(false), mParamBufferIdx(-1)
	{
		mMaterial = material->getCore();

		FIND_TECHNIQUE_DESC findTechniqueDesc;
		findTechniqueDesc.variation = &variation;

		variation.setBool("ALPHA", false);
		mTechnique = mMaterial->findTechnique(findTechniqueDesc);

		variation.setBool("ALPHA", true);
		mAlphaTechnique = mMaterial->findTechnique(findTechniqueDesc);

		mMaterialStored.store(true, std::memory_order_release);

		gCoreThread().queueCommand(std::bind(&SpriteMaterial::initialize, this));
	}

	SpriteMaterial::~SpriteMaterial()
	{
		gCoreThread().queueCommand(std::bind(&SpriteMaterial::destroy, mMaterial, mParams, mAlphaParams));
	}

	void SpriteMaterial::Initialize()
	{
		// Make sure that mMaterial assignment completes on the previous thread before continuing
		const bool materialStored = mMaterialStored.load(std::memory_order_acquire);
		assert(materialStored == true);

		const SPtr<ct::Pass>& pass = mMaterial->getPass(0, mTechnique);

		if(pass)
			pass->compile();

		const SPtr<ct::Pass>& alphaPass = mMaterial->getPass(0, mAlphaTechnique);

		if(alphaPass)
			alphaPass->compile();

		mParams = mMaterial->createParamsSet(mTechnique);
		mAlphaParams = mMaterial->createParamsSet(mAlphaTechnique);

		SPtr<ct::Shader> shader = mMaterial->getShader();
		if(shader->hasTextureParam("gMainTexture"))
		{
			mTextureParam = mMaterial->getParamTexture("gMainTexture");
			mSamplerParam = mMaterial->getParamSamplerState("gMainTexSamp");
		}

		mParamBufferIdx = mParams->getParamBlockBufferIndex("GUIParams");
		mAlphaParamBufferIdx = mAlphaParams->getParamBlockBufferIndex("GUIParams");

		if(mParamBufferIdx == (UINT32)-1 || mAlphaParamBufferIdx == (UINT32)-1)
			BS_LOG(Error, GUI, "Sprite material shader missing \"GUIParams\" block.");
	}

	void SpriteMaterial::Destroy(const SPtr<ct::Material>& material, const SPtr<ct::GpuParamsSet>& params, const SPtr<ct::GpuParamsSet>& alphaParams)
	{
		// Do nothing, we just need to make sure the material pointer's last reference is lost while on the core thread
	}

	UINT64 SpriteMaterial::GetMergeHash(const SpriteMaterialInfo& info) const
	{
		UINT64 textureId = 0;
		if (info.texture.isLoaded())
			textureId = info.texture->getInternalID();

		size_t hash = 0;
		bs_hash_combine(hash, info.groupId);
		bs_hash_combine(hash, getId());
		bs_hash_combine(hash, textureId);
		bs_hash_combine(hash, info.tint);

		return (UINT64)hash;
	}

	void SpriteMaterial::render(const SPtr<ct::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<ct::Texture>& texture,
		const SPtr<ct::SamplerState>& sampler, const SPtr<ct::GpuParamBlockBuffer>& paramBuffer,
		const SPtr<SpriteMaterialExtraInfo>& additionalData, bool alphaOnly) const
	{
		SPtr<ct::Texture> spriteTexture;
		if (texture != nullptr)
			spriteTexture = texture;
		else
			spriteTexture = ct::Texture::WHITE;

		mTextureParam.set(spriteTexture);
		mSamplerParam.set(sampler);

		if(!alphaOnly)
		{
			if(mParamBufferIdx != (UINT32)-1)
				mParams->setParamBlockBuffer(mParamBufferIdx, paramBuffer, true);

			mMaterial->updateParamsSet(mParams);
			ct::gRendererUtility().setPass(mMaterial, 0, mTechnique);
			ct::gRendererUtility().setPassParams(mParams);
		}
		else
		{
			if(mAlphaParamBufferIdx != (UINT32)-1)
				mAlphaParams->setParamBlockBuffer(mAlphaParamBufferIdx, paramBuffer, true);

			mMaterial->updateParamsSet(mAlphaParams);
			ct::gRendererUtility().setPass(mMaterial, 0, mAlphaTechnique);
			ct::gRendererUtility().setPassParams(mAlphaParams);
		}

		ct::gRendererUtility().draw(mesh, subMesh);
	}
}
