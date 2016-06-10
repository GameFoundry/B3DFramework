//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsSpriteMaterial.h"

namespace BansheeEngine
{
	/** @addtogroup 2D-Internal
	 *  @{
	 */

	/** Sprite material used for rendering transparent images. */
	class BS_EXPORT SpriteImageTransparentMaterial : public SpriteMaterial
	{
	public:
		SpriteImageTransparentMaterial();
	};

	/** Sprite material used for rendering opaque images. */
	class BS_EXPORT SpriteImageOpaqueMaterial : public SpriteMaterial
	{
	public:
		SpriteImageOpaqueMaterial();
	};

	/** Sprite material used for rendering text. */
	class BS_EXPORT SpriteTextMaterial : public SpriteMaterial
	{
	public:
		SpriteTextMaterial();
	};

	/** Sprite material used for antialiased lines. */
	class BS_EXPORT SpriteLineMaterial : public SpriteMaterial
	{
	public:
		SpriteLineMaterial();

		/** @copydoc SpriteMaterial::getMergeHash */
		UINT64 getMergeHash(const SpriteMaterialInfo& info) const override;

		/** @copydoc SpriteMaterial::merge */
		void merge(SpriteMaterialInfo& mergeInto, const SpriteMaterialInfo& mergeFrom) const override;

		/** @copydoc SpriteMaterial::render */
		void render(const SPtr<MeshCoreBase>& mesh, const SPtr<TextureCore>& texture,
			const SPtr<SamplerStateCore>& sampler, const Color& tint, const Matrix4& worldTransform,
			const Vector2& invViewportSize, const SPtr<SpriteMaterialExtraInfo>& additionalData) const override;
	};

	/** Extra data required by the SpriteLineMaterial. */
	struct SpriteMaterialLineInfo : SpriteMaterialExtraInfo
	{
		/** @copydoc SpriteMaterialLineInfo::clone() */
		SPtr<SpriteMaterialExtraInfo> clone() const override
		{
			SPtr<SpriteMaterialLineInfo> info = bs_shared_ptr_new<SpriteMaterialLineInfo>();
			info->points = points;
			info->width = width;

			return info;
		}

		Vector<Vector2> points;
		float width;

		// Core thread only
		SPtr<GpuBufferCore> pointBuffer;
	};

	/** @} */
}