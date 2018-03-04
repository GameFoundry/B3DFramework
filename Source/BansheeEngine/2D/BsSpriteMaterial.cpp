//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
	SpriteMaterial::SpriteMaterial(UINT32 id, const HMaterial& material)
		:mId(id), mMaterialStored(false), mParamBufferIdx(-1)
	{
		mMaterial = material->getCore();
		mMaterialStored.store(true, std::memory_order_release);

		gCoreThread().queueCommand(std::bind(&SpriteMaterial::initialize, this));
	}

	SpriteMaterial::~SpriteMaterial()
	{
		gCoreThread().queueCommand(std::bind(&SpriteMaterial::destroy, mMaterial, mParams));
	}

	void SpriteMaterial::initialize()
	{
		// Make sure that mMaterial assignment completes on the previous thread before continuing
		bool materialStored = mMaterialStored.load(std::memory_order_acquire);
		assert(materialStored == true);

		mParams = mMaterial->createParamsSet();

		SPtr<ct::Shader> shader = mMaterial->getShader();
		if(shader->hasTextureParam("gMainTexture"))
		{
			mTextureParam = mMaterial->getParamTexture("gMainTexture");
			mSamplerParam = mMaterial->getParamSamplerState("gMainTexSamp");
		}

		mParamBufferIdx = mParams->getParamBlockBufferIndex("GUIParams");

		if(mParamBufferIdx == (UINT32)-1)
			LOGERR("Sprite material shader missing \"GUIParams\" block.");
	}

	void SpriteMaterial::destroy(const SPtr<ct::Material>& material, const SPtr<ct::GpuParamsSet>& params)
	{
		// Do nothing, we just need to make sure the material pointer's last reference is lost while on the core thread
	}

	UINT64 SpriteMaterial::getMergeHash(const SpriteMaterialInfo& info) const
	{
		UINT64 textureId = 0;
		if (info.texture.isLoaded())
			textureId = info.texture->getInternalID();

		size_t hash = 0;
		hash_combine(hash, info.groupId);
		hash_combine(hash, getId());
		hash_combine(hash, textureId);
		hash_combine(hash, info.tint);

		return (UINT64)hash;
	}

	void SpriteMaterial::render(const SPtr<ct::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<ct::Texture>& texture,
		const SPtr<ct::SamplerState>& sampler, const SPtr<ct::GpuParamBlockBuffer>& paramBuffer,
		const SPtr<SpriteMaterialExtraInfo>& additionalData) const
	{
		SPtr<ct::Texture> spriteTexture;
		if (texture != nullptr)
			spriteTexture = texture;
		else
			spriteTexture = ct::Texture::WHITE;

		mTextureParam.set(spriteTexture);
		mSamplerParam.set(sampler);

		if(mParamBufferIdx != (UINT32)-1)
			mParams->setParamBlockBuffer(mParamBufferIdx, paramBuffer, true);

		mMaterial->updateParamsSet(mParams);

		ct::gRendererUtility().setPass(mMaterial);
		ct::gRendererUtility().setPassParams(mParams);
		ct::gRendererUtility().draw(mesh, subMesh);
	}
}