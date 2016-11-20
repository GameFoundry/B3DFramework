//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSprite.h"
#include "BsMaterial.h"
#include "BsTexture.h"
#include "BsMesh.h"
#include "BsShader.h"
#include "BsRendererUtility.h"
#include "BsCoreThread.h"

namespace bs
{
	SpriteMaterial::SpriteMaterial(UINT32 id, const HMaterial& material)
		:mId(id), mMaterialStored(false)
	{
		mMaterial = material->getCore();
		mMaterialStored.store(true, std::memory_order_release);

		gCoreAccessor().queueCommand(std::bind(&SpriteMaterial::initialize, this));
	}

	SpriteMaterial::~SpriteMaterial()
	{
		gCoreAccessor().queueCommand(std::bind(&SpriteMaterial::destroy, mMaterial, mParams));
	}

	void SpriteMaterial::initialize()
	{
		// Make sure that mMaterial assignment completes on the previous thread before continuing
		bool materialStored = mMaterialStored.load(std::memory_order_acquire);
		assert(materialStored == true);

		mParams = mMaterial->createParamsSet();
		SPtr<ShaderCore> shader = mMaterial->getShader();

		if(shader->hasTextureParam("mainTexture"))
		{
			mTextureParam = mMaterial->getParamTexture("mainTexture");
			mSamplerParam = mMaterial->getParamSamplerState("mainTexSamp");
		}

		mTintParam = mMaterial->getParamColor("tint");
		mInvViewportWidthParam = mMaterial->getParamFloat("invViewportWidth");
		mInvViewportHeightParam = mMaterial->getParamFloat("invViewportHeight");
		mWorldTransformParam = mMaterial->getParamMat4("worldTransform");
	}

	void SpriteMaterial::destroy(const SPtr<MaterialCore>& material, const SPtr<GpuParamsSetCore>& params)
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

	void SpriteMaterial::render(const SPtr<MeshCoreBase>& mesh, const SPtr<TextureCore>& texture,
		const SPtr<SamplerStateCore>& sampler, const Color& tint, const Matrix4& worldTransform,
		const Vector2& invViewportSize, const SPtr<SpriteMaterialExtraInfo>& additionalData) const
	{
		SPtr<TextureCore> spriteTexture;
		if (texture != nullptr)
			spriteTexture = texture;
		else
			spriteTexture = TextureCore::WHITE;

		mTextureParam.set(spriteTexture);
		mSamplerParam.set(sampler);
		mTintParam.set(tint);
		mInvViewportWidthParam.set(invViewportSize.x);
		mInvViewportHeightParam.set(invViewportSize.y);
		mWorldTransformParam.set(worldTransform);

		mMaterial->updateParamsSet(mParams);

		gRendererUtility().setPass(mMaterial);
		gRendererUtility().setPassParams(mParams);
		gRendererUtility().draw(mesh, mesh->getProperties().getSubMesh(0));
	}
}