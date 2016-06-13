//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSpriteMaterials.h"
#include "BsBuiltinResources.h"
#include "BsGpuBuffer.h"
#include "BsMaterial.h"

namespace BansheeEngine
{
	SpriteImageTransparentMaterial::SpriteImageTransparentMaterial()
		:SpriteMaterial(0, BuiltinResources::instance().createSpriteImageMaterial())
	{ }

	SpriteImageOpaqueMaterial::SpriteImageOpaqueMaterial()
		: SpriteMaterial(1, BuiltinResources::instance().createSpriteNonAlphaImageMaterial())
	{ }

	SpriteTextMaterial::SpriteTextMaterial()
		: SpriteMaterial(2, BuiltinResources::instance().createSpriteTextMaterial())
	{ }

	SpriteLineMaterial::SpriteLineMaterial()
		: SpriteMaterial(3, BuiltinResources::instance().createSpriteLineMaterial())
	{
		
	}

	UINT64 SpriteLineMaterial::getMergeHash(const SpriteMaterialInfo& info) const
	{
		size_t hash = SpriteMaterial::getMergeHash(info);

		SPtr<SpriteMaterialLineInfo> extraInfo = std::static_pointer_cast<SpriteMaterialLineInfo>(info.additionalData);
		hash_combine(hash, extraInfo->width);

		return (UINT64)hash;
	}

	void SpriteLineMaterial::merge(SpriteMaterialInfo& mergeInto, const SpriteMaterialInfo& mergeFrom) const
	{
		SPtr<SpriteMaterialLineInfo> extraInfoDest = std::static_pointer_cast<SpriteMaterialLineInfo>(mergeInto.additionalData);
		SPtr<SpriteMaterialLineInfo> extraInfoSrc = std::static_pointer_cast<SpriteMaterialLineInfo>(mergeFrom.additionalData);

		extraInfoDest->points.insert(extraInfoDest->points.end(), extraInfoSrc->points.begin(), extraInfoSrc->points.end());
	}

	void SpriteLineMaterial::render(const SPtr<MeshCoreBase>& mesh, const SPtr<TextureCore>& texture,
		const SPtr<SamplerStateCore>& sampler, const Color& tint, const Matrix4& worldTransform,
		const Vector2& invViewportSize, const SPtr<SpriteMaterialExtraInfo>& additionalData) const
	{
		SPtr<SpriteMaterialLineInfo> lineInfo = std::static_pointer_cast<SpriteMaterialLineInfo>(additionalData);
		if(lineInfo->pointBuffer == nullptr)
		{
			UINT32 numPoints = (UINT32)lineInfo->points.size();
			lineInfo->pointBuffer = GpuBufferCore::create(numPoints, 0, GBT_STANDARD, BF_32X2F, GBU_STATIC);
			lineInfo->pointBuffer->writeData(0, numPoints * sizeof(Vector2), &lineInfo->points[0], BufferWriteType::Discard);
		}
		
		mMaterial->setBuffer("linePoints", lineInfo->pointBuffer);
		mMaterial->setFloat("lineWidth", lineInfo->width);

		// Note: If the line shader ends up using a more complex filter, generate a filter LUT as a pre-process step
		SpriteMaterial::render(mesh, texture, sampler, tint, worldTransform, invViewportSize, additionalData);
	}
}