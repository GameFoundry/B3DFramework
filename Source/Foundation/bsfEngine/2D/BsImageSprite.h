//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "2D/BsSprite.h"
#include "Math/BsVector2.h"
#include "Image/BsColor.h"

namespace bs
{
	/** @addtogroup 2D
	 *  @{
	 */

	/**	Image sprite description structure used for initializing or updating an image sprite. */
	struct IMAGE_SPRITE_DESC
	{
		IMAGE_SPRITE_DESC() = default;

		u32 Width = 0; /**< Width of the image in pixels. */
		u32 Height = 0; /**< Height of the image in pixels. */
		SpriteAnchor Anchor = SA_TopLeft; /**< Determines where in the provided bounds will the sprite be placed. */
		Vector2 UvScale = Vector2(1.0f, 1.0f); /**< Scale applied to UV width/height used for rendering the sprite. */
		Vector2 UvOffset = Vector2(0.0f, 0.0f); /**< Offset applied to UV coordinates when rendering the sprite. */
		bool Transparent = true; /**< Should the sprite be rendered with transparency. */

		HSpriteTexture Texture; /**< Texture to overlay on the sprite. */
		Color Color; /**< Color tint to apply to the sprite. */
		/**
		 * Time (since application start) at which the sprite texture's 0th frame is played. Used if the sprite texture
		 * has sprite sheet animation defined.
		 */
		float AnimationStartTime = 0.0f;

		/**
		 * Borders (in texels) that allow you to control how is the texture scaled. If borders are 0 the texture will be
		 * scaled uniformly. If they are not null only the area inside the borders will be scaled and the outside are will
		 * remain the original size as in the texture. This allows you to implement "Scale9Grid" functionality.
		 */
		u32 BorderLeft = 0;
		u32 BorderRight = 0;
		u32 BorderTop = 0;
		u32 BorderBottom = 0;
	};

	/**	A sprite consisting of a single image represented by a sprite texture. */
	class BS_EXPORT ImageSprite : public Sprite
	{
	public:
		ImageSprite() = default;
		~ImageSprite();

		/**
		 * Recreates internal sprite data according the specified description structure.
		 *
		 * @param[in]	desc	Describes the geometry and material of the sprite.
		 * @param[in]	groupId	Group identifier that forces different materials to be used for different groups (for
		 *						example you don't want the sprites to share the same material if they use different world
		 *						transform matrices).
		 */
		void Update(const IMAGE_SPRITE_DESC& desc, u64 groupId);

		/**
		 * Calculates the required UV scale in order for a texture of size @p sourceSize to be placed on the surface
		 * of @p destSize size, while respecting the chosen scale mode.
		 */
		static Vector2 GetTextureUvScale(Vector2I sourceSize, Vector2I destSize, TextureScaleMode scaleMode);

	private:
		/**	Clears internal geometry buffers. */
		void ClearMesh();
	};

	/** @} */
} // namespace bs
